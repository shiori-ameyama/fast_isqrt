#ifndef FAST_ISQRT_HPP
#define FAST_ISQRT_HPP

#include <cstdint>
#include <cmath>
#include <array>
#include <type_traits>
#include <utility>

#ifndef __SIZEOF_INT128__
#error "fast_isqrt requires 128-bit integer support (__int128_t / __uint128_t) in Clang or GCC."
#endif

namespace fast_isqrt {

using uint128_t = unsigned __int128;

// -----------------------------------------------------------------------------
// Bit Utilities
// -----------------------------------------------------------------------------
[[nodiscard]] static inline int clz128(uint128_t x) noexcept {
    return __builtin_clzg(x,128);
}

// -----------------------------------------------------------------------------
// 64-bit Integer Square Root
// -----------------------------------------------------------------------------
[[nodiscard]] inline uint64_t isqrt64(uint64_t n) noexcept {    
    // FPU 経由の初期推測
    uint64_t x = static_cast<uint64_t>(std::sqrt(static_cast<double>(n)));
    
    // UINT32_MAX ガードにより (x + 1)^2 の 64-bit オーバーフローを完全に防御
    if ((x + 1) * (x + 1) <= n && x < UINT32_MAX) [[unlikely]] {
        x++;
    }
    if (x * x > n || x > UINT32_MAX) [[unlikely]] {
        x--;
    }
    
    return x;
}

struct alignas(16) IsqrtResult {
    uint64_t root; // x
    uint64_t sq;   // x * x
};

[[nodiscard]] inline IsqrtResult isqrt64_with_sq(uint64_t n) noexcept {
    // 1. FPU 経由の初期推測
    uint64_t x = static_cast<uint64_t>(std::sqrt(static_cast<double>(n)));
    uint64_t sq = x * x;

    // 2. インクリメント補正
    // (x + 1)^2 = sq + (2*x + 1)
    // x < UINT32_MAX ガードによって (x+1)^2 の 64bit ラップアラウンドを確実に防ぐ
    uint64_t next_sq = sq + ((x << 1) | 1);
    if (next_sq <= n && x < UINT32_MAX) [[unlikely]] {
        x++;
        sq = next_sq;
    } 
    // 3. デクリメント補正
    else if (sq > n || x > UINT32_MAX) [[unlikely]] {
        sq -= ((x << 1) - 1);
        x--;
    }

    return IsqrtResult{x, sq};
}
// -----------------------------------------------------------------------------
// 128-bit Integer Square Root (Taylor Expansion + 2^2a Scaling)
// -----------------------------------------------------------------------------
[[nodiscard]] inline uint128_t isqrt128(uint128_t n) noexcept {

    int lz = clz128(n);
    if (lz >= 64) [[unlikely]] {
        return isqrt64(static_cast<uint64_t>(n));
    }

    int a = lz >> 1;
    uint128_t scaled_n = n << (a << 1);
    uint64_t u = static_cast<uint64_t>(scaled_n >> 64);
    // uint64_t isqu = isqrt64(u);
    auto [isqu, isqu_sq] = isqrt64_with_sq(u);

    // テイラー展開1次の項まで と解釈してもいい
    // ニュートン法1step と解釈してもいい

    //uint128_t x = ((static_cast<uint128_t>(isqu << 32)) + ((((u - isqu * isqu) << 31) | (static_cast<uint64_t>(scaled_n) >> 33)) / isqu)) >> a;
    //uint128_t x = (static_cast<uint128_t>(isqu << (32-a))) + (((((u - isqu * isqu) << 31) | (static_cast<uint64_t>(scaled_n) >> 33)) / isqu) >> a);
    uint128_t x = (static_cast<uint128_t>(isqu << (32-a))) + (((((u - isqu_sq) << 31) | (static_cast<uint64_t>(scaled_n) >> 33)) / isqu) >> a);

    // UINT64_MAX ガード付き最終精度補正ループ
    if ((x + 1) * (x + 1) <= n && x < UINT64_MAX) [[unlikely]] {
        x++;
    }
    if (x * x > n || x > UINT64_MAX) [[unlikely]] {
        x--;
    }
    
    return x;
}

} // namespace fast_isqrt

#endif // FAST_ISQRT_HPP