// Build gated gamma-gamma matrices from calibrated compact triple-gamma trees.
// 从已经刻度完成的紧凑 triple-gamma Tree 构建带 gate 的 gamma-gamma 矩阵。
//
// Gate convention / Gate 定义:
//   gateWidthKeV=3 means three 1-keV gate channels: center-1 to center+1 keV.
//   gateWidthKeV=3 表示连续3个 1 keV gate channel，即中心值 -1 到 +1 keV。
//
// Filling convention / 填充定义:
//   For every gamma hit inside a gate, remove that hit and fill every unordered
//   pair among the remaining gamma hits symmetrically: (a,b) and (b,a).
//   每发现一个落入 gate 的 gamma，就去掉该 gamma，并将剩余 gamma 两两对称填充：
//   (a,b) 与 (b,a)。若同一事件有多个 gamma 落入 gate，则每个 gate hit 分别贡献。
//
// Usage / 用法:
// root -l -b -q 'triple_gamma_gate_fill.C("@compact_inputs.txt","gated.root","502,365,485",3,512,0,4096,"10,15,20,30,40")'

#include "triple_gamma_ana_calibrated.C"

#include "TH2D.h"
#include "TTreeReaderValue.h"

namespace {

struct GateNeutronMatrix {
  double lowMeV = 0.0;
  double highMeV = 0.0;
  TString tag;
  std::unique_ptr<TH2D> matrix;
  Long64_t acceptedEvents = 0;
  Long64_t acceptedGateHits = 0;
  Long64_t acceptedPairs = 0;
};

struct GammaGateResult {
  double centerKeV = 0.0;
  double lowKeV = 0.0;
  double highKeV = 0.0;
  TString tag;
  std::unique_ptr<TH2D> totalMatrix;
  std::vector<GateNeutronMatrix> neutronMatrices;
  Long64_t totalAcceptedEvents = 0;
  Long64_t totalAcceptedGateHits = 0;
  Long64_t totalAcceptedPairs = 0;
};

std::vector<double> ParseGateCenters(const char *gateCentersKeV)
{
  std::vector<double> centers;
  std::string text = gateCentersKeV ? gateCentersKeV : "";
  std::replace(text.begin(), text.end(), static_cast<char>(44), static_cast<char>(32));
  std::stringstream stream(text);
  double center = 0.0;
  while (stream >> center) {
    if (!std::isfinite(center) || center < 0.0) continue;
    if (std::find(centers.begin(), centers.end(), center) == centers.end()) {
      centers.push_back(center);
    }
  }
  return centers;
}

std::unique_ptr<TH2D> MakeGateMatrix(const TString &name,
                                     const TString &title,
                                     int energyBins,
                                     double energyMinKeV,
                                     double energyMaxKeV)
{
  std::unique_ptr<TH2D> matrix(new TH2D(name, title,
                                        energyBins, energyMinKeV, energyMaxKeV,
                                        energyBins, energyMinKeV, energyMaxKeV));
  matrix->SetDirectory(nullptr);
  matrix->GetXaxis()->SetTitle("Calibrated gamma energy 1 (keV)");
  matrix->GetYaxis()->SetTitle("Calibrated gamma energy 2 (keV)");
  return matrix;
}

std::vector<GammaGateResult> BuildGateResults(const std::vector<double> &gateCenters,
                                              double gateWidthKeV,
                                              const char *neutronEnergyEdgesMeV,
                                              int energyBins,
                                              double energyMinKeV,
                                              double energyMaxKeV)
{
  std::vector<GammaGateResult> results;
  const std::vector<double> neutronEdges = ParseEnergyEdges(neutronEnergyEdgesMeV);

  // The requested width counts 1-keV gate channels. Width 3 therefore gives
  // half width (3-1)/2 = 1 keV, exactly center +/- 1 keV.
  // 用户输入的是 1 keV channel 数；宽度3对应半宽 (3-1)/2 = 1 keV。
  const double halfWidthKeV = 0.5 * (gateWidthKeV - 1.0);
  for (double center : gateCenters) {
    GammaGateResult gate;
    gate.centerKeV = center;
    gate.lowKeV = center - halfWidthKeV;
    gate.highKeV = center + halfWidthKeV;
    gate.tag = TString::Format("Gate_%skeV", FormatNumberForName(center).Data());

    const TString totalTitle = TString::Format(
      "Gated gamma-gamma matrix, gate %.6g keV [%.6g, %.6g] keV, Total;Calibrated gamma energy 1 (keV);Calibrated gamma energy 2 (keV)",
      center, gate.lowKeV, gate.highKeV);
    gate.totalMatrix = MakeGateMatrix("GatedGammaGamma_Total", totalTitle,
                                      energyBins, energyMinKeV, energyMaxKeV);

    double lowMeV = 0.0;
    for (double highMeV : neutronEdges) {
      GateNeutronMatrix neutron;
      neutron.lowMeV = lowMeV;
      neutron.highMeV = highMeV;
      neutron.tag = TString::Format("E%s_%sMeV",
                                     FormatNumberForName(lowMeV).Data(),
                                     FormatNumberForName(highMeV).Data());
      const TString title = TString::Format(
        "Gated gamma-gamma matrix, gate %.6g keV [%.6g, %.6g] keV, neutron %s;Calibrated gamma energy 1 (keV);Calibrated gamma energy 2 (keV)",
        center, gate.lowKeV, gate.highKeV, neutron.tag.Data());
      neutron.matrix = MakeGateMatrix(
        TString::Format("GatedGammaGamma_%s", neutron.tag.Data()),
        title, energyBins, energyMinKeV, energyMaxKeV);
      gate.neutronMatrices.push_back(std::move(neutron));
      lowMeV = highMeV;
    }
    results.push_back(std::move(gate));
  }
  return results;
}

int FindGateNeutronMatrix(const GammaGateResult &gate, double neutronEnergyMeV)
{
  for (std::size_t index = 0; index < gate.neutronMatrices.size(); ++index) {
    const GateNeutronMatrix &bin = gate.neutronMatrices[index];
    if (neutronEnergyMeV >= bin.lowMeV && neutronEnergyMeV < bin.highMeV) {
      return static_cast<int>(index);
    }
  }
  return -1;
}

Long64_t FillRemainingPairs(TH2D *matrix,
                            const std::vector<double> &energies,
                            std::size_t gatedIndex)
{
  Long64_t pairs = 0;
  for (std::size_t first = 0; first < energies.size(); ++first) {
    if (first == gatedIndex) continue;
    for (std::size_t second = first + 1; second < energies.size(); ++second) {
      if (second == gatedIndex) continue;
      matrix->Fill(energies[first], energies[second]);
      matrix->Fill(energies[second], energies[first]);
      pairs++;
    }
  }
  return pairs;
}

} // namespace

void triple_gamma_gate_fill(const char *compactInputFiles = "@compact_inputs.txt",
                            const char *outputFile = "triple_gamma_gated.root",
                            const char *gateCentersKeV = "502,365,485",
                            double gateWidthKeV = 3.0,
                            int energyBins = 512,
                            double energyMinKeV = 0.0,
                            double energyMaxKeV = 4096.0,
                            const char *neutronEnergyEdgesMeV = "10,15,20,30,40",
                            int minCloverMultiplicity = 3,
                            Long64_t maxEntries = -1)
{
  if (gateWidthKeV < 1.0 || energyBins <= 0 || energyMaxKeV <= energyMinKeV ||
      minCloverMultiplicity < 3) {
    std::cerr << "Invalid gated gamma-gamma parameters." << std::endl;
    gSystem->Exit(2);
    return;
  }

  const std::vector<TString> compactInputs = SplitCsv(compactInputFiles);
  const std::vector<double> gateCenters = ParseGateCenters(gateCentersKeV);
  const std::vector<double> neutronEdges = ParseEnergyEdges(neutronEnergyEdgesMeV);
  if (compactInputs.empty() || gateCenters.empty() || neutronEdges.empty()) {
    std::cerr << "Compact inputs, gamma gates, or neutron-energy edges are empty." << std::endl;
    gSystem->Exit(2);
    return;
  }

  std::vector<GammaGateResult> gates = BuildGateResults(
    gateCenters, gateWidthKeV, neutronEnergyEdgesMeV,
    energyBins, energyMinKeV, energyMaxKeV);

  Long64_t scannedEntries = 0;
  Long64_t malformedEntries = 0;
  Long64_t processedFiles = 0;
  for (const TString &inputName : compactInputs) {
    TFile input(inputName, "READ");
    if (!input.IsOpen()) {
      std::cerr << "Cannot open compact input: " << inputName << std::endl;
      continue;
    }
    TTree *tree = dynamic_cast<TTree *>(input.Get("TripleGammaCompact"));
    if (!tree || !tree->GetBranch("EventNeutronEnergyMeV") ||
        !tree->GetBranch("CloverEnergyKeV")) {
      std::cerr << "Missing TripleGammaCompact branches in: " << inputName << std::endl;
      continue;
    }
    processedFiles++;

    TTreeReader reader(tree);
    TTreeReaderValue<Double_t> neutronEnergyMeV(reader, "EventNeutronEnergyMeV");
    TTreeReaderArray<float> cloverEnergyKeV(reader, "CloverEnergyKeV");
    while (reader.Next()) {
      if (maxEntries > 0 && scannedEntries >= maxEntries) break;
      scannedEntries++;

      const std::size_t multiplicity = static_cast<std::size_t>(cloverEnergyKeV.GetSize());
      if (multiplicity < static_cast<std::size_t>(minCloverMultiplicity)) {
        malformedEntries++;
        continue;
      }

      std::vector<double> energies;
      energies.reserve(multiplicity);
      for (std::size_t index = 0; index < multiplicity; ++index) {
        const double energy = cloverEnergyKeV[index];
        if (std::isfinite(energy) && energy >= energyMinKeV && energy < energyMaxKeV) {
          energies.push_back(energy);
        }
      }
      if (energies.size() < static_cast<std::size_t>(minCloverMultiplicity)) continue;

      for (GammaGateResult &gate : gates) {
        const int neutronBinIndex = FindGateNeutronMatrix(gate, *neutronEnergyMeV);
        bool eventAccepted = false;
        bool neutronEventAccepted = false;
        for (std::size_t gatedIndex = 0; gatedIndex < energies.size(); ++gatedIndex) {
          const double gatedEnergy = energies[gatedIndex];
          if (gatedEnergy < gate.lowKeV || gatedEnergy > gate.highKeV) continue;

          eventAccepted = true;
          gate.totalAcceptedGateHits++;
          gate.totalAcceptedPairs += FillRemainingPairs(
            gate.totalMatrix.get(), energies, gatedIndex);

          if (neutronBinIndex >= 0) {
            GateNeutronMatrix &bin = gate.neutronMatrices[neutronBinIndex];
            neutronEventAccepted = true;
            bin.acceptedGateHits++;
            bin.acceptedPairs += FillRemainingPairs(
              bin.matrix.get(), energies, gatedIndex);
          }
        }
        if (eventAccepted) gate.totalAcceptedEvents++;
        if (neutronEventAccepted && neutronBinIndex >= 0) {
          gate.neutronMatrices[neutronBinIndex].acceptedEvents++;
        }
      }
    }
    input.Close();
    if (maxEntries > 0 && scannedEntries >= maxEntries) break;
  }

  TString outName = outputFile && TString(outputFile).Length() > 0
                      ? TString(outputFile)
                      : TString("triple_gamma_gated.root");
  TFile output(outName, "RECREATE");
  if (!output.IsOpen()) {
    std::cerr << "Cannot create gated output: " << outName << std::endl;
    gSystem->Exit(2);
    return;
  }
  TDirectory *baseDirectory = output.mkdir("triple_gamma_gated");
  for (GammaGateResult &gate : gates) {
    TDirectory *gateDirectory = baseDirectory->mkdir(gate.tag);
    gateDirectory->cd();
    gate.totalMatrix->Write();
    for (GateNeutronMatrix &bin : gate.neutronMatrices) bin.matrix->Write();
  }
  output.Close();

  std::cout << "Processed compact files: " << processedFiles << std::endl;
  std::cout << "Scanned compact entries: " << scannedEntries << std::endl;
  std::cout << "Malformed compact entries: " << malformedEntries << std::endl;
  for (const GammaGateResult &gate : gates) {
    std::cout << gate.tag << " range=[" << gate.lowKeV << ", " << gate.highKeV
              << "] keV: events=" << gate.totalAcceptedEvents
              << " gate_hits=" << gate.totalAcceptedGateHits
              << " unordered_pairs=" << gate.totalAcceptedPairs << std::endl;
    for (const GateNeutronMatrix &bin : gate.neutronMatrices) {
      std::cout << "  " << bin.tag
                << ": events=" << bin.acceptedEvents
                << " gate_hits=" << bin.acceptedGateHits
                << " unordered_pairs=" << bin.acceptedPairs << std::endl;
    }
  }
  std::cout << "Gated output: " << outName << std::endl;
}
