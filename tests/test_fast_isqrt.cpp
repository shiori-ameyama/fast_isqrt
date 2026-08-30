#include <cstdint>
#include <cmath>
#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <fast_isqrt/fast_isqrt.hpp>

using namespace fast_isqrt;
// --------------------------------------------------
// 検証用ヘルパー関数
// --------------------------------------------------

// nsq(n) == x が厳密に floor(sqrt(n)) であるかの数学的検証
// 条件: x^2 <= n < (x + 1)^2
bool verify_isqrt(uint128_t n, uint128_t x) {
    if (x > UINT64_MAX) return false;

    uint128_t x2 = x * x;
    if (x2 > n) return false;
    
    // x + 1 が 2^64 を超える場合のオーバーフローチェック
    if (x == UINT64_MAX) {
        return true;
        // uint128_t の max に対して (x+1)^2 > max128 は成り立つ
    }
    
    uint128_t next_x2 = (x + 1) * (x + 1);
    return n < next_x2;
}

int main() {
    std::cout << "[1] Edge Cases Verification..." << std::endl;
    // 1. 小さな値 & 代表的な境界値
    std::vector<uint128_t> edge_cases = {
        0, 1, 2, 3, 4, 8, 9, 15, 16, 24, 25,
        0xFFFFFFFFULL, 0x100000000ULL,
        0xFFFFFFFFFFFFFFFFULL, static_cast<uint128_t>(0xFFFFFFFFFFFFFFFFULL) + 1
    };

    for (auto n : edge_cases) {
        uint128_t res = isqrt128(n);
        if (!verify_isqrt(n, res)) {
            std::cerr << "FAILED at edge case n = " << static_cast<uint64_t>(n) << std::endl;
            return 1;
        }
    }

    std::cout << "[2] Perfect Squares & Off-by-One Boundaries..." << std::endl;
    // 2. 完全平方数とその前後 (x^2 - 1, x^2, x^2 + 1)
    // 2^1 〜 2^64 までのベース値でテスト
    for (int bit = 1; bit <= 64; ++bit) {
        uint128_t x = (1ULL << (bit - 1));
        if (bit == 64) x = 0xFFFFFFFFFFFFFFFFULL;

        uint128_t sq = x * x;
        
        // sq - 1
        if (sq > 0) {
            if (!verify_isqrt(sq - 1, isqrt128(sq - 1))) {
                std::cerr << "FAILED at (x^2 - 1) bit=" << bit << std::endl;
                return 1;
            }
        }
        // sq
        if (!verify_isqrt(sq, isqrt128(sq))) {
            std::cerr << "FAILED at (x^2) bit=" << bit << std::endl;
            return 1;
        }
        // sq + 1
        if (sq < ~(uint128_t)0) {
            if (!verify_isqrt(sq + 1, isqrt128(sq + 1))) {
                std::cerr << "FAILED at (x^2 + 1) bit=" << bit << std::endl;
                return 1;
            }
        }
    }

    std::cout << "[3] Bit Length Transition Boundaries (lz = 0..127)..." << std::endl;
    // 3. ビット長切り替わりの境界 (2^k - 1, 2^k, 2^k + 1)
    for (int k = 1; k <= 127; ++k) {
        uint128_t base = (uint128_t(1) << k);

        if (!verify_isqrt(base - 2, isqrt128(base - 2))) {
            std::cerr << "FAILED at 2^" << k << " - 2" << std::endl;
            return 1;
        }
        if (!verify_isqrt(base - 1, isqrt128(base - 1))) {
            std::cerr << "FAILED at 2^" << k << " - 1" << std::endl;
            return 1;
        }
        if (!verify_isqrt(base, isqrt128(base))) {
            std::cerr << "FAILED at 2^" << k << std::endl;
            return 1;
        }
        if (!verify_isqrt(base + 1, isqrt128(base + 1))) {
            std::cerr << "FAILED at 2^" << k << " + 1" << std::endl;
            return 1;
        }
        if (!verify_isqrt(base + 2, isqrt128(base + 2))) {
            std::cerr << "FAILED at 2^" << k << " + 2" << std::endl;
            return 1;
        }
    }

    // 128bit 最大値境界
    uint128_t max128 = ~(uint128_t)0;
    if (!verify_isqrt(max128, isqrt128(max128))) {
        std::cerr << "FAILED at uint128_t MAX" << std::endl;
        return 1;
    }
    if (!verify_isqrt(max128-1, isqrt128(max128-1))) {
        std::cerr << "FAILED at uint128_t MAX - 1" << std::endl;
        return 1;
    }

    std::cout << "[4] Massive Stress Test across All Bit Lengths (128M Random Samples)..." << std::endl;
    // 4. 全ビット領域のランダムストレス環境テスト (1 bit 〜 128 bit を均等走査)
    std::mt19937_64 rng(42);
    const int SAMPLES_PER_BIT = 1000000; // 1ビット長あたり100万個 (計1億2800万個)

    for (int bit = 1; bit <= 128; ++bit) {
        for (int i = 0; i < SAMPLES_PER_BIT; ++i) {
            uint128_t n = 0;
            if (bit <= 64) {
                uint64_t mask = (bit == 64) ? ~0ULL : ((1ULL << bit) - 1);
                uint64_t min_val = (bit == 1) ? 0 : (1ULL << (bit - 1));
                n = min_val + (rng() % (mask - min_val + 1));
            } else {
                uint64_t hi_mask = (bit == 128) ? ~0ULL : ((1ULL << (bit - 64)) - 1);
                uint64_t hi_min = (bit == 65) ? 1ULL : (1ULL << (bit - 65));
                uint64_t hi = hi_min + (rng() % (hi_mask - hi_min + 1));
                uint64_t lo = rng();
                n = (static_cast<uint128_t>(hi) << 64) | lo;
            }

            uint128_t x = isqrt128(n);
            if (!verify_isqrt(n, x)) {
                std::cerr << "FAILED Random Test at bit " << bit << " for n = 0x" 
                          << std::hex << static_cast<uint64_t>(n >> 64) 
                          << static_cast<uint64_t>(n) << std::dec << std::endl;
                return 1;
            }
        }
        if (bit % 16 == 0) {
            std::cout << "  Bit lengths 1 to " << bit << " PASSED." << std::endl;
        }
    }

    std::cout << "[5] Testing Random Perfect Squares Neighbor Neighborhood (±10^6)..." << std::endl;
    // 5. ランダムな 64bit 整数 x を決め、x^2 の ±10^6 近傍と [0, uint128_max] の共通部分を全検証
    {
        std::mt19937_64 rng_sq(1337);
        const int NUM_SQUARES = 1000;               // テストする完全平方数の個数
        const int64_t RADIUS = 1000000;             // 近傍半径 (±10^6)
        const uint128_t MAX128 = ~(uint128_t)0;

        for (int i = 0; i < NUM_SQUARES; ++i) {
            uint64_t base_x = rng_sq();
            uint128_t sq = static_cast<uint128_t>(base_x) * base_x;

            // [0, MAX128] と [sq - RADIUS, sq + RADIUS] の交差範囲 (共通部分) を計算
            uint128_t start_n = (sq > static_cast<uint128_t>(RADIUS)) ? (sq - RADIUS) : 0;
            uint128_t end_n = (MAX128 - sq > static_cast<uint128_t>(RADIUS)) ? (sq + RADIUS) : MAX128;

            // 範囲内の全整数 n をテスト
            for (uint128_t n = start_n; n <= end_n; ++n) {
                uint128_t res = isqrt128(n);
                if (!verify_isqrt(n, res)) {
                    std::cerr << "FAILED at Square Neighborhood Test!" << std::endl;
                    std::cerr << "  base_x = " << base_x << std::endl;
                    std::cerr << "  n      = 0x" << std::hex 
                              << static_cast<uint64_t>(n >> 64) 
                              << static_cast<uint64_t>(n) << std::dec << std::endl;
                    return 1;
                }
            }

            if ((i + 1) % 100 == 0) {
                std::cout << "  Square neighborhood test: " << (i + 1) << " / " << NUM_SQUARES << " passed." << std::endl;
            }
        }
    }

    std::cout << "\nALL CORRECTNESS TESTS PASSED PERFECTLY!" << std::endl;
    
    // std::cout << "\n[6] Performance & Latency Benchmark (Speed Test)..." << std::endl;
    // {
    //     std::mt19937_64 rng_speed(2026);
    //     const int SPEED_SAMPLES = 10000000; // 1,000万回試行
    //     std::vector<uint128_t> bench_data(SPEED_SAMPLES);

    //     // 各種ビット長（全領域）からランダムに 128bit 整数を生成
    //     for (int i = 0; i < SPEED_SAMPLES; ++i) {
    //         uint64_t hi = rng_speed();
    //         uint64_t lo = rng_speed();
    //         bench_data[i] = (static_cast<uint128_t>(hi) << 64) | lo;
    //     }

    //     // ウォームアップ (キャッシュ・CPUクロックの安定化)
    //     volatile uint128_t dummy = 0;
    //     for (int i = 0; i < 100000; ++i) {
    //         dummy += isqrt128(bench_data[i]);
    //     }

    //     // 本計測
    //     volatile uint128_t sink = 0;
    //     auto t_start = std::chrono::high_resolution_clock::now();

    //     for (int i = 0; i < SPEED_SAMPLES; ++i) {
    //         sink += isqrt128(bench_data[i]);
    //     }

    //     auto t_end = std::chrono::high_resolution_clock::now();

    //     double total_ms = std::chrono::duration<double, std::milli>(t_end - t_start).count();
    //     double ns_per_op = std::chrono::duration<double, std::nano>(t_end - t_start).count() / SPEED_SAMPLES;

    //     std::cout << "--------------------------------------------------" << std::endl;
    //     std::cout << " Benchmark Results (isqrt128):" << std::endl;
    //     std::cout << "   - Data Size : " << SPEED_SAMPLES << " elements (Random 128-bit)" << std::endl;
    //     std::cout << "   - Total Time: " << total_ms << " ms" << std::endl;
    //     std::cout << "   - Avg Speed : " << ns_per_op << " ns / call" << std::endl;
    //     std::cout << "--------------------------------------------------" << std::endl;
    // }

    return 0;
}