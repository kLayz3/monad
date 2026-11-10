#pragma once

/* To have rust-like primitive types. Yay. */

#include <cstdint>
#include <limits>

#define MND_RUSTIFY_INTEGRAL_TYPE(N) \
	using u##N = std::uint##N##_t; \
	using i##N = std::int##N##_t;

#define MND_RUSTIFY_INTEGRAL_LITERAL(N) \
	inline constexpr i##N operator""_i##N(unsigned long long int value) noexcept { \
		return static_cast<i##N>(value); \
	} \
	inline constexpr u##N operator""_u##N(unsigned long long int value) noexcept { \
		return static_cast<u##N>(value); \
	}

/* By default, typedefs and literal ops are in global ns.
 * Define MND_NAMESPACE_RUSTIFIED_PRIMITIVES _before_ including the header 
 * to box the typedefs in mnd:: and ops in mnd::literals instead. */
#if defined(MND_NAMESPACE_RUSTIFIED_PRIMITIVES)
namespace mnd {
#endif

MND_RUSTIFY_INTEGRAL_TYPE( 8)
MND_RUSTIFY_INTEGRAL_TYPE(16)
MND_RUSTIFY_INTEGRAL_TYPE(32)
MND_RUSTIFY_INTEGRAL_TYPE(64)

using f64 = double;
using f32 = float;

static_assert(sizeof(f64) == 8, "Must hold for the implementation to be valid.");
static_assert(sizeof(f32) == 4, "Must hold for the implementation to be valid.");
static_assert(std::numeric_limits<f64>::is_iec559);
static_assert(std::numeric_limits<f32>::is_iec559);

static_assert(std::numeric_limits<f64>::radix == 2);
static_assert(std::numeric_limits<f64>::digits == 53);
static_assert(std::numeric_limits<f64>::max_exponent == 1024);

static_assert(std::numeric_limits<f32>::radix == 2);
static_assert(std::numeric_limits<f32>::digits == 24);
static_assert(std::numeric_limits<f32>::max_exponent == 128);


#if defined(MND_NAMESPACE_RUSTIFIED_PRIMITIVES)
namespace literals {
#endif

MND_RUSTIFY_INTEGRAL_LITERAL( 8)
MND_RUSTIFY_INTEGRAL_LITERAL(16)
MND_RUSTIFY_INTEGRAL_LITERAL(32)
MND_RUSTIFY_INTEGRAL_LITERAL(64)

inline constexpr f32 operator""_f32(long double value) noexcept {
	return static_cast<f32>(value);
}
inline constexpr f32 operator""_f32(unsigned long long int value) noexcept {
	return static_cast<f32>(value);
}
inline constexpr f64 operator""_f64(long double value) noexcept {
	return static_cast<f64>(value);
}
inline constexpr f64 operator""_f64(unsigned long long int value) noexcept {
	return static_cast<f64>(value);
}
#if defined(MND_NAMESPACE_RUSTIFIED_PRIMITIVES)
} // namespace literals
} // namespace mnd
#endif

#undef MND_RUSTIFY_INTEGRAL_TYPE
#undef MND_RUSTIFY_INTEGRAL_LITERAL
