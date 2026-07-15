// EN: Compare every top-level NFS histogram in a standalone output and an ADNE
//     nfs_histoExogam2_1.root file. The report includes structure, entries, and
//     cell-by-cell differences (including underflow/overflow cells).
// CN: 比较独立程序输出与 ADNE nfs_histoExogam2_1.root 中的全部顶层 NFS 直方图。
//     报告检查结构、entries 以及逐 bin 差异，并包含 underflow/overflow。
//
// Usage / 用法:
// root -l -b -q 'compare_nfs_histo.C("standalone.root","adne.root","comparison.tsv",1e-6)'

#include <TFile.h>
#include <TH1.h>
#include <TKey.h>

#include <algorithm>
#include <cmath>
#include <fstream>
#include <iomanip>
#include <iostream>
#include <memory>
#include <set>
#include <string>

namespace {

bool SameAxis(const TAxis *first, const TAxis *second)
{
  if(!first || !second)return first==second;
  if(first->GetNbins()!=second->GetNbins())return false;
  for(int bin=1; bin<=first->GetNbins()+1; ++bin){
    if(std::abs(first->GetBinLowEdge(bin)-second->GetBinLowEdge(bin))>1e-12)return false;
  }
  return true;
}

bool SameShape(const TH1 *first, const TH1 *second)
{
  if(!first || !second)return false;
  if(first->GetDimension()!=second->GetDimension())return false;
  if(!SameAxis(first->GetXaxis(),second->GetXaxis()))return false;
  if(first->GetDimension()>=2 && !SameAxis(first->GetYaxis(),second->GetYaxis()))return false;
  if(first->GetDimension()>=3 && !SameAxis(first->GetZaxis(),second->GetZaxis()))return false;
  return first->GetNcells()==second->GetNcells();
}

} // namespace

void compare_nfs_histo(const char *standalonePath,
                       const char *adnePath,
                       const char *reportPath="nfs_histo_comparison.tsv",
                       double tolerance=1e-6)
{
  std::unique_ptr<TFile> standalone(TFile::Open(standalonePath,"READ"));
  std::unique_ptr<TFile> adne(TFile::Open(adnePath,"READ"));
  if(!standalone || standalone->IsZombie()){
    std::cerr<<"Cannot open standalone output / 无法打开独立输出: "<<standalonePath<<std::endl;
    return;
  }
  if(!adne || adne->IsZombie()){
    std::cerr<<"Cannot open ADNE output / 无法打开 ADNE 输出: "<<adnePath<<std::endl;
    return;
  }

  std::ofstream report(reportPath);
  if(!report){
    std::cerr<<"Cannot create report / 无法创建报告: "<<reportPath<<std::endl;
    return;
  }
  report<<"name\tstatus\tdimension\tentries_standalone\tentries_adne"
          "\tdifferent_cells\tmax_abs_difference\tsum_abs_difference\n";

  Long64_t compared=0;
  Long64_t exact=0;
  Long64_t different=0;
  Long64_t missing=0;
  std::set<std::string> referenceNames;

  TIter next(adne->GetListOfKeys());
  while(auto *key=dynamic_cast<TKey *>(next())){
    const std::string name=key->GetName();
    if(name.rfind("nfs_",0)!=0)continue;
    auto *reference=dynamic_cast<TH1 *>(adne->Get(name.c_str()));
    if(!reference)continue;
    referenceNames.insert(name);
    auto *candidate=dynamic_cast<TH1 *>(standalone->Get(name.c_str()));
    if(!candidate){
      report<<name<<"\tMISSING\t"<<reference->GetDimension()
            <<"\t0\t"<<std::setprecision(17)<<reference->GetEntries()
            <<"\t-1\tnan\tnan\n";
      ++missing;
      continue;
    }
    if(!SameShape(candidate,reference)){
      report<<name<<"\tSHAPE_MISMATCH\t"<<reference->GetDimension()
            <<"\t"<<std::setprecision(17)<<candidate->GetEntries()
            <<"\t"<<reference->GetEntries()<<"\t-1\tnan\tnan\n";
      ++different;
      ++compared;
      continue;
    }

    Long64_t differentCells=0;
    double maximumDifference=0.;
    long double absoluteDifferenceSum=0.;
    for(int cell=0; cell<reference->GetNcells(); ++cell){
      const double delta=std::abs(candidate->GetBinContent(cell)-reference->GetBinContent(cell));
      if(delta>tolerance)++differentCells;
      maximumDifference=std::max(maximumDifference,delta);
      absoluteDifferenceSum+=delta;
    }
    const double entryDifference=std::abs(candidate->GetEntries()-reference->GetEntries());
    const bool isExact=(differentCells==0 && entryDifference<=tolerance);
    report<<name<<'\t'<<(isExact ? "MATCH" : "DIFFERENT")
          <<'\t'<<reference->GetDimension()
          <<'\t'<<std::setprecision(17)<<candidate->GetEntries()
          <<'\t'<<reference->GetEntries()
          <<'\t'<<differentCells
          <<'\t'<<maximumDifference
          <<'\t'<<static_cast<double>(absoluteDifferenceSum)<<'\n';
    ++compared;
    if(isExact)++exact;
    else ++different;
  }

  Long64_t extra=0;
  TIter nextStandalone(standalone->GetListOfKeys());
  while(auto *key=dynamic_cast<TKey *>(nextStandalone())){
    const std::string name=key->GetName();
    if(name.rfind("nfs_",0)!=0)continue;
    auto *histogram=dynamic_cast<TH1 *>(standalone->Get(name.c_str()));
    if(histogram && referenceNames.count(name)==0){
      report<<name<<"\tEXTRA\t"<<histogram->GetDimension()
            <<"\t"<<std::setprecision(17)<<histogram->GetEntries()
            <<"\t0\t-1\tnan\tnan\n";
      ++extra;
    }
  }
  report.close();

  std::cout<<"NFS histogram comparison / NFS 直方图比较"<<std::endl
           <<"  compared: "<<compared<<std::endl
           <<"  exact matches: "<<exact<<std::endl
           <<"  different: "<<different<<std::endl
           <<"  missing: "<<missing<<std::endl
           <<"  extra: "<<extra<<std::endl
           <<"  report: "<<reportPath<<std::endl;
}
