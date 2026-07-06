// 中文说明：把 ROOT TH2 normal bins 写成 RadWare m4b/spn 风格的 signed int32 原始矩阵。
#include <TFile.h>
#include <TH2.h>
#include <cstdio>
#include <cstdint>
#include <iostream>
#include <limits>

void save2m4b(const char* rootfile,
              const char* histname,
              const char* outfile) {
    TFile f(rootfile, "READ");
    if (f.IsZombie()) {
        std::cerr << "Cannot open " << rootfile << std::endl;
        return;
    }
    auto h2 = dynamic_cast<TH2*>(f.Get(histname));
    if (!h2) {
        std::cerr << "Cannot find TH2 named " << histname << std::endl;
        return;
    }

    FILE* out = std::fopen(outfile, "wb");
    if (!out) {
        std::cerr << "Cannot create " << outfile << std::endl;
        return;
    }

    const double i32_max = static_cast<double>(std::numeric_limits<int32_t>::max());
    const double i32_min = static_cast<double>(std::numeric_limits<int32_t>::min());
    long long clamp_low = 0;
    long long clamp_high = 0;

    int nx = h2->GetNbinsX();
    int ny = h2->GetNbinsY();
    for (int ix = 1; ix <= nx; ++ix) {
        for (int iy = 1; iy <= ny; ++iy) {
            double x = h2->GetBinContent(ix, iy);
            int32_t val = 0;
            if (x > i32_max) {
                val = std::numeric_limits<int32_t>::max();
                ++clamp_high;
            } else if (x < i32_min) {
                val = std::numeric_limits<int32_t>::min();
                ++clamp_low;
            } else {
                val = static_cast<int32_t>(x);
            }
            std::fwrite(&val, sizeof(val), 1, out);
        }
    }

    std::fclose(out);
    f.Close();
    std::cout << "Done: wrote " << outfile
              << " (" << nx << "x" << ny
              << " int32_t values, truncation toward zero";
    if (clamp_low || clamp_high) {
        std::cout << ", clamped low=" << clamp_low
                  << ", clamped high=" << clamp_high;
    }
    std::cout << ")" << std::endl;
}
