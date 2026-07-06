// 中文说明：把一维谱展开成二维辅助矩阵，主要用于兼容旧的 m4b/slice 投影流程。
#include <TFile.h>
#include <TH1.h>
#include <TH2D.h>
#include <TString.h>
#include <iostream>

void expand1Dto2D(const char* filename,
                  const char* inputHistName,
                  const char* outputHistName) {
    // 1) 用 UPDATE 模式打开文件
    TFile* file = TFile::Open(filename, "UPDATE");
    if (!file || file->IsZombie()) {
        std::cerr << "[ERROR] 无法打开或创建文件: " << filename << std::endl;
        return;
    }

    // 2) 从这个文件里拿到原始的 1D 直方图
    TH1* h1 = dynamic_cast<TH1*>(file->Get(inputHistName));
    if (!h1) {
        std::cerr << "[ERROR] 在文件 \"" 
                  << filename << "\" 中找不到名为 \"" 
                  << inputHistName << "\" 的一维直方图。" << std::endl;
        file->Close();
        return;
    }

    // **关键一步：把 h1 从 file 所属的 Directory 脱离出来**
    h1->SetDirectory(nullptr);  
    // 这样一来，file->Close() 时就不会 delete 掉 h1，h1 依然保留在内存里。

    // 3) 读取 h1 的 bin 信息，创建一个全新的 TH2D
    Int_t    nBinsX = h1->GetNbinsX();
    Double_t xMin   = h1->GetXaxis()->GetXmin();
    Double_t xMax   = h1->GetXaxis()->GetXmax();

    Int_t    nBinsY = nBinsX;
    Double_t yMin   = 0.0;
    Double_t yMax   = static_cast<Double_t>(nBinsY);

    TH2D* h2 = new TH2D(outputHistName,
                        outputHistName,
                        nBinsX, xMin, xMax,
                        nBinsY, yMin, yMax);

    // 用 h1 的每个 bin 内容填充到 h2 的 y-bin=1 那一行
    for (Int_t ix = 1; ix <= nBinsX; ++ix) {
        Double_t cnt = h1->GetBinContent(ix);
        h2->SetBinContent(ix, 1, cnt);
    }

    // **同样，如果你不想让 h2 被 file->Close() 自动删掉，也脱离它的目录**
    // 其实 h2 目前是直接用 new 创建的，默认是挂在当前 gDirectory（就是 file）里的。
    // 所以也要先脱离：
    h2->SetDirectory(nullptr);

    // 4) 把 h2 写入文件
    file->cd();
    h2->Write(outputHistName, TObject::kOverwrite);

    std::cout << "[INFO] 已将 2D 直方图 \"" 
              << outputHistName 
              << "\" 写入文件 " << filename 
              << "（UPDATE 模式）" << std::endl;

    // 5) 先手动 delete 掉我们在内存里独立拥有的 h1 / h2，避免 ROOT 后续清理时再去碰野指针
    delete h1;
    delete h2;

    // 6) 关闭文件。此时，file 目录下不再有 h1、h2，所以 ROOT 只会删掉它自己内部的目录结构，不会触碰刚刚 delete 过的那两个 pointer。
    file->Close();
}
