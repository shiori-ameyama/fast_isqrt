#include <cstdint>
#include <cmath>
#include <iostream>
#include <vector>
#include <random>
#include <chrono>
#include <fast_isqrt/fast_isqrt.hpp>

using namespace fast_isqrt;

int main() {

    std::cout << "\nPerformance & Latency Benchmark (Speed Test)..." << std::endl;

    std::cout << "\n [sqrt64]" << std::endl;
    std::cout << "--------------------------------------------------" << std::endl;

    for (int i = 0; i < 3; i++) {

        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

        std::mt19937_64 rng_speed(static_cast<uint64_t>(millis));
        const int SPEED_SAMPLES = 100000000; // 1億回試行
        std::vector<uint64_t> bench_data(SPEED_SAMPLES);

        // 各種ビット長（全領域）からランダムに 64bit 整数を生成
        for (int i = 0; i < SPEED_SAMPLES; ++i) {
            bench_data[i] = rng_speed();
        }

        // ウォームアップ (キャッシュ・CPUクロックの安定化)
        volatile uint64_t dummy = 0;
        for (int i = 0; i < 100000; ++i) {
            dummy += isqrt64(bench_data[i]);
        }

        // 本計測
        volatile uint64_t sink = 0;
        auto t_start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < SPEED_SAMPLES; ++i) {
            sink += isqrt64(bench_data[i]);
        }

        auto t_end = std::chrono::high_resolution_clock::now();

        double total_ms = std::chrono::duration<double, std::milli>(t_end - t_start).count();
        double ns_per_op = std::chrono::duration<double, std::nano>(t_end - t_start).count() / SPEED_SAMPLES;

        std::cout << " Benchmark Results (isqrt64):" << std::endl;
        std::cout << "   - Seed      : " << static_cast<uint64_t>(millis) << std::endl;
        std::cout << "   - Data Size : " << SPEED_SAMPLES << " elements (Random 64-bit)" << std::endl;
        std::cout << "   - Total Time: " << total_ms << " ms" << std::endl;
        std::cout << "   - Avg Speed : " << ns_per_op << " ns / call" << std::endl;
        std::cout << "--------------------------------------------------" << std::endl;
    }

    std::cout << "\n [sqrt128]" << std::endl;
    std::cout << "--------------------------------------------------" << std::endl;

    for (int i = 0; i < 3; i++) {

        auto now = std::chrono::system_clock::now();
        auto duration = now.time_since_epoch();
        auto millis = std::chrono::duration_cast<std::chrono::milliseconds>(duration).count();

        std::mt19937_64 rng_speed(static_cast<uint64_t>(millis));
        const int SPEED_SAMPLES = 100000000; // 1億回試行
        std::vector<uint128_t> bench_data(SPEED_SAMPLES);

        // 各種ビット長（全領域）からランダムに 128bit 整数を生成
        for (int i = 0; i < SPEED_SAMPLES; ++i) {
            uint64_t hi = rng_speed();
            uint64_t lo = rng_speed();
            bench_data[i] = (static_cast<uint128_t>(hi) << 64) | lo;
        }

        // ウォームアップ (キャッシュ・CPUクロックの安定化)
        volatile uint128_t dummy = 0;
        for (int i = 0; i < 100000; ++i) {
            dummy += isqrt128(bench_data[i]);
        }

        // 本計測
        volatile uint128_t sink = 0;
        auto t_start = std::chrono::high_resolution_clock::now();

        for (int i = 0; i < SPEED_SAMPLES; ++i) {
            sink += isqrt128(bench_data[i]);
        }

        auto t_end = std::chrono::high_resolution_clock::now();

        double total_ms = std::chrono::duration<double, std::milli>(t_end - t_start).count();
        double ns_per_op = std::chrono::duration<double, std::nano>(t_end - t_start).count() / SPEED_SAMPLES;

        std::cout << " Benchmark Results (isqrt128):" << std::endl;
        std::cout << "   - Seed      : " << static_cast<uint64_t>(millis) << std::endl;
        std::cout << "   - Data Size : " << SPEED_SAMPLES << " elements (Random 128-bit)" << std::endl;
        std::cout << "   - Total Time: " << total_ms << " ms" << std::endl;
        std::cout << "   - Avg Speed : " << ns_per_op << " ns / call" << std::endl;
        std::cout << "--------------------------------------------------" << std::endl;
    }

    return 0;
}