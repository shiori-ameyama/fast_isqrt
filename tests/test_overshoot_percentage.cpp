#include <iostream>
#include <cstdint>
#include <cmath>
#include <random>
#include <format>
#include <chrono>

struct BranchStats {
    uint64_t total_calls = 0;
    uint64_t exact_hits   = 0; // 補正なし（一発的中）
    uint64_t undershoots  = 0; // x++ が走った (x < true_root)
    uint64_t overshoots   = 0; // x-- が走った (x > true_root)
};

// 判定付き isqrt64
inline uint64_t tracked_isqrt64(uint64_t n, BranchStats& stats) noexcept {
    stats.total_calls++;

    uint64_t x = static_cast<uint64_t>(std::sqrt(static_cast<double>(n)));

    bool undershot = false;
    bool overshot  = false;

    // UINT32_MAX ガード
    while ((x + 1) * (x + 1) <= n && x < UINT32_MAX) [[unlikely]] {
        undershot = true;
        x++;
    }
    while (x * x > n || x > UINT32_MAX) [[unlikely]] {
        overshot = true;
        x--;
    }

    if (undershot) {
        stats.undershoots++;
    } else if (overshot) {
        std::cout << n << std::endl;
        stats.overshoots++;
    } else {
        stats.exact_hits++;
    }

    return x;
}

// 結果表示用
void print_stats(const std::string& title, const BranchStats& s) {
    std::cout << std::format("=== {} ===\n", title);
    std::cout << std::format("  Total Calls : {}\n", s.total_calls);
    std::cout << std::format("  Exact Hits  : {:12} ({:8.5f}%)\n", s.exact_hits, (double)s.exact_hits / s.total_calls * 100.0);
    std::cout << std::format("  Undershoots : {:12} ({:8.5f}%) [x++]\n", s.undershoots, (double)s.undershoots / s.total_calls * 100.0);
    std::cout << std::format("  Overshoots  : {:12} ({:8.5f}%) [x--]\n", s.overshoots, (double)s.overshoots / s.total_calls * 100.0);
    std::cout << "--------------------------------------------------\n\n";
}

int main() {
    // 1. 全域小領域テスト (0 ~ 1億)
    {
        BranchStats stats;
        constexpr uint64_t LIMIT = 100'000'000;
        for (uint64_t n = 0; n < LIMIT; ++n) {
            tracked_isqrt64(n, stats);
        }
        print_stats("Exhaustive Test (n = 0 .. 100,000,000)", stats);
    }

    // 2. 境界値（平方数・その直前直後）集中テスト
    {
        BranchStats stats;
        constexpr uint64_t R_MAX = 10'000'000;
        for (uint64_t r = 1; r <= R_MAX; ++r) {
            uint64_t sq = r * r;
            tracked_isqrt64(sq - 1, stats);
            tracked_isqrt64(sq, stats);
            if (sq < UINT64_MAX) tracked_isqrt64(sq + 1, stats);
        }
        print_stats("Squares Neighborhood Test (r = 1 .. 10,000,000)", stats);
    }

    // 3. 64-bit 全域ランダムテスト (1億サンプル)
    {
        BranchStats stats;
        std::mt19937_64 rng(1788519815138ULL);
        constexpr size_t SAMPLES = 100'000'000;

        for (size_t i = 0; i < SAMPLES; ++i) {
            tracked_isqrt64(rng(), stats);
        }
        print_stats("64-bit Full Range Random Test (100M Samples)", stats);
    }

    // 4. 浮動小数点数精度限界領域 (2^52 〜 2^64-1) ランダムテスト
    {
        BranchStats stats;
        std::mt19937_64 rng(42);
        constexpr size_t SAMPLES = 100'000'000;
        // 2^52 以上に限定（仮数部 52bit を超えて精度落ちが発生しやすい領域）
        constexpr uint64_t MIN_VAL = 1ULL << 52; 

        for (size_t i = 0; i < SAMPLES; ++i) {
            uint64_t n = MIN_VAL + (rng() % (UINT64_MAX - MIN_VAL));
            tracked_isqrt64(n, stats);
        }
        print_stats("High-Bit Range Test (n >= 2^52, 100M Samples)", stats);
    }

    return 0;
}