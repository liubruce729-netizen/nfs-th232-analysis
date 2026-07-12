// Serial TH3D writer for calibrated compact triple-gamma trees.
// 对刻度后的紧凑三重 gamma Tree 串行写 TH3D。
//
// Only one dense TH3D exists in memory at a time. The compact trees are scanned
// once for Total and once for every neutron-energy interval. Each completed cube
// is written and deleted before the next cube is allocated.
// 任意时刻内存中只有一张密集 TH3D。程序分别为 Total 和各中子能区扫描紧凑 Tree；
// 每张图写入并释放后，才创建下一张图。
//
// Direct usage / 直接调用示例:
// root -l -b -q 'triple_gamma_fill_serial.C("@compact_files.txt","triple_gamma.root",3,512,0,2048,"10,15,20,30,40",12)'

#include "triple_gamma_ana_calibrated.C"
#include "TTreeReaderValue.h"

namespace {

struct SerialCubeSpec {
  TString name;
  TString title;
  bool total = false;
  double neutronLowMeV = 0.0;
  double neutronHighMeV = 0.0;
};

bool FillOneSerialCube(const SerialCubeSpec &spec,
                       const std::vector<TString> &compactInputs,
                       TDirectory *outputDirectory,
                       int minCloverMultiplicity,
                       int energyBins,
                       double energyMinKeV,
                       double energyMaxKeV,
                       Long64_t &acceptedEvents,
                       Long64_t &acceptedTriples)
{
  // SetDirectory(nullptr) is essential: deleting the histogram after Write()
  // must really release its dense bin array before the next cube is allocated.
  // 脱离 ROOT 目录所有权，确保 Write() 后 delete 能立即释放密集 bin 数组。
  std::unique_ptr<TH3D> cube(MakeCube(spec.name, spec.title, energyBins, energyMinKeV, energyMaxKeV));
  cube->SetDirectory(nullptr);
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

      FillAllTriples(cube.get(), energies);
      acceptedEvents++;
      acceptedTriples += CountUnorderedTriples(energies.size());
    }
    input.Close();
  }

  outputDirectory->cd();
  cube->Write();
  // unique_ptr releases this cube at function return, before the next call.
  // 函数返回时立即释放当前 cube，再进入下一张图。
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
                                double maxSingleCubeMemoryGiB)
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

  const std::vector<double> edges = ParseEnergyEdges(neutronEnergyEdgesMeV);
  if (edges.empty()) {
    std::cerr << "No valid neutron-energy edges were provided." << std::endl;
    return false;
  }

  // The guard now checks one cube, not all six cubes, because allocation is serial.
  // 现在内存保护只计算一张图，因为6张图不会同时存在。
  const double oneCubeGiB = EstimateTH3DStorageGiB(energyBins, 1);
  if (oneCubeGiB > maxSingleCubeMemoryGiB) {
    std::cerr << "One requested TH3D exceeds the memory guard." << std::endl;
    std::cerr << "  energyBins=" << energyBins
              << " one_cube=" << oneCubeGiB << " GiB"
              << " limit=" << maxSingleCubeMemoryGiB << " GiB" << std::endl;
    return false;
  }

  std::vector<SerialCubeSpec> specs;
  specs.push_back({"TripleGammaCube_Total",
                   "Calibrated triple-gamma cube Total;Calibrated gamma energy 1 (keV);Calibrated gamma energy 2 (keV);Calibrated gamma energy 3 (keV)",
                   true, 0.0, 0.0});
  double low = 0.0;
  for (double high : edges) {
    SerialCubeSpec spec;
    const TString tag = TString::Format("E%s_%sMeV", FormatNumberForName(low).Data(), FormatNumberForName(high).Data());
    spec.name = TString::Format("TripleGammaCube_%s", tag.Data());
    spec.title = TString::Format("Calibrated triple-gamma cube %s;Calibrated gamma energy 1 (keV);Calibrated gamma energy 2 (keV);Calibrated gamma energy 3 (keV)", tag.Data());
    spec.neutronLowMeV = low;
    spec.neutronHighMeV = high;
    specs.push_back(spec);
    low = high;
  }

  TString outName = outputFile && TString(outputFile).Length() > 0
                      ? TString(outputFile)
                      : TString("triple_gamma_ana_calibrated.root");
  TFile out(outName, "RECREATE");
  if (!out.IsOpen()) {
    std::cerr << "Cannot create output: " << outName << std::endl;
    return false;
  }
  TDirectory *directory = out.mkdir("triple_gamma_ana_calibrated");
  if (!directory) {
    std::cerr << "Cannot create output directory in: " << outName << std::endl;
    out.Close();
    return false;
  }

  std::cout << "Serial cube mode: " << specs.size()
            << " cubes, one allocation at a time, estimated "
            << oneCubeGiB << " GiB per cube." << std::endl;
  for (std::size_t i = 0; i < specs.size(); ++i) {
    Long64_t acceptedEvents = 0;
    Long64_t acceptedTriples = 0;
    std::cout << "Build cube " << (i + 1) << "/" << specs.size()
              << ": " << specs[i].name << std::endl;
    if (!FillOneSerialCube(specs[i], compactInputs, directory,
                           minCloverMultiplicity, energyBins,
                           energyMinKeV, energyMaxKeV,
                           acceptedEvents, acceptedTriples)) {
      out.Close();
      return false;
    }
    // Flush the completed object before allocating the next dense cube.
    // 在创建下一张密集 cube 前，将当前对象刷新到磁盘。
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
                              double maxSingleCubeMemoryGiB = 12.0)
{
  if (!FillTripleGammaCubesSerial(compactInputFiles, outputFile,
                                  minCloverMultiplicity, energyBins,
                                  energyMinKeV, energyMaxKeV,
                                  neutronEnergyEdgesMeV,
                                  maxSingleCubeMemoryGiB)) {
    gSystem->Exit(2);
  }
}
