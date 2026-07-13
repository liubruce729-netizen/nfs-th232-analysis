// Serial ROOT histogram writer for calibrated compact triple-gamma trees.
// 对刻度后的紧凑三重 gamma Tree 串行写 ROOT 三维直方图。
//
// ROOT cannot serialize a single object whose byte count exceeds 1,073,741,822.
// A dense TH3D with 512 bins per axis exceeds this hard limit. In "auto" mode,
// this macro therefore uses TH3F whenever TH3D would be too large. The launcher
// runs one cube per ROOT process, so all memory and ROOT write buffers are released
// before the next cube is allocated.
// ROOT 单对象序列化上限为 1,073,741,822 字节。512^3 的密集 TH3D 超过该硬限制，
// 因此 auto 模式会在 TH3D 过大时自动使用 TH3F。启动脚本让每张 cube 在独立 ROOT
// 进程中运行，下一张图开始前，上一进程的直方图和写缓冲会被操作系统彻底释放。
//
// Direct usage / 直接调用示例（cubeIndex: 0=Total, 1..5=neutron bins）:
// root -l -b -q 'triple_gamma_fill_serial.C("@compact_files.txt","triple.root",3,512,0,4096,"10,15,20,30,40",12,0,"auto")'

#include "triple_gamma_ana_calibrated.C"
#include "TH3.h"
#include "TH3F.h"
#include "TTreeReaderValue.h"

namespace {

constexpr double kRootMaximumObjectByteCount = 1073741822.0;
constexpr double kDoubleObjectSafetyFraction = 0.95;

struct SerialCubeSpec {
  TString name;
  TString title;
  bool total = false;
  double neutronLowMeV = 0.0;
  double neutronHighMeV = 0.0;
};

double EstimateOneCubeGiB(int bins, int bytesPerBin)
{
  const double cells = static_cast<double>(bins + 2) *
                       static_cast<double>(bins + 2) *
                       static_cast<double>(bins + 2);
  return cells * static_cast<double>(bytesPerBin) / 1024.0 / 1024.0 / 1024.0;
}

std::vector<SerialCubeSpec> BuildSerialCubeSpecs(const char *neutronEnergyEdgesMeV)
{
  std::vector<SerialCubeSpec> specs;
  specs.push_back({"TripleGammaCube_Total",
                   "Calibrated triple-gamma cube Total;Calibrated gamma energy 1 (keV);Calibrated gamma energy 2 (keV);Calibrated gamma energy 3 (keV)",
                   true, 0.0, 0.0});

  double low = 0.0;
  for (double high : ParseEnergyEdges(neutronEnergyEdgesMeV)) {
    const TString tag = TString::Format("E%s_%sMeV",
                                         FormatNumberForName(low).Data(),
                                         FormatNumberForName(high).Data());
    SerialCubeSpec spec;
    spec.name = TString::Format("TripleGammaCube_%s", tag.Data());
    spec.title = TString::Format(
      "Calibrated triple-gamma cube %s;Calibrated gamma energy 1 (keV);Calibrated gamma energy 2 (keV);Calibrated gamma energy 3 (keV)",
      tag.Data());
    spec.neutronLowMeV = low;
    spec.neutronHighMeV = high;
    specs.push_back(spec);
    low = high;
  }
  return specs;
}

std::unique_ptr<TH3> MakeSerialCube(const SerialCubeSpec &spec,
                                    bool useFloatStorage,
                                    int bins,
                                    double energyMinKeV,
                                    double energyMaxKeV)
{
  TH3 *histogram = nullptr;
  if (useFloatStorage) {
    histogram = new TH3F(spec.name, spec.title,
                         bins, energyMinKeV, energyMaxKeV,
                         bins, energyMinKeV, energyMaxKeV,
                         bins, energyMinKeV, energyMaxKeV);
  }
  else {
    histogram = new TH3D(spec.name, spec.title,
                         bins, energyMinKeV, energyMaxKeV,
                         bins, energyMinKeV, energyMaxKeV,
                         bins, energyMinKeV, energyMaxKeV);
  }
  histogram->SetDirectory(nullptr);
  histogram->GetXaxis()->SetTitle("Calibrated gamma energy 1 (keV)");
  histogram->GetYaxis()->SetTitle("Calibrated gamma energy 2 (keV)");
  histogram->GetZaxis()->SetTitle("Calibrated gamma energy 3 (keV)");
  return std::unique_ptr<TH3>(histogram);
}

void FillOrderedTripleGeneric(TH3 *cube, double a, double b, double c)
{
  cube->Fill(a, b, c);
  cube->Fill(a, c, b);
  cube->Fill(b, a, c);
  cube->Fill(b, c, a);
  cube->Fill(c, a, b);
  cube->Fill(c, b, a);
}

void FillAllTriplesGeneric(TH3 *cube, const std::vector<double> &energies)
{
  for (std::size_t i = 0; i < energies.size(); ++i) {
    for (std::size_t j = i + 1; j < energies.size(); ++j) {
      for (std::size_t k = j + 1; k < energies.size(); ++k) {
        FillOrderedTripleGeneric(cube, energies[i], energies[j], energies[k]);
      }
    }
  }
}

bool FillOneSerialCube(const SerialCubeSpec &spec,
                       const std::vector<TString> &compactInputs,
                       TDirectory *outputDirectory,
                       bool useFloatStorage,
                       int minCloverMultiplicity,
                       int energyBins,
                       double energyMinKeV,
                       double energyMaxKeV,
                       Long64_t &acceptedEvents,
                       Long64_t &acceptedTriples)
{
  std::unique_ptr<TH3> cube = MakeSerialCube(spec, useFloatStorage,
                                             energyBins, energyMinKeV, energyMaxKeV);
  acceptedEvents = 0;
  acceptedTriples = 0;

  for (const auto &inputName : compactInputs) {
    TFile input(inputName, "READ");
    if (!input.IsOpen()) {
      std::cerr << "Cannot open compact input: " << inputName << std::endl;
      return false;
    }
    TTree *tree = dynamic_cast<TTree *>(input.Get("TripleGammaCompact"));
    if (!tree) {
      std::cerr << "Cannot find TripleGammaCompact in: " << inputName << std::endl;
      return false;
    }
    if (!tree->GetBranch("EventNeutronEnergyMeV") || !tree->GetBranch("CloverEnergyKeV")) {
      std::cerr << "Compact branches are missing in: " << inputName << std::endl;
      return false;
    }

    TTreeReader reader(tree);
    TTreeReaderValue<Double_t> neutronEnergyMeV(reader, "EventNeutronEnergyMeV");
    TTreeReaderArray<float> cloverEnergyKeV(reader, "CloverEnergyKeV");
    while (reader.Next()) {
      if (!spec.total && !(*neutronEnergyMeV >= spec.neutronLowMeV &&
                           *neutronEnergyMeV < spec.neutronHighMeV)) {
        continue;
      }

      const std::size_t multiplicity = static_cast<std::size_t>(cloverEnergyKeV.GetSize());
      if (multiplicity < static_cast<std::size_t>(minCloverMultiplicity)) continue;

      std::vector<double> energies;
      energies.reserve(multiplicity);
      for (std::size_t i = 0; i < multiplicity; ++i) {
        const double energy = cloverEnergyKeV[i];
        if (energy >= energyMinKeV && energy < energyMaxKeV && std::isfinite(energy)) {
          energies.push_back(energy);
        }
      }
      if (energies.size() < static_cast<std::size_t>(minCloverMultiplicity)) continue;

      FillAllTriplesGeneric(cube.get(), energies);
      acceptedEvents++;
      acceptedTriples += CountUnorderedTriples(energies.size());
    }
    input.Close();
  }

  outputDirectory->cd();
  const Int_t writtenBytes = cube->Write("", TObject::kOverwrite);
  if (writtenBytes <= 0) {
    std::cerr << "Failed to write cube: " << spec.name << std::endl;
    return false;
  }
  return true;
}

} // namespace

bool FillTripleGammaCubesSerial(const char *compactInputFiles,
                                const char *outputFile,
                                int minCloverMultiplicity,
                                int energyBins,
                                double energyMinKeV,
                                double energyMaxKeV,
                                const char *neutronEnergyEdgesMeV,
                                double maxSingleCubeMemoryGiB,
                                int cubeIndex,
                                const char *histogramStorage)
{
  if (minCloverMultiplicity < 3 || energyBins <= 0 || energyMaxKeV <= energyMinKeV) {
    std::cerr << "Invalid serial cube parameters." << std::endl;
    return false;
  }

  const std::vector<TString> compactInputs = SplitCsv(compactInputFiles);
  if (compactInputs.empty()) {
    std::cerr << "No compact input files were provided." << std::endl;
    return false;
  }

  const std::vector<SerialCubeSpec> specs = BuildSerialCubeSpecs(neutronEnergyEdgesMeV);
  if (specs.size() < 2) {
    std::cerr << "No valid neutron-energy edges were provided." << std::endl;
    return false;
  }
  if (cubeIndex < -1 || cubeIndex >= static_cast<int>(specs.size())) {
    std::cerr << "Invalid cube index " << cubeIndex << "; available range is 0-"
              << (specs.size() - 1) << std::endl;
    return false;
  }

  TString storage(histogramStorage ? histogramStorage : "auto");
  storage.ToLower();
  if (storage != "auto" && storage != "double" && storage != "float") {
    std::cerr << "histogramStorage must be auto, double, or float." << std::endl;
    return false;
  }

  const double doubleBytes = EstimateOneCubeGiB(energyBins, sizeof(Double_t)) *
                             1024.0 * 1024.0 * 1024.0;
  const bool doubleWouldExceedRootLimit =
    doubleBytes > kRootMaximumObjectByteCount * kDoubleObjectSafetyFraction;
  if (storage == "double" && doubleWouldExceedRootLimit) {
    std::cerr << "Requested TH3D cannot be serialized safely by ROOT." << std::endl;
    std::cerr << "  bins=" << energyBins
              << " estimated TH3D array=" << doubleBytes / 1024.0 / 1024.0 / 1024.0
              << " GiB, ROOT object byte-count limit="
              << kRootMaximumObjectByteCount / 1024.0 / 1024.0 / 1024.0 << " GiB" << std::endl;
    std::cerr << "Use histogramStorage=auto/float or reduce --gamma-bins." << std::endl;
    return false;
  }

  const bool useFloatStorage = storage == "float" ||
                               (storage == "auto" && doubleWouldExceedRootLimit);
  const double oneCubeGiB = EstimateOneCubeGiB(
    energyBins, useFloatStorage ? sizeof(Float_t) : sizeof(Double_t));
  if (oneCubeGiB > maxSingleCubeMemoryGiB) {
    std::cerr << "One requested histogram exceeds the memory guard." << std::endl;
    std::cerr << "  bins=" << energyBins
              << " storage=" << (useFloatStorage ? "TH3F" : "TH3D")
              << " one_cube=" << oneCubeGiB << " GiB"
              << " limit=" << maxSingleCubeMemoryGiB << " GiB" << std::endl;
    return false;
  }

  TString outName = outputFile && TString(outputFile).Length() > 0
                      ? TString(outputFile)
                      : TString("triple_gamma_ana_calibrated.root");
  const bool writeAll = cubeIndex < 0;
  const char *openMode = (writeAll || cubeIndex == 0) ? "RECREATE" : "UPDATE";
  TFile out(outName, openMode);
  if (!out.IsOpen()) {
    std::cerr << "Cannot open output in " << openMode << " mode: " << outName << std::endl;
    return false;
  }
  TDirectory *directory = out.GetDirectory("triple_gamma_ana_calibrated");
  if (!directory) directory = out.mkdir("triple_gamma_ana_calibrated");
  if (!directory) {
    std::cerr << "Cannot create output directory in: " << outName << std::endl;
    out.Close();
    return false;
  }

  const int firstIndex = writeAll ? 0 : cubeIndex;
  const int lastIndex = writeAll ? static_cast<int>(specs.size()) - 1 : cubeIndex;
  std::cout << "Histogram storage: " << (useFloatStorage ? "TH3F" : "TH3D")
            << ", estimated " << oneCubeGiB << " GiB per cube." << std::endl;
  for (int index = firstIndex; index <= lastIndex; ++index) {
    Long64_t acceptedEvents = 0;
    Long64_t acceptedTriples = 0;
    std::cout << "Build cube " << (index + 1) << "/" << specs.size()
              << ": " << specs[index].name << std::endl;
    if (!FillOneSerialCube(specs[index], compactInputs, directory,
                           useFloatStorage, minCloverMultiplicity,
                           energyBins, energyMinKeV, energyMaxKeV,
                           acceptedEvents, acceptedTriples)) {
      out.Close();
      return false;
    }
    out.Flush();
    std::cout << "  accepted events=" << acceptedEvents
              << " unordered triples=" << acceptedTriples
              << " filled permutations=" << acceptedTriples * 6 << std::endl;
  }
  out.Close();

  std::cout << "Serial triple-gamma output: " << outName << std::endl;
  return true;
}

void triple_gamma_fill_serial(const char *compactInputFiles = "@compact_files.txt",
                              const char *outputFile = "triple_gamma_ana_calibrated.root",
                              int minCloverMultiplicity = 3,
                              int energyBins = 256,
                              double energyMinKeV = 0.0,
                              double energyMaxKeV = 4096.0,
                              const char *neutronEnergyEdgesMeV = "10,15,20,30,40",
                              double maxSingleCubeMemoryGiB = 12.0,
                              int cubeIndex = -1,
                              const char *histogramStorage = "auto")
{
  if (!FillTripleGammaCubesSerial(compactInputFiles, outputFile,
                                  minCloverMultiplicity, energyBins,
                                  energyMinKeV, energyMaxKeV,
                                  neutronEnergyEdgesMeV,
                                  maxSingleCubeMemoryGiB,
                                  cubeIndex, histogramStorage)) {
    gSystem->Exit(2);
  }
}
