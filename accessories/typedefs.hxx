#pragma once

/* Small header to declare monad types and macros without tagging along the
 * entire header with it. Useful for small libs that do not wish to carry ROOT/boost dependency. */

#include <cstdarg>
#include <cstdio>
#include <cstdlib>
#include <cstring>
#include <stdexcept>
#include <string>

#include "primitives.hxx"

#if !defined(__has_include)
#	error "__has_include macro not defined by the standard compiler? Update the GCC/Clang/MSVC please!"
#endif

#if __cplusplus >= 202000L
#	include <span>
	namespace mnd {
		template<typename T>
		using span = std::span<T>;
	}
#elif __has_include("boost/beast/core/span.hpp")
#	include "boost/beast/core/span.hpp"
	namespace mnd {
		template<typename T>
		using span = boost::beast::span<T>;
	}
#else
#	error "Neither C++20 given, nor boost span library found. Cannot proceed."
#endif

#ifndef __FILE_NAME__
#	ifdef __unix__
#		define __FILE_NAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#	elif defined(__WIN32) || defined(WIN32)
#		define __FILE_NAME__  (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#	else
#		define __FILE_NAME__ __FILE__
#	endif
#endif

// Regular text.
#define KBLK "\e[0;30m"
#define KRED "\e[0;31m"
#define KGRN "\e[0;32m"
#define KYEL "\e[0;33m"
#define KBLU "\e[0;34m"
#define KMAG "\e[0;35m"
#define KCYN "\e[0;36m"
#define KWHT "\e[0;37m"

// Regular bold text.
#define KB_BLK "\e[1;30m"
#define KB_RED "\e[1;31m"
#define KB_GRN "\e[1;32m"
#define KB_YEL "\e[1;33m"
#define KB_BLU "\e[1;34m"
#define KB_MAG "\e[1;35m"
#define KB_CYN "\e[1;36m"
#define KB_WHT "\e[1;37m"

// Regular underline text.
#define KU_BLK "\e[4;30m"
#define KU_RED "\e[4;31m"
#define KU_GRN "\e[4;32m"
#define KU_YEL "\e[4;33m"
#define KU_BLU "\e[4;34m"
#define KU_MAG "\e[4;35m"
#define KU_CYN "\e[4;36m"
#define KU_WHT "\e[4;37m"

// High intensity text.
#define KH_BLK "\e[0;90m"
#define KH_RED "\e[0;91m"
#define KH_GRN "\e[0;92m"
#define KH_YEL "\e[0;93m"
#define KH_BLU "\e[0;94m"
#define KH_MAG "\e[0;95m"
#define KH_CYN "\e[0;96m"
#define KH_WHT "\e[0;97m"

// Bold high intensity text.
#define KBH_BLK "\e[1;90m"
#define KBH_RED "\e[1;91m"
#define KBH_GRN "\e[1;92m"
#define KBH_YEL "\e[1;93m"
#define KBH_BLU "\e[1;94m"
#define KBH_MAG "\e[1;95m"
#define KBH_CYN "\e[1;96m"
#define KBH_WHT "\e[1;97m"

// Bright backgrounds.
#define BKH_BLK "\e[100m"
#define BKH_RED "\e[101m"
#define BKH_GRN "\e[102m"
#define BKH_YEL "\e[103m"
#define BKH_BLU "\e[104m"
#define BKH_MAG "\e[105m"
#define BKH_CYN "\e[106m"
#define BKH_WHT "\e[107m"

// Reset.
#define KRNM "\e[0m"
#define KNRM "\e[0m"
#define COLOR_RESET "\e[0m"
#define KBLINK "\e[5m"

#define BOLD "\e[1m"
#define EBOLD(x) BOLD #x KNRM
#define EMPH(x) KBH_YEL #x KNRM
#define EMPH1(x) KBH_CYN #x KNRM
#define EMPH2(x) KBH_RED #x KNRM

#define _MND_STRINGIFY_IMPL(x) #x
#define MND_STRINGIFY(x) _MND_STRINGIFY_IMPL(x)
#define MND_FILE_LINE_STR \
	KGRN __FILE_NAME__ KNRM ":" KCYN MND_STRINGIFY(__LINE__) KNRM

#define MND_EMPTY_MACRO(...)

#define MND_RGB_COL(r, g, b) \
       "\x1b[38;2;" \
       MND_STRINGIFY(r) ";" \
       MND_STRINGIFY(g) ";" \
       MND_STRINGIFY(b) "m"
#define MND_RGB_BOLD_COL(r, g, b) \
      BOLD MND_RGB_COL(r, g, b)

#define MND_RGB_BCKGRND_COL(r, g, b) \
       "\x1b[48;2;" \
       MND_STRINGIFY(r) ";" \
       MND_STRINGIFY(g) ";" \
       MND_STRINGIFY(b) "m"

#if defined(__clang__) || defined(__GNUC__)
	#define MND_FORCE_INLINE inline __attribute__((always_inline))
	#define MND_NEVER_INLINE __attribute__((noinline))
#elif defined(_MSC_VER)
	#define MND_FORCE_INLINE __forceinline
	#define MND_NEVER_INLINE __declspec(noinline)
#else
	#define MND_FORCE_INLINE inline
	#define MND_NEVER_INLINE
#endif

#if defined(__GNUC__) || defined(__clang__)
	#define MND_LIKELY(expr)   __builtin_expect(static_cast<bool>(expr), 1)
	#define MND_UNLIKELY(expr) __builtin_expect(static_cast<bool>(expr), 0)
#else
	#define MND_LIKELY(expr)   (expr)
	#define MND_UNLIKELY(expr) (expr)
#endif

/* ROOT-like Form() w/o dependency. */
namespace mnd {
inline const char* msg(const char* fmt, ...) {
	thread_local std::string buffer;
	buffer.clear();

	/* Determine the size needed. */
	va_list args;
	va_start(args, fmt);
	int size = std::vsnprintf(nullptr, 0, fmt, args);
	va_end(args);

	if(size < 0) return "";  // formatting error?

	/* Resize buffer and actually write the formatted string. */
	buffer.resize(size + 1);

	va_start(args, fmt);
	std::vsnprintf(buffer.data(), buffer.size(), fmt, args);
	va_end(args);

	/* Remove the trailing null. */
	buffer.pop_back();

	return buffer.c_str();
}

enum class BinaryOpt : i32 { No = 0, Yes = 1 };

} // namespace mnd

#define WARN(...) \
	do { \
		fprintf(stderr, KGRN "%s" KNRM ":" KCYN "%d" KNRM " => ", __FILE_NAME__, __LINE__); \
		fprintf(stderr, __VA_ARGS__); \
	} while(0)

#define YELL(...) \
	do { \
		fprintf(stderr, KGRN "%s" KNRM ":" KCYN "%d" KNRM " => ", __FILE_NAME__, __LINE__); \
		fprintf(stderr, KBH_RED); fprintf(stderr, __VA_ARGS__); fprintf(stderr, KNRM); \
	} while(0)


#define ERROR(...) \
	do { \
		YELL(__VA_ARGS__); \
		fputc('\n', stderr); \
		std::abort(); \
	} while(0)

#define MND_THROW(...) \
	do { \
		std::string what = ::mnd::msg("\n" KGRN "%s" KNRM ":" KCYN "%d" KNRM \
			" => " MND_RGB_COL(252,110,242), __FILE_NAME__, __LINE__); \
		what += ::mnd::msg(__VA_ARGS__); \
		what += KNRM; \
		throw std::runtime_error(what); \
	} while(0)
