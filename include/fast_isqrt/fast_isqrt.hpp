#ifndef FAST_ISQRT_HPP
#define FAST_ISQRT_HPP

#include <cstdint>
#include <cmath>
#include <array>
#include <type_traits>

#ifndef __SIZEOF_INT128__
#error "fast_isqrt requires 128-bit integer support (__int128_t / __uint128_t) in Clang or GCC."
#endif

namespace fast_isqrt {

using uint128_t = unsigned __int128;

// -----------------------------------------------------------------------------
// Bit Utilities
// -----------------------------------------------------------------------------
static inline int clz128(uint128_t x) noexcept {
    return __builtin_clzg(x,128);
}

// -----------------------------------------------------------------------------
// 64-bit Integer Square Root
// -----------------------------------------------------------------------------
inline uint64_t isqrt64(uint64_t n) noexcept {    
    // FPU 経由の初期推測
    uint64_t x = static_cast<uint64_t>(std::sqrt(static_cast<double>(n)));
    
    // UINT32_MAX ガードにより (x + 1)^2 の 64-bit オーバーフローを完全に防御
    while ((x + 1) * (x + 1) <= n && x < UINT32_MAX) x++;
    while (x * x > n || x > UINT32_MAX) x--;
    
    return x;
}

// -----------------------------------------------------------------------------
// 128-bit Integer Square Root (Taylor Expansion + 2^2a Scaling)
// -----------------------------------------------------------------------------
inline uint128_t isqrt128(uint128_t n) noexcept {

    int lz = clz128(n);
    if (lz >= 64) return isqrt64(static_cast<uint64_t>(n));

    int a = lz >> 1;
    uint128_t scaled_n = n << (a << 1);
    uint64_t u = static_cast<uint64_t>(scaled_n >> 64);
    uint64_t isqu = isqrt64(u);

    // テイラー展開1次の項まで と解釈してもいい
    // ニュートン法1step と解釈してもいい
    uint128_t x = ((static_cast<uint128_t>(isqu) << 32) + ((((u - isqu * isqu) << 31) | (static_cast<uint64_t>(scaled_n) >> 33)) / isqu)) >> a;

    // UINT64_MAX ガード付き最終精度補正ループ
    while ((x + 1) * (x + 1) <= n && x < UINT64_MAX) x++;
    while (x * x > n || x > UINT64_MAX) x--;
    
    return x;
}

} // namespace fast_isqrt

#endif // FAST_ISQRT_HPP