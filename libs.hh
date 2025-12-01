#pragma once
#include <cstdint>

#include <cstdlib>  // std::abort
#include <iostream> // std::cerr
#include <string>   // std::string
#include <cstdarg>  // va_list

#if defined(__has_include) && __has_include (<boost/stacktrace.hpp>)
#   define _HAS_BOOST_INCLUDE
#	include <boost/stacktrace.hpp>
#endif

#define _STR_IMPL_MND(x) #x
#define _TO_STRING_MND(x) _STR_IMPL_MND(x)

#ifdef __unix__
#define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#elif defined(__WIN32) || defined(WIN32)
#define __FILENAME__  (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
#define __FILENAME__ __FILE__
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

// High intensty text.
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

// Reset.
#define KRNM "\e[0m"
#define KNRM "\e[0m"
#define COLOR_RESET "\e[0m"

#define BOLD "\e[1m"
#define EBOLD(x) BOLD #x KNRM
#define EMPH(x) KBH_YEL #x KNRM
#define EMPH1(x) KBH_CYN #x KNRM

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
}

#define YELL(...) \
	do { \
		fprintf(stderr, KGRN "%s" KNRM ":" KCYN "%d" KNRM " => ", __FILENAME__, __LINE__); \
		fprintf(stderr, KBH_RED); fprintf(stderr, __VA_ARGS__); fprintf(stderr, KNRM); \
	} while(0);

#define WARN_ST(...) \
	WARN(__VA_ARGS__) \
	fputc('\n', stderr); \
	std::cerr << boost::stacktrace::stacktrace(); \

#define WARN(...) \
	do { \
		fprintf(stderr, KGRN "%s" KNRM ":" KCYN "%d" KNRM " => ", __FILENAME__, __LINE__); \
		fprintf(stderr, __VA_ARGS__); \
	} while (0);

#define WARN_ASYNC(...) \
	do { \
		const char* msg_ = mnd::msg("\n" KGRN "%s" KNRM ":" KCYN "%d" KNRM " => ", __FILENAME__, __LINE__); \
		const char* msg_v_ = mnd::msg(__VA_ARGS__); \
		write(STDERR_FILENO, msg_, strlen(msg_)); \
		write(STDERR_FILENO, msg_v_, strlen(msg_v_)); \
	} while (0);

#define println(...) \
	do { \
		printf(__VA_ARGS__); \
		printf("\n"); \
	} while(0)

#ifdef _HAS_BOOST_INCLUDE
#define ERROR(...) do { \
	YELL(__VA_ARGS__); \
	fputc('\n', stderr); \
	std::cerr << boost::stacktrace::stacktrace(); \
	std::abort(); \
} while (0)
#else
#define ERROR(...) do { \
	YELL(__VA_ARGS__); \
	fputc('\n', stderr); \
	std::abort(); \
} while (0)
#endif

#define FOR(i, N) \
	for(int i=0; i < (N); ++i)

#define RUSTIFY_TYPE(N) \
	using u##N = uint##N##_t; \
	using i##N = int##N##_t;

RUSTIFY_TYPE( 8)
RUSTIFY_TYPE(16)
RUSTIFY_TYPE(32)
RUSTIFY_TYPE(64)
