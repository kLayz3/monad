#pragma once

/* To have rust-like primitive types. Yay. */

#include <cstdint>

#define MND_RUSTIFY_TYPE(N) \
	using u##N = uint##N##_t; \
	using i##N = int##N##_t;

/* An indicator to export the primitive typedefs into the global ns. */
#define MND_EXPORT_RUSTIFIED_PRIMITIVES

#if !defined(MND_EXPORT_RUSTIFIED_PRIMITIVES)
namespace mnd {
#endif

MND_RUSTIFY_TYPE( 8)
MND_RUSTIFY_TYPE(16)
MND_RUSTIFY_TYPE(32)
MND_RUSTIFY_TYPE(64)
using f64 = double;
using f32 = float;

#undef MND_RUSTIFY_TYPE

#if !defined(MND_EXPORT_RUSTIFIED_PRIMITIVES)
}
#endif
