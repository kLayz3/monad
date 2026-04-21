/*
 * This file is part of MONAD - Modular Object/NTuple Analysis Dataflow.
 *
 * Copyright (C) 2025  Martin Bajzek  <m.bajzek@gsi.de>
 *
 * This library is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * This library is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with this library.  If not, see <https://www.gnu.org/licenses/> 
 * or write to the Free Software Foundation, Inc., 51 Franklin Street, 
 * Fifth Floor, Boston, MA  02110-1301  USA
 */

#ifndef __MONAD_INCLUDE_HXX__
#define __MONAD_INCLUDE_HXX__

#include <cstdint>
#include <cstdarg> 
#include <cstdlib>
#include <cstdio>

#include <iostream>
#include <string>
#include <chrono>
#include <sstream>
#include <type_traits>
#include <utility>
#include <variant>
#include <vector>
#include <fstream>
#include <typeinfo>
#include <optional>
#include <filesystem>
#include <unordered_map>
#include <tuple>
#include <atomic>
#include <thread>

#include "TTree.h"
#include "TKey.h"
#include "RtypesCore.h"
#include "TDirectory.h"
#include "TNamed.h"
#include "TFile.h"
#include "TH1.h"
#include "TROOT.h"

#if !defined(__has_include)
#	error "__has_include macro not defined by the standard compiler? Update the GCC/Clang/MSVC please!"
#endif

#if __has_include(<immintrin.h>)
#   define __HAS_SMALL_INTEL_SPIN
#	include <immintrin.h>
#endif

#if !defined(ROOT_VERSION)
#	error "ROOT_VERSION macro not found in the default build? Either old ROOT is used or `ROOT/RVersion.hxx` file missing!"
#elif ROOT_VERSION_CODE < ROOT_VERSION(6,34,0)
#	error "The MONAD framework cannot work with ROOT versions earlier than 63404 (6.34). "
#endif

#include "ROOT/RNTuple.hxx"
#include "ROOT/RNTupleReader.hxx"
#include "ROOT/RNTupleWriter.hxx"
#include "ROOT/RNTupleFillContext.hxx"
#include "ROOT/RNTupleModel.hxx"
#include "ROOT/RNTupleParallelWriter.hxx"

#if ROOT_VERSION_CODE < ROOT_VERSION(6,36,0)
	namespace RExp = ROOT::Experimental;
#else /* >= 63600 */
	namespace RExp = ROOT;
#endif
namespace RExp2 = ROOT::Experimental;

#if __has_include("boost/stacktrace.hpp")
#   define _HAS_BOOST_INCLUDE
#	include "boost/stacktrace.hpp"
#	include "boost/lockfree/spsc_queue.hpp"
#	include "boost/lockfree/policies.hpp"
#else
#	error "Boost packages not found in the environment?"
#endif

#ifndef _MSC_VER
#	include <cxxabi.h>
#endif
#if __has_include(<new>)
#	include <new>
#endif

/* Keep this header as an option, not a dependency */
#if __has_include("indicators/indicators.hh")
#	define __HAS_INDICATORS
#	include "indicators/indicators.hh"
#endif

#define _STR_IMPL_MND(x) #x
#define _TO_STRING_MND(x) _STR_IMPL_MND(x)

#ifdef __unix__
#	define __FILENAME__ (strrchr(__FILE__, '/') ? strrchr(__FILE__, '/') + 1 : __FILE__)
#elif defined(__WIN32) || defined(WIN32)
#	define __FILENAME__  (strrchr(__FILE__, '\\') ? strrchr(__FILE__, '\\') + 1 : __FILE__)
#else
#	define __FILENAME__ __FILE__
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
#define EMPH2(x) KBH_RED #x KNRM

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
#	define ERROR(...) do { \
		YELL(__VA_ARGS__); \
		fputc('\n', stderr); \
		std::cerr << boost::stacktrace::stacktrace(); \
		std::abort(); \
	} while (0)
#else
#	define ERROR(...) do { \
		YELL(__VA_ARGS__); \
		fputc('\n', stderr); \
		std::abort(); \
	} while (0)
#endif

#define MND_RUSTIFY_TYPE(N) \
	using u##N = uint##N##_t; \
	using i##N = int##N##_t;

/* An indicator to export certain free functions/aliases to global namespace.
 * It can confuse ADL, in that case just disable it. */
#define MND_POLLUTE_G_NAMESPACE

#if !defined(MND_POLLUTE_G_NAMESPACE)
namespace mnd {
#endif

MND_RUSTIFY_TYPE( 8)
MND_RUSTIFY_TYPE(16)
MND_RUSTIFY_TYPE(32)
MND_RUSTIFY_TYPE(64)

template<typename T, std::size_t N>
void Add(std::array<T, N>& lhs, const std::array<T, N>& rhs);
template<typename T>
void Add(std::vector<T>& lhs, const std::vector<T>& rhs);

#if !defined(MND_POLLUTE_G_NAMESPACE)
}
#endif

namespace mnd {

#if defined(__HAS_INDICATORS)
inline void 
PrintProgress(indicators::ProgressBar& bar, const u64 n_entry, const u64 max_entries, const u64 step = 250) noexcept {
	static u64 n_entry_called = 0;
	if(n_entry - n_entry_called < step) return;
	bar.set_progress( (n_entry*100) / max_entries );

	n_entry_called = n_entry;
}
#endif

template<typename T>  
void QuickSwap(std::vector<T>& v, int i, int j) noexcept {
	if(!v.size()) return;
    std::swap(v[i], v[j]);
}

template<typename T>  
void QuickErase(std::vector<T>& v, int i) noexcept {
    std::swap(v[i], v.back());
    v.pop_back();
}

template<typename T>
void Append(std::vector<T>& dst, const std::vector<T>& src) noexcept {
	dst.reserve(dst.size() + src.size());
	dst.insert(dst.end(), src.begin(), src.end());
}

template<typename T>
void Append(std::vector<T>& dst, std::vector<T>&& src) noexcept {
	if(&dst == &src) return;
	dst.reserve(dst.size() + src.size());
	dst.insert(dst.end(),
		std::make_move_iterator(src.begin()),
		std::make_move_iterator(src.end()));
	/* src vector left in undefined state. */
}

/* Filter a vector in-place based on the predicate `p`. 
 * If `p` evaluates to true, element is kicked out. */
template<typename T, typename Predicate>
void Erase(std::vector<T>& v, Predicate p) noexcept ( 
	std::is_nothrow_invocable_v<Predicate&, const T&> && std::is_nothrow_move_assignable_v<T> 	
	/*^^^ I guess...? How else to check if the underlying block is noexcept? */
) {
	v.erase(std::remove_if(v.begin(), v.end(), std::move(p)), v.end());
}

/* ------------------------- */
template<typename T>
T rround(double x) noexcept { return static_cast<T>(x + 0.5); }

template<typename T, typename U>
bool IsInside(const T& value, const std::array<U,2>& bounds) {
	static_assert (
		std::is_convertible_v<decltype(std::declval<const U&>() <= std::declval<const T&>()), bool> &&
		std::is_convertible_v<decltype(std::declval<const T&>() <  std::declval<const U&>()), bool>,
		"T and U must support comparison operators T <= U and U < T."
	);
	return bounds[0] <= value and value < bounds[1];
}


using std::chrono::duration_cast;
using std::chrono::seconds;
using std::chrono::milliseconds;

#define timeNow() std::chrono::high_resolution_clock::now()
struct TimePoint {
	std::chrono::high_resolution_clock::time_point t;
	std::string tag;
	TimePoint() : t(timeNow()), tag("?") {};
	TimePoint(std::string s) : t(timeNow()), tag(s) {};
};
#undef timeNow

enum TimingVariant : u64 {
	kMINUTE = 1,
	kSECOND = 60,
	kMILLISECOND = 60'000,
	kMICROSECOND = 60'000'000,
};

template<TimingVariant E = kMILLISECOND>
void PrintElapsed(const TimePoint& end, const TimePoint& start) {
	using us = std::chrono::microseconds;
	using UT = std::underlying_type_t<TimingVariant>;

	static const char* mode[] = {"us", "ms", "sec", "min"};
	int mode_i = 0;
	double elapsed;
	if constexpr(E == kMINUTE) {
		elapsed = (std::chrono::duration_cast<us>(end.t - start.t).count()) * static_cast<double>(kMINUTE) / static_cast<UT>(kMICROSECOND);
		mode_i = 3;
	}
	else if constexpr(E == kSECOND) {
		elapsed = (std::chrono::duration_cast<us>(end.t - start.t).count()) * static_cast<double>(kSECOND) / static_cast<UT>(kMICROSECOND);
		if(elapsed > 60) {
			elapsed *= static_cast<double>(kMINUTE) / static_cast<UT>(kSECOND);
			mode_i = 3;
		} else 
			mode_i = 2;
	}
	else if constexpr(E == kMILLISECOND) {
		elapsed = (std::chrono::duration_cast<us>(end.t - start.t).count()) * static_cast<double>(kMILLISECOND) / static_cast<UT>(kMICROSECOND);
		mode_i = 1;
	}
	else if constexpr(E == kMICROSECOND) {
		elapsed = (std::chrono::duration_cast<us>(end.t - start.t).count());
		mode_i = 0;
	}
	printf("Elapsed time from " EMPH1(%s) " to " EMPH1(%s) ": " EMPH(%.1f) " %s\n", start.tag.c_str(), end.tag.c_str(), elapsed, mode[mode_i]);
}

template<TimingVariant E = kMILLISECOND>
void PrintElapsed(const std::vector<TimePoint>& v) {
	if(v.size() < 2) return;
	PrintElapsed<E>(v[v.size()-1], v[v.size() - 2]);
}

template<TimingVariant E = kMILLISECOND>
void PrintElapsed(std::vector<TimePoint>&& v) {
	if(v.size() < 2) return;
	printf("Total execution time: ");
	PrintElapsed<E>(v.back(), v.front());
}

inline void __concat_impl__(std::ostringstream& ) {}

template<typename T, typename... Args>
void __concat_impl__(std::ostringstream& oss, T&& first, Args&&... args) {
	oss << std::forward<T>(first);
	__concat_impl__(oss, std::forward<Args>(args)... );
}

/**
 * Concatenates bunch of string arguments which can be lvalues, statics, etc. and returns an owned std::string. 
 */
template<typename... Args>
std::string sstrcat(Args&&... args) {
	std::ostringstream oss;
	__concat_impl__(oss, std::forward<Args>(args)... );
	return oss.str();
}

/* ----- std::variant trickery. ----- */
using Empty = std::monostate;

template<typename T>
using Maybe = std::optional<T>;

inline constexpr auto None = std::nullopt;

template<typename... Ts>
using Variant = std::variant<Empty, Ts...>;

template<typename... Ts>
constexpr bool IsEmpty(const Variant<Ts...>& v) {
	return v.valueless_by_exception() or std::holds_alternative<Empty>(v);
}

enum class BinaryOpt : i32 { No = 0, Yes = 1 };

/**
 * Apply 'std::visit' https://en.cppreference.com/w/cpp/utility/variant/visit2.html
 * to the mnd::Variant type. Throws if the 'mnd::Empty' type is encountered. 
 */
template<typename Visitor, typename Var>
decltype(auto) visit_non_empty(Visitor&& vs, Var&& /* Variant&& */ var) {
	return std::visit (
		[&](auto&& value) -> decltype(auto) {
			using T = std::decay_t<decltype(value)>;
			if constexpr(std::is_same_v<T, Empty>)
				throw std::runtime_error("mnd::visit_non_empty except: `Empty` type encountered.");
			else 
				return std::invoke(std::forward<Visitor>(vs),
					std::forward<decltype(value)>(value));
		},
		std::forward<Var>(var)
	);
}

template<typename Tuple, typename Callable, std::size_t... Is>
void _for_each_in_tuple_impl(Tuple&& t, Callable&& f, std::index_sequence<Is...>) {
	(..., f (std::get<Is>(std::forward<Tuple>(t))));
}
template<typename Tuple, typename Callable>
void for_each_in_tuple(Tuple&& t, Callable&& f) {
	constexpr std::size_t N = std::tuple_size_v<std::decay_t<Tuple>>;
	_for_each_in_tuple_impl(std::forward<Tuple>(t), std::forward<Callable>(f), 
		std::make_index_sequence<N>{});
}

template<typename T>
struct is_an_array : std::false_type {};
template<typename T, size_t N>
struct is_an_array<T[N]> : std::true_type { using value_type = T; static constexpr size_t size = N; };
template<typename T, size_t N>
struct is_an_array<std::array<T,N>> : std::true_type { using value_type = T; static constexpr size_t size = N; };
template<typename T>
constexpr bool is_an_array_v = is_an_array<T>::value;

template<typename T>
struct ToStdArray { using type = T; };
template<typename T, std::size_t N>
struct ToStdArray<T[N]> { using type = std::array<typename ToStdArray<T>::type, N>; };
template<typename T>
using ToStdArray_t = typename ToStdArray<T>::type;

template<typename T, typename = void>
struct has_clean_noexcept : std::false_type {};
template<typename T>
struct has_clean_noexcept <T, 
	std::void_t<decltype(std::declval<T&>().Clean())>
> : std::bool_constant <
		std::is_same_v<void, decltype(std::declval<T&>().Clean())> &&
		noexcept(std::declval<T&>().Clean())
	> {};

template<typename, typename = std::void_t<>>
struct has_value_type : std::false_type {};
template<typename U>
struct has_value_type<U, std::void_t<typename U::value_type>> : std::true_type {};

template<typename T>
struct is_std_array : std::false_type{};
template<typename T, std::size_t N>
struct is_std_array<std::array<T, N>> : std::true_type {};

template<typename T, typename = void>
struct has_clone : std::false_type {};
template<typename T>
struct has_clone<T, std::void_t<decltype(std::declval<const T&>().Clone())>> : std::true_type {};

template<typename T, typename = void>
struct has_setname : std::false_type {};
template<typename T>
struct has_setname<T, std::void_t<
	decltype( std::declval<T&>().SetName(std::declval<const char*>() ) )
>> : std::true_type {};

template<typename T, typename = void>
struct has_copy : std::false_type {};
template<typename T>
struct has_copy<T, std::void_t<
	decltype( std::declval<const T&>().Copy( std::declval<TObject&>() ) )
>> : std::true_type {};

template<typename T, typename = void>
struct has_set_directory : std::false_type {};
template<typename T>
struct has_set_directory<T, std::void_t<
	decltype( std::declval<const T&>().SetDirectory( std::declval<const char*>() ) )
>> : std::true_type {};

template<typename T, typename = void>
struct has_dyadic_add_ref : std::false_type {};
template<typename T>
struct has_dyadic_add_ref<T, std::void_t<
	decltype( std::declval<T&>().Add( std::declval<const T&>() ) )
>> : std::true_type {};

template<typename T, typename = void>
struct has_dyadic_add_ptr : std::false_type {};
template<typename T>
struct has_dyadic_add_ptr<T, std::void_t<
	decltype( std::declval<T&>().Add( std::declval<const T*>() ) )
>> : std::true_type {};

template<typename T, typename = void>
struct has_free_add_fn : std::false_type {};
template<typename T>
struct has_free_add_fn<T, std::void_t<
	decltype( Add( std::declval<T&>(), std::declval<const T&>() ) )
>> : std::true_type {};

template<typename T, typename = void>
struct has_add_unary_op : std::false_type {};
template<typename T>
struct has_add_unary_op<T, std::void_t<
	decltype( std::declval<T&>().operator+=(std::declval<const T&>() ) )
>> : std::true_type {};

template<typename T, typename = void>
struct has_free_mean_fn : std::false_type {};
template<typename T>
struct has_free_mean_fn<T, std::void_t<
	decltype( Mean( std::declval<T&>(), std::declval<const T&>() ) )
>> : std::true_type {};

template<typename T, typename = void>
struct has_add_div_unary_op : std::false_type {};
template<typename T>
struct has_add_div_unary_op<T, std::void_t <
	std::conjunction <
		decltype( std::declval<T&>().operator+=(std::declval<const T&>() ) ) ,
		decltype( std::declval<T&>().operator/=(std::declval<const int>() ) )
	>
>> : std::true_type {};

template<typename T, typename = void>
struct has_process_entry : std::false_type {};
template<typename T>
struct has_process_entry<T, std::void_t<
	decltype(std::declval<T&>().ProcessEntry())
>> : std::is_same <
		decltype(std::declval<T&>().ProcessEntry()),
		void
	> {};

template< template<typename...> typename Base, typename Derived>
struct _is_base_of_template_impl {
	template<typename... Ts>
	static constexpr std::true_type test(Base<Ts...>& );
	static constexpr std::false_type test(...);
	using type = decltype(test(std::declval<Derived&>()));
};

template< template<typename...> typename Base, typename Derived,
	typename Bare = std::remove_reference_t<std::remove_cv_t<Derived>>>
using is_base_of_template = typename _is_base_of_template_impl<Base, Bare>::type;

template<typename Tuple, std::size_t... Is>
auto zip_refs_impl(Tuple& t1, const Tuple& t2, std::index_sequence<Is...>) {
	using R = std::tuple <
		std::pair <
			std::add_lvalue_reference_t<std::tuple_element_t<Is, Tuple>>,
			std::add_lvalue_reference_t<std::add_const_t<std::tuple_element_t<Is, Tuple>>>
		>...
	>;
	
	return R {
		std::pair <
			std::add_lvalue_reference_t<std::tuple_element_t<Is, Tuple>>,
			std::add_lvalue_reference_t<std::add_const_t<std::tuple_element_t<Is, Tuple>>>
		> { std::get<Is>(t1), std::get<Is>(t2) }...
	};
}

template<typename Tuple>
auto zip_refs(Tuple& t1, const Tuple& t2) {
	constexpr std::size_t N = std::tuple_size_v<Tuple>;
	return zip_refs_impl( t1, t2, std::make_index_sequence<N>{} );
}

template<typename T>
constexpr bool is_pathlike_arg_v =
	std::is_base_of_v<std::ifstream, std::decay_t<T>> ||
	std::is_constructible_v<std::ifstream, T&&> ||
	std::is_constructible_v<std::string, T&&> ||
	std::is_constructible_v<std::filesystem::path, T&&>;

template<typename T,
	typename U = std::decay_t<T>,
	typename std::enable_if<is_pathlike_arg_v<T>>::type* = nullptr
> std::optional<std::ifstream> get_maybe_ifstream(T&& arg) {
	if constexpr(std::is_base_of_v<std::istream, U>) {
		if(arg.is_open()) return arg;
		else return {};
	}
	else if constexpr(
		std::is_constructible_v<std::ifstream, T&&> ||
		std::is_constructible_v<std::filesystem::path, T&&>
	) {
		auto f = std::ifstream(std::forward<T>(arg));
		if(f.is_open()) return f;
		else return {};
	}
	else if constexpr(std::is_constructible_v<std::string, T&&>) {
		auto f = std::ifstream(std::string(std::forward<T>(arg)));
		if(f.is_open()) return f;
		else return {};
	}
	else 
		static_assert(std::is_constructible_v<std::string, T&&>,
			"Type not constructible to ifstream or path-like type.");
}

template<typename T>	
bool is_file_readable(T&& arg) {
	return get_maybe_ifstream(std::forward<T>(arg)).has_value();
}

template<typename T,
	typename U = std::decay_t<T>
> std::optional<std::string> get_file_path(T&& arg) {
	static_assert(is_pathlike_arg_v<U>, "Type <T> must be path-like");
	if constexpr(std::is_base_of_v<std::istream, U>) 
		return {};
	else if constexpr(std::is_constructible_v<std::string, T&&>)
		return std::string(std::forward<T>(arg));
	else if constexpr(std::is_constructible_v<std::filesystem::path, T&&>)
		return std::filesystem::path(std::forward<T>(arg)).string();
}

template<typename T,
	typename U = std::remove_cv_t<std::remove_reference_t<T>>,
	std::size_t N = is_an_array<U>::size,
	char(*)[N % 2] = nullptr
> auto median(const T& sorted_arr) noexcept { return sorted_arr[N/2]; }

template<typename T,
	typename U = std::remove_cv_t<std::remove_reference_t<T>>,
	std::size_t N = is_an_array<U>::size,
	char(*)[!(N % 2)] = nullptr
> auto median(const T& sorted_arr) noexcept { return ( sorted_arr[N/2] + sorted_arr[N/2 - 1] ) / 2; }

template<typename T, typename... Ts>
constexpr auto min(T t, Ts... ts) noexcept {
	static_assert(std::is_arithmetic_v<
		std::remove_reference_t<T>
	> &&
	(std::is_arithmetic_v<
		std::remove_reference_t<Ts>
	> && ...), "Types passed must be arithmetic, basic types.");

	using R = std::common_type_t<T, Ts...>;
	R r = static_cast<R>(t);
	((r = std::min<R>(r, static_cast<R>(ts))), ...);
	return r;
}
template<typename T, typename... Ts>
constexpr auto max(T t, Ts... ts) noexcept {
	static_assert(std::is_arithmetic_v<
		std::remove_reference_t<T>
	> &&
	(std::is_arithmetic_v<
		std::remove_reference_t<Ts>
	> && ...), "Types passed must be arithmetic, basic types.");

	using R = std::common_type_t<T, Ts...>;
	R r = static_cast<R>(t);
	((r = std::max<R>(r, static_cast<R>(ts))), ...);
	return r;
}

template<typename T,
	typename U = std::remove_cv_t<std::remove_reference_t<T>>
> constexpr int FindIndex(const T& arr, const typename is_an_array<U>::value_type& val) noexcept {
	static_assert(is_an_array_v<U>, "Type T must be a C-style array or \'std::array<T,N>\'");
	constexpr std::size_t N = is_an_array<U>::size;
	for(int i=0; i < (int)N; ++i)
		if(arr[i] == val) return i;
	return -1;
}

template<typename T,
	typename U = std::remove_cv_t<std::remove_reference_t<T>>
> constexpr int len(const T& arr) noexcept {
	static_assert(is_an_array_v<U>, "Passed type must either be an array reference `T (&)[N]` or `std::array<T,N>&` .");
	return static_cast<int>(is_an_array<U>::size);	
}

/* Returns the name of the type passed, also adding 
 * ref-cv qualifiers. Demangles templated types, too :-) */
template<typename T,
	typename U = std::decay_t<T>	
> std::string type_name() {
	std::unique_ptr<char, void(*)(void*)> own(
#ifndef _MSC_VER
		abi::__cxa_demangle(typeid(U).name(), nullptr,
			nullptr, nullptr),
#else
		nullptr,
#endif
		std::free
	);
	std::string r = (own) ? own.get() : typeid(U).name();
	if constexpr(std::is_const_v<T>)
		r += " const";
	if constexpr(std::is_volatile_v<T>)
		r += " volatile";
	if constexpr(std::is_lvalue_reference_v<T>)
		r += "&";
	else if constexpr(std::is_rvalue_reference_v<T>)
		r += "&&";
	return r;
}

#define _SELF_TYPE_CSTR \
	mnd::type_name<typename std::remove_reference<decltype(*this)>::type>().c_str()

/* Sometimes cursor can be hidden mid execution,
 * if the program dies due to a system signal,
 * execute this to bring it back. Only POSIX async-safe 
 * calls are allowed. NOTE: this *might* disable standard ROOT
 * stack trace dump in case of SIGSEGV catch. Unmap it in this case.
 * and in Linux shell execute 'tput cnorm' to get the cursor back. */
inline void sig_callback_handler(int signum) {
	const char show[] = "\x1b[?25h";
	const char nl   = '\n';
	write(STDERR_FILENO, &nl, 1);
	write(STDERR_FILENO, show, sizeof show - 1);
	WARN_ASYNC("Caught abort/seg signal [%d].\n", signum);
	_exit(128 + signum);
}

constexpr std::size_t CL = 
#if defined(__cpp_lib_hardware_interference_size) && defined(__clang__)
	std::hardware_destructive_interference_size
#elif defined(__x86_64__)
	64 // fallback, for sure.
#else
	64 // Suppose.
#endif
	;

} // namespace mnd

struct TOnceBase {
	TOnceBase() { TH1::AddDirectory(kFALSE); }
	TOnceBase(const char* name)  : _name(name) {}
	TOnceBase(std::string name) : _name(std::move(name)) {}

	TOnceBase(const TOnceBase& ) = default;
	TOnceBase& operator=(const TOnceBase& ) = default;
	TOnceBase(TOnceBase&& ) noexcept = default;
	TOnceBase& operator=(TOnceBase&& ) noexcept = default;
	virtual ~TOnceBase() = default; 

	virtual Int_t Write(TFile* file = nullptr, const char* target = "") = 0;
	virtual void* Load(TFile* file, const char* target = "") = 0;
	virtual void Collect(const TOnceBase& ) = 0;
	virtual	std::unique_ptr<TOnceBase> Clone() const = 0;

	inline virtual void SetName(std::string name, const char* title = "") { 
		(void)title;
		_name = std::move(name); 
	}

	inline const char* GetName() const noexcept { return this->_name.c_str(); }

protected:
	std::string _name;
};

/**
 * A wrapper type for objects (such as TVectorD, THXX, TArray, TCutG, etc)
 * but also stl-containers such as std::vector<T>, std::array<T, N>, etc that might carry 
 * a name or maybe not. This type wraps them so that all of these types' instances carry a name,
 * which shall be serialized (once) into a `TFile`.
 * Unlike `TContainer`'s other parts which will be serialized row-wise (per-event) into the RNTuple.
 * Instances are differentiated by their unique string key `_name`.
 */

/* Different threads will have their own unique exclusive objects of this wrapper type (for writing),
 * e.g. TH1I's. Once their work is done, there has to be a way to collect and combine them into a single object,
 * that is to be then written into the output ROOT file.
 * Here we differentiate two such categories of types.
 * [1] Sum type - T must implement one of the following, checked in order:
 *         void T::Add(const T& rhs)   (such as 'TH1')
 *         void Add(T& lhs, const T& rhs) (free function; custom types; e.g. std::array<T>)
 * [2] Mean types - T must not be a Sum type, and must implement one of the following, checked in order:
 *         void Mean(T& lhs, const T& rhs)
 *         T& operator+=(const T& rhs) and T& operator/=(const int) (such as 'int', 'double')
 *
 *  The collector will sum up all the instances of identical Sum type, 
 *  and make a mean value of all the instances of identical Mean type.
 *  Collecting is done via a dyadic (binary) fold, e.g. for 8 instances of TOnce<T> objects:
 *
 *  a0, a1, a2, a3, a4, a5, a6, a7 
 *    \/      \/      \/      \/    
 *   a01,    a23,    a45,    a67
 *      \   /           \   /
 *      a0123     ,     a4567
 *           \         /
 *            a01234567
 *
 * It is done in-place, a0 now carries the summed/mean'ed up value, while the other (N-1)
 * instances carry possibly intermediate calculations, and should not be used. All together, the transformation 
 * looks like this:
 * ▶ a0 → a01234567
 * ▶ a1 → a1
 * ▶ a2 → a23
 * ▶ a3 → a3
 * ▶ a4 → a4567
 * ▶ a5 → a5
 * ▶ a6 → a67
 * ▶ a7 → a7
 * 
 * In the master Pool instantization, it's asserted that N forms a perfect log2: ∃n ∈ ℕ₀ | N == 2^n.
 *
 * To make your type `T` fit into this picture, usual procedure is to define a free function (non-templated;
 * most specific overload) `void Add(T&, const T&)` or `void Mean(T&, const T&)`,
 * Or you supply a custom collector (function pointer) to the constructor of this wrapper type.
 * Passing a function pointer instead will force that specific invocation instead of compiler-deduced one
 * There *might* still be a valid function declaration seen; e.g. on top I declare a free Add symbol for all 
 * std::array/vector. In case this gives a bad resolution (recursion fails at bottommost non-array type), 
 * then just define the type-specific no-op overload yourself (no template shenanigans).
 */

template<typename T> struct TContainer;

template<typename T>
struct alignas(mnd::CL) TOnce : TOnceBase {
	static_assert(! std::is_pointer_v<T>, "Must not pass pointer type (T*) to TOnce<T>");
	static_assert(! std::is_fundamental_v<T>, "Must not pass trivial type. Wrap it in e.g. TParameter<T> first.");
	static_assert(! std::is_void_v<T>, "Hello?");
	static_assert(! std::is_array_v<T>, "Must not pass raw C-style arrays. Pass an `std::array<T,N>` instead.");
	static_assert(! std::is_base_of_v<TTree, T>, "Cannot wrap the TTree type here.");
	static_assert(  std::is_copy_assignable_v<T>, "Type T must be copy-assignable");
	template<typename U> friend struct TContainer;

private:
	void (*_collector)(T&, const T&) = nullptr;
	T _internal;

public:
	using type = T;

	/* Case 1: T constructible with (const char*, Args&&...) */
	template<typename... Ts,
		typename std::enable_if<std::is_constructible_v<T, const char*, Ts...>>::type* = nullptr
	> TOnce(const char* name, Ts&&... args) : TOnceBase(name), 
		_internal(name, std::forward<Ts>(args)...) {}

	/* Case 2: T constructible with (Args&&...) but not (const char*, Args&&...) */
	template<typename... Ts,
		typename std::enable_if<
			!std::is_constructible_v<T, const char*, Ts...> && std::is_constructible_v<T, Ts...>
			>::type* = nullptr
		> TOnce(const char* name, Ts&&... args) : TOnceBase(name),
		_internal(std::forward<Ts>(args)...) {}

	/* Case 3: T constructible via init list; C++ painpoint. */
	template<typename U = T, typename std::enable_if<mnd::has_value_type<U>::value>::type* = nullptr>
		TOnce(const char* name, std::initializer_list<typename U::value_type> il) : TOnceBase(name) {
			if constexpr(std::is_constructible_v<U, std::initializer_list<typename U::value_type>>)
				_internal = U(il); // vector, list, etc
			else if constexpr(mnd::is_std_array<U>::value) {
				if(il.size() == 0)
					_internal.fill(typename U::value_type() );
				else if(il.size() == 1)
					_internal.fill(*il.begin());
				else if(il.size() == _internal.size())
					std::copy(il.begin(), il.end(), _internal.begin());
				else
					assert(il.size() == _internal.size() && "Initializer list for std::array<T,N> must have either 0, 1 or N members"); 
			}
			else static_assert(std::is_constructible_v<U, std::initializer_list<typename U::value_type>>,
				"Type doesn't correctly support initializer lists ctor.");
		}

	/* Case 4: default constructor. */
	template<typename std::is_default_constructible<T>::type* = nullptr>
		TOnce(const char* name = "") : TOnceBase(name), _internal() {}
	
	/* Case 5: Custom collector + name. Delegates to either [1] or [2] */
	template<typename... Ts>
		TOnce(const char* name, void (*fn)(T&, const T&), Ts&&... args) : TOnce(name, std::forward<Ts>(args)...) 
		{ _collector = fn; }
	
	/* Case 5.5: same as previous, but with init list. */
	template<typename U = T, typename std::enable_if<mnd::has_value_type<U>::value>::type* = nullptr>
		TOnce(const char* name, void (*fn)(T&, const T&), std::initializer_list<typename U::value_type> il) 
		: TOnce(name, il) { _collector = fn; }

	void SetName(std::string name, const char* title = "") override {
		if constexpr(std::is_base_of_v<TNamed, T> || mnd::has_setname<T>::value) {
			_internal.SetName(name.c_str());
			if constexpr(std::is_base_of_v<TNamed, T>)
				if(title && *title) _internal.SetTitle(title);
		}
		TOnceBase::SetName(name);
	}

	Int_t Write(TFile* f = nullptr, const char* name = "") override {
		if(!name) ERROR("%s [[ %s ]]. Don't pass nullptr for name arg in TOnce::Write.", _name.c_str(), _SELF_TYPE_CSTR);
		if(!f || f->IsZombie() || !f->IsOpen()) {
			f = gDirectory->GetFile();
			if(!f || f->IsZombie() || !f->IsOpen())
				ERROR("%s [[ %s ]] (label: \'%s\') : output ROOT file not supplied or invalid and gDirectory holds no open valid file. (f=0x%016lx)", 
					_name.c_str(), _SELF_TYPE_CSTR, name, (uintptr_t)f);
		}

		if constexpr(std::is_base_of_v<TObject, T>)
			return f->WriteTObject(&_internal, *name ? name : _name.c_str());
		else 
			return f->WriteObject (&_internal, *name ? name : _name.c_str());
	}
    
	void* Load(TFile* f, const char* target = "") override {
		const char* name = *target ? target : _name.c_str();
		if(!name || ! *name) ERROR("Unnamed TOnce<T> object while trying to load from a file.");
		
		if(!f || f->IsZombie() || !f->IsOpen())
			ERROR("(%s) (target: \'%s\') tried to open the file, but didn't receive a handle and also gDirectory holds no open valid file.", _name.c_str(), target);

		auto tmp = std::unique_ptr<T>( f->Get<T>(name) );
		if(!tmp) ERROR("(%s) - asked for " EMPH(%s) " name as key to a static object, got back nullptr.", _name.c_str(), name);
		
		if constexpr(std::is_base_of_v<TH1, T>) {
			tmp->SetDirectory(nullptr);
			_internal.SetDirectory(nullptr);
		}
		
		if constexpr(std::is_base_of_v<TObject, T>) {
			// Try calling `Copy` if it exists in the derived class.
			if constexpr(mnd::has_copy<T>::value)
				tmp->Copy(_internal);
			else
				_internal = *tmp; // deep-copy ; asserted via type traits on the top.
		}
		else { // Plain STL-types. std::vector, std::array, etc
			_internal = std::move(*tmp);
		}
		
		return (void*)&_internal;
	}
	
	std::unique_ptr<TOnceBase> Clone() const override {
		std::unique_ptr<TOnce<T>> copy;

		if constexpr(std::is_base_of_v<TH1, T>) {
			copy = std::make_unique<TOnce<T>>( this->GetName() );
			this->_internal.Copy( copy->_internal); /* T& -> TH1& -> TObject& */
			copy->_internal.SetDirectory(nullptr);
		}

		else if constexpr(std::is_base_of_v<TObject, T>) {
			copy = std::make_unique<TOnce<T>>( this->GetName() );

			if constexpr(mnd::has_copy<T>::value)
				this->_internal.Copy(copy->_internal);

			else { /* virtual TObject* Clone(const char *newname="") const */
				if constexpr(mnd::has_set_directory<T>::value) /* Reassigning the unique_ptr must not double-free. */
					copy->_internal.SetDirectory(nullptr);

				/* This calls case [5] ctor of TOnce<T> */
				copy = std::make_unique<TOnce<T>> (this->GetName(),
						/* T&& */ *static_cast<T*>( _internal.Clone(this->GetName()) ));

				if constexpr(mnd::has_set_directory<T>::value) /* Reassigning the unique_ptr must not double-free. */
					copy->_internal.SetDirectory(nullptr);
			}
		}

		else if constexpr(std::is_copy_constructible_v<T>) {
			copy = std::make_unique<TOnce<T>>(*this);
		}
		else if constexpr(std::is_copy_assignable_v<T>) {
			copy->_internal = _internal;
		}
		else {
			static_assert(! std::is_copy_assignable_v<T>,
				"Wrapped type <T> isn't a ROOT object (inherits from TObject), "
				"and doesn't have copy ctor or copy-assignment operator to clone from!");
		}

		copy->_collector = this->_collector;
		return copy;
	}
 
	void Collect(const TOnceBase& rhs) override {
		const TOnce<T>* cvt = dynamic_cast<const TOnce<T>*> (&rhs);

		if( !cvt) 
			ERROR("Type: %s, wrapped type: %s. TOnce object name \'%s\'. In Collect(..) - dynamic cast failed. "
				"RHS is named \'%s\'", _SELF_TYPE_CSTR, mnd::type_name<T>().c_str(),
				this->GetName(), rhs.GetName());

		if(strcmp(this->GetName(), rhs.GetName()) != 0)
			ERROR("Type: %s, wrapped type: %s. Trying to collect but objects are called differently. "
					"[1]: %s ; [2]: %s", _SELF_TYPE_CSTR, mnd::type_name<T>().c_str(),
					this->GetName(), rhs.GetName());

		if(_collector != nullptr)
			return _collector( this->_internal, cvt->_internal );

		/* Go over type traits, try to figure out which call to make, in priority. */
		if constexpr(mnd::has_dyadic_add_ref<T>::value) {
			return (void)this->_internal.Add( cvt->operator()() );
		}
		else if constexpr(mnd::has_dyadic_add_ptr<T>::value) {
			return (void)this->_internal.Add( cvt->operator->() );
		}
		else if constexpr(mnd::has_free_add_fn<T>::value) {
			return (void)Add( this->_internal, cvt->operator()() );
		}
		else if constexpr(mnd::has_free_mean_fn<T>::value) {
			return (void)Mean( this->_internal, cvt->operator()() );	
		}
		else if constexpr(mnd::has_add_div_unary_op<T>::value) {
			_internal += cvt->operator()();
			_internal /= 2;
		}
		/* This is tricky part. We should not issue a compile time error, since runtime collector can be provided.
		 * But compiler cannot know this ahead of time. Therefore, in this case issue a runtime error (an std::abort). 
		 * Now, for generic templates sometime an overload will match the template, but remain undefined, provocing 
		 * an lderror. This is a TODO. Temporary fix: user-defined strictest overload as a no-op to shut up the compiler. 
		 * IDK, maybe define and catch last-resort a generic symbol: 
		 * template<typename T> void AddG_(T&, const T&) {} ? Then warn users that the code flow bounces here I guess. */
		else { 
			ERROR("(%s) - Name: '\%s\' ; Underlying type \'%s\' doesn't define how to add or mean-up two instances, "
					"and also its been constructed without a runtime callback. "
					"Define a `void Add(T&, const T& )` function or pass a lambda as second argument "
					"of \'RegisterObject(..)\' (or other ctor).", 
					_SELF_TYPE_CSTR, this->GetName(), mnd::type_name<T>().c_str());
		}
	}

	T& operator()()             noexcept { return _internal; }
	const T& operator()() const noexcept { return _internal; }

	T* operator->()             noexcept { return &_internal; }
	const T* operator->() const noexcept { return &_internal; }

}; // struct TOnce

namespace mnd {
	template<typename T>
	constexpr auto noop_fn() -> void(*)(T&, const T&) {
		return +[](T&, const T&) {};
	}
}

#if !defined(MND_POLLUTE_G_NAMESPACE)
namespace mnd {
#endif

/* A free function, if type `T` is tucked away behind std::array|vector.
 * This also recurses nicely into std::array< std::array<... >>, as long as the 
 * underlying type at the base implements the Add function.
 * Further specializations will over-specialize and this generic overload won't get selected. */
template<typename T, std::size_t N,
	typename std::enable_if<mnd::has_dyadic_add_ref<T>::value>::type* = nullptr
> void Add(std::array<T, N>& lhs, const std::array<T, N>& rhs) {
	for(std::size_t i = 0; i < N; ++i) {
		Add(lhs[i], rhs[i]); 
	}
}
template<typename T,
	typename std::enable_if<mnd::has_dyadic_add_ref<T>::value>::type* = nullptr
> void Add(std::vector<T>& lhs, const std::vector<T>& rhs) {
	if(lhs.size() != rhs.size())
		ERROR("Vectors non-equal sized. Sizes: lhs=%zu, rhs=%zu. Cannot safely collect them together.",
			lhs.size(), rhs.size());
	for(std::size_t i = 0; i < lhs.size(); ++i) { Add(lhs[i], rhs[i]); }
}

#if !defined(MND_POLLUTE_G_NAMESPACE)
}
#endif

using TDictInfo = std::unordered_map<std::string, std::string>;

/* This base type isn't just used for polymorphism - note non-virtual dtor.
 * It's just to indicate that Setup should- and `Init` could- be overridden. */
struct TContainerBase {
	std::string _name;
	
	inline const char* GetName() const noexcept { return _name.c_str(); }
	inline void SetName(std::string name) noexcept { _name = std::move(name); }

	TContainerBase() = default;
	TContainerBase(std::string name) : _name(std::move(name)) {}

	/** 
	 * Should be initial (optional) call. Setting up persistent metadata, and names.
	 */
	virtual void Init(TDictInfo info) { (void)info; }

	/**
	 * Delayed construction of TOnce objects. Possibly override it in the derived classes.
	 */
	virtual void Setup() = 0;
};

/**
 * Encapsulates a type needed to be used as a (de)serialization target
 * from/to RNTuple column. It still is an abstract type, since the `Setup()` method remains pure virtual.
 * Users are at most expected to extend it and define the Setup() method, where the `RegisterObject` methods 
 * will be chained, to give back the raw resource handles to the users' derived container class.
 */
template<typename T>
struct TContainer : TContainerBase {
	static_assert(!std::is_void_v<T>, "Hello?");
	static_assert(!std::is_array_v<T>, "Must not pass raw C-style arrays. Pass an `std::array<T,N>` instead.");
	static_assert(!std::is_pointer_v<T>, "Must not pass pointer type (T*).");
	static_assert(!std::is_reference_v<T>, "Must not pass ref type (T&).");

	template<typename>    friend struct TProcessor;
	template<typename...> friend struct TAnalysisProcess;

	using inner_type = T;
	using TOnceBaseVec = std::vector <
		std::shared_ptr<TOnceBase>
	>;

protected:
	TOnceBaseVec _vc;

private:
	std::shared_ptr<T> _inner;

public:
	TContainer() {};
	TContainer(std::string name) : TContainerBase(name) {}

	/* Ok, some clarification. Lets say `struct Derived : TContainer<T>` is the derived class.
	 * Then the next two methods below must get called in `Derived::Setup()`
	 * We create the objects, pulling the read objects from disk, and receiving back a shared handle of the resource. 
	 * This works fine for the original TContainer<T> instance.
	 * But when cloning a TAnalysisProcess, it clones the underlying 'TProcessor<Out(Ins...)>' types. 
	 * Instance behind *this* pointer, for the reader, is created via the copy ctor of the TContainer<T>,
	 * while the write instances are default constructed. The clone's ctor will also call this sequence initially, each pulling its own
	 * copy of the read-objects from disk into RAM. Using unnecessary disk space, if simply all the clones own a 
	 * read-only shared pointer to a single instance of such objects.
	 * - key difference is that then the Original singleton will simply switch the clones' variant to non-owning
	 * type, and the temporarily created (unique) objects of the clone will get deleted. 
	 * This is important for read containers - as only one set of these `TOnce<T>` objects will get (de)serialized.
	 * Clones and the original holds the shared pointers. 
	 * For write containers, each clone has its own unique copy. */

	/**
	 * Create an object to be serialized to/from a ROOT file. Two overloads exist for passing an
	 * initializer list ctor (like for `std::array<T>`) or a basic forwarding ctor (emplace-style). 
	 */
	template<typename U>
	[[nodiscard]] U* RegisterObject(const char* name, std::initializer_list<typename U::value_type> il) {
		/* Flag the owned object with `CONTAINERNAME_` prefix. */
		std::string obj_name = mnd::sstrcat(this->GetName(), "_", name); 
		for(auto& o : _vc) {
			if(! strcmp(o->GetName(), obj_name.c_str())) {
				TOnce<U> *dcast = dynamic_cast<TOnce<U>*>( o.get() );
				if(!dcast)
					ERROR("Attempted to retrieve object named \'%s\' from %zu-sized list of owned TOnce<..> objects."
						"Found at address 0x%lx a 'TOnceBase' but dynamic_cast failed? Same object registered multiple times? (%s)",
						name, _vc.size(), (uintptr_t)o.get(), _SELF_TYPE_CSTR);
				return dcast->operator->();
			};
		}
		
		std::shared_ptr<TOnce<U>> obj = std::make_shared<TOnce<U>>(obj_name.c_str(), il);
		TOnce<U>* p = obj.get();

		_vc.push_back( std::move(obj) );
		return p->operator->();
	}

	template<typename U>
	[[nodiscard]] U* RegisterObject(const char* name, void (*fn)(U&, const U&), std::initializer_list<typename U::value_type> il) {
		U* obj = this->template RegisterObject<U>(name, il);
		TOnce<U>* wrapped_obj = static_cast<TOnce<U>*>( _vc.back().get() );	
		wrapped_obj->_collector = fn;

		return obj;
	}

	template<typename U, typename... Ts>
	[[nodiscard]] U* RegisterObject(const char* name, Ts&&... args) {
		/* Flag the owned object with `CONTAINERNAME_` prefix. */
		std::string obj_name = mnd::sstrcat(this->GetName(), "_", name); 
		for(auto& o : _vc) {
			if(! strcmp(o->GetName(), obj_name.c_str())) {
				TOnce<U> *dcast = dynamic_cast<TOnce<U>*>( o.get() );
				if(!dcast)
					ERROR("Attempted to retrieve object named \'%s\' from %zu-sized list of owned TOnce<..> objects."
						"Found at address 0x%lx a 'TOnceBase' but dynamic_cast failed? Same object registered multiple times? (%s)",
						name, _vc.size(), (uintptr_t)o.get(), _SELF_TYPE_CSTR);
				return dcast->operator->();
			};
		}
		
		std::shared_ptr<TOnce<U>> obj = std::make_shared<TOnce<U>>(obj_name.c_str(), std::forward<Ts>(args)...);
		TOnce<U>* p = obj.get();

		_vc.push_back( std::move(obj) );
		return p->operator->();
	}

	void Clean() noexcept {
		if constexpr(mnd::has_clean_noexcept<T>::value)
			_inner->Clean();
		else
			static_assert(mnd::has_clean_noexcept<T>::value, "Type <T> has no `void Clean() noexcept` method defined.\n");
	}

	/* Rule of five. We keep it here to be explicit about copy-ctor (not really necessary...) */
	TContainer(const TContainer& )                = default;
	TContainer& operator=(const TContainer& )     = default;
	TContainer(TContainer&& )            noexcept = default;
	TContainer& operator=(TContainer&& ) noexcept = default;
	~TContainer()                                 = default;
	
	const TOnceBaseVec& GetTOnceVec() const noexcept { return this->_vc; }

	T& operator*()              noexcept { return *_inner; }
	const T& operator*()  const noexcept { return *_inner; }
	T& inner()                  noexcept { return this->operator*(); }
	const T& inner()      const noexcept { return this->operator*(); }
	T* operator->()             noexcept { return _inner.get(); }
	const T* operator->() const noexcept { return _inner.get(); }
};

/* Type `T` is either something like TXXXYYYEvent (Go4) or a custom structure
 * for UCESB. 
 * Namely, UCESB splits all the trees directly into leaves, so address has to be mapped
 * sequentally.
 */
template<typename T>
struct TRawContainer : TContainerBase {
	static_assert(!std::is_void_v<T>, "Hello?");
	static_assert(!std::is_array_v<T>, "Must not pass raw C-style array. Prefer `std::array<T,N>` instead.");
	static_assert(!std::is_pointer_v<T>, "Must not pass pointer type (T*).");
	static_assert(!std::is_reference_v<T>, "Must not pass ref type (T&).");
	
	template<typename>    friend struct TProcessor;
	template<typename...> friend struct TAnalysisProcess;
	
	using inner_type = T;

private:
	T* _inner;

public:
	TRawContainer() = default;
	TRawContainer(std::string name) : TContainerBase(name) {}

	void Setup() override {}

	T& operator*()              noexcept { return *_inner; }
	const T& operator*()  const noexcept { return *_inner; }
	T& inner()                  noexcept { return this->operator*(); }
	const T& inner()      const noexcept { return this->operator*(); }
	T* operator->()             noexcept { return _inner; }
	const T* operator->() const noexcept { return _inner; }
	T* raw()                    noexcept { return this->operator->(); }
	const T* raw()        const noexcept { return this->operator->(); }
};

struct TProcessorBase {
	TProcessorBase() = default;
	TProcessorBase(const TProcessorBase& ) = default;
	TProcessorBase& operator=(const TProcessorBase& other) = default;
	TProcessorBase(TProcessorBase&& other) = default;
	TProcessorBase& operator=(TProcessorBase&& other) = default;

	virtual ~TProcessorBase() = default;
};

template<typename T> struct TRawContainer;

template<typename>
struct TProcessor; /* Undefined. */

/**
 * Base type of a single analysis subprocess.
 * User-derived types will implement `void ProcessEntry() noexcept` method to map the data
 * from combination of input structures (types: `Ins...`) to the one unique output (type `Out`) structure.
 */
template<typename Out, typename... Ins>
struct TProcessor<Out(Ins...)> : TProcessorBase {
	static_assert((std::disjunction_v<
		mnd::is_base_of_template<TContainer, Ins>,
		mnd::is_base_of_template<TRawContainer, Ins>> && ...), 
		"Input type(s) must inherit from (or be) TContainer<T> / TRawContainer<T>.");
	static_assert(mnd::is_base_of_template<TContainer, Out>::value, "Output type must inherit from (or be) TContainer<T>.");

	Out out;
	std::tuple<Ins...> in;
	
	TProcessor() = default;

	/** 
	 * Only other acceptable param ctor is to take ownership from an existing output object.
	 * The output object doesn't need to be specifically `std::move`'ed to be less verbose in user code.
	 * But the Processor does take full ownership of the resource behind this reference.
	 * Input objects, might be shared between different subprocesses - here we simply make our own copy.
	 */
	explicit TProcessor(Out& _out, const Ins&... ins) : 
		out(std::move(_out)), in(std::make_tuple(ins...)) {}

	TProcessor(const TProcessor& rhs) : TProcessorBase(rhs), 
		out(rhs.out), 
		in(rhs.in) /* _vc from each input will just get copied - sharing the *same* pointers. */ 
		{
			out._vc.clear();

			/* https://en.cppreference.com/w/cpp/memory/shared_ptr/shared_ptr.html -- Case [13]
			 * template< class Y, class Deleter > shared_ptr( std::unique_ptr<Y, Deleter>&& r );*/
			for(const std::shared_ptr<TOnceBase>& v  : rhs.out._vc)
				out._vc.emplace_back( v->Clone() );

			/* Previous for-loop constructs the objects, the next dynamic dispatch
			 * will just give the raw pointer handles back to the user. */ 
			out.Setup();
		}
	/*  ^^^^^ Now, each `_vc` is completely unique in the output container. */
	
	/* Identical logic for copy-assignment op */
	TProcessor& operator=(const TProcessor& rhs) {
		this->in = rhs.in;
		this->out = rhs.out;
		this->out._vc.clear();
		
		for(const std::shared_ptr<TOnceBase>& v  : rhs.out._vc)
			out._vc.emplace_back( v->Clone() );
		
		out.Setup();
		return *this;
	}

	TProcessor(TProcessor&& )            noexcept = default;	
	TProcessor& operator=(TProcessor&& ) noexcept = default;	
	
	~TProcessor() = default;

	/**
	 * Returns a reference to the input container.
	 */
	template<u32 N = 0> 
	decltype(auto) GetInput() const& {
		static_assert(N < std::tuple_size_v<decltype(in)>, "Accessing N >= tuple size.");
		return ( std::get<N>(in).inner() );
	}

	template<u32 N = 0> 
	decltype(auto) GetInput() & {
		static_assert(N < std::tuple_size_v<decltype(in)>, "Accessing N >= tuple size.");
		return ( std::get<N>(in).inner() );
	}
	
	/* These calls are sent during the final collection. Strict type checks
	 * are kept, as runtime isn't sacrificed too much. */
	void Collect(const TProcessor& rhs) {
		std::vector<std::shared_ptr<TOnceBase>>       & lvc =     out._vc;
		const std::vector<std::shared_ptr<TOnceBase>> & rvc = rhs.out.GetTOnceVec();
		if( lvc.size() != rvc.size() )
			ERROR("(%s) trying to collect but output object named \'%s\' has unmatching sizes. %zu != %zu",
				_SELF_TYPE_CSTR, this->out.GetName(), lvc.size(), rvc.size());
		
		for(int i=0; i<(int)lvc.size(); ++i)
			lvc[i]->Collect( *(rvc[i]) );
	}

}; // TProcessor 

template<u32, typename...> struct TAnalysisPool;

namespace mnd {

struct Job { u64 first, last; };
using JobQueue = boost::lockfree::spsc_queue <
	Job, boost::lockfree::capacity<8>
>;

struct IOInfo {
	struct {
		std::string fname;
	} in;
	struct {
		std::string fname;
		std::string out_rnname;
	} out;
};

struct RNPerThreadReader {
	std::unique_ptr<RExp::RNTupleModel>  _model;
	std::unique_ptr<RExp::RNTupleReader> _reader;
};

struct TTreePerThreadReader {
	std::unique_ptr<TFile> _file;
	TTree* _tree;
};

using PerThreadReader = Variant <
	RNPerThreadReader,
	TTreePerThreadReader
>;

/** 
 * Return codes:
 * 0  => properly set-up.
 * 1  => variant in empty state
 * 2  => switched to TTree variant but file handle is null. 
 * 4  => switched to TTree variant but ttree handle is null. 
 * 8  => switched to RNTuple variant but model isn't null.
 * 16 => switched to RNTuple variant but reader is null.
 */
inline int GetValidity(const PerThreadReader& reader) {
	int r = 0;
	if(IsEmpty(reader))   r |= 0x1;
	else if(auto* p = std::get_if<TTreePerThreadReader>( &reader);
		p != nullptr) {
		if(! p->_file )   r |= 0x2;
		if(! p->_tree )   r |= 0x4;
	}
	else if(auto* p = std::get_if<RNPerThreadReader>( &reader);
		p != nullptr) {
		if( p->_model )   r |= 0x8;
		if(! p->_reader ) r |= 0x10;
	}
	return r;
}

struct PerThreadWriter {
	/* Thread [0]'s instance owns the parallelwriter, other threads
	 * will default to only holding the raw pointer handle to it. */
	Variant <
		std::unique_ptr<RExp2::RNTupleParallelWriter>,
		RExp2::RNTupleParallelWriter*
	> pwriter;
	std::shared_ptr<RExp2::RNTupleFillContext> ctx;
	std::unique_ptr<RExp::REntry> entry;

	void Reset() {
		entry.reset();
		ctx.reset();
		auto* p = std::get_if<std::unique_ptr<RExp2::RNTupleParallelWriter>>(&pwriter);
		if(p != nullptr)
			p->reset();
	}

	/** 
	 * Return codes:
	 * 0 => properly set-up.
	 * 1 => pwriter variant in empty state
	 * 2 => pwriter non-empty state, but is null.
	 * 4 => context in null state.
	 * 8 => entry in null state.
	 */
	int GetValidity() const {
		int r = 0;
		if(IsEmpty(pwriter)) r |= 0x1;
		else if ( /* Try to cast to 1st type instance. */
			auto* p = std::get_if<std::unique_ptr<RExp2::RNTupleParallelWriter>>(&pwriter);
			p != nullptr && *p == nullptr 
		)                          r |= 0x2;
		else if ( /* Try to cast to 2nd type instance. */
			auto* p = std::get_if<RExp2::RNTupleParallelWriter*>(&pwriter);
			p != nullptr && *p == nullptr 
		)                          r |= 0x2;
		if(! ctx)                  r |= 0x4;
		if(! entry)                r |= 0x8;
		return r;
	}
};

} // namespace mnd

inline std::unordered_set<std::string> g_loaded_containers {};

/**
 * Represents the full analysis process, where an input entry 
 * will go through all of the `T::ProcessEntry()` for each of the 
 * sequential processes to create a complete output event.
 *
 * It holds a queue of entries that the main thread distributes, and encapsulates
 * all the resources a worker thread will need.
 */
template<typename... Ts>
struct alignas(mnd::CL) TAnalysisProcess final {
	static_assert((mnd::is_base_of_template<TProcessor, Ts>::value && ...),
		"All the inderlying subprocess types <Ts> must inherit from TProcessor<...>.");
	static_assert((mnd::has_process_entry<Ts>::value && ...),
		"All the inderlying subprocess types <Ts> need a public `void ProcessEntry() noexcept` method implemented.");
	static_assert((std::is_copy_constructible_v<Ts> && ...),
		"All the inderlying subprocess types <Ts> need a copy ctor.");
	static_assert((std::is_copy_assignable_v<Ts> && ...),
		"All the inderlying subprocess types <Ts> need copy assignment op.");
	static_assert((std::is_move_constructible_v<Ts> && ...),
		"All the inderlying subprocess types <Ts> need a move ctor.");
	static_assert((std::is_move_assignable_v<Ts> && ...),
		"All the inderlying subprocess types <Ts> need move assignment op.");

	template<u32, typename...> friend struct TAnalysisPool;
	static constexpr size_t Size() { return sizeof...(Ts); }

private:
	mnd::JobQueue q;
	std::atomic<bool> _running {false}; /* Flagging this will join the thread back to the main. */
	bool _do_write {true};
	std::thread _thread;

	std::tuple<Ts...> _proc;

	mnd::IOInfo info;
	mnd::PerThreadReader reader;
	mnd::PerThreadWriter writer;
	
private:
	template<size_t I>
	using base_t = std::tuple_element_t<I, decltype(_proc)>;

	template<size_t I>
	static TProcessorBase* get_at(TAnalysisProcess* self) noexcept {
		return static_cast<TProcessorBase*>(&std::get<I>(self->_proc));
	}
	
	using Getter = TProcessorBase* (*)(TAnalysisProcess*);

	template<std::size_t... Is>
	static constexpr std::array<Getter, sizeof...(Is)>
	make_getter_table(std::index_sequence<Is...>) noexcept { return { &get_at<Is>... }; }

public:
	TAnalysisProcess() = default;
	TAnalysisProcess(std::string file_in, std::string file_out, std::string rn_out) :
		TAnalysisProcess() {
			info.in.fname  = std::move(file_in);
			info.out.fname = std::move(file_out);
			info.out.out_rnname = std::move(rn_out);
		}
	
	explicit TAnalysisProcess(
		std::tuple<Ts...>&& w,
		mnd::IOInfo&& _info,
		mnd::PerThreadReader&& _reader,
		mnd::PerThreadWriter&& _writer
	) : _proc(std::move(w)), info(std::move(_info)), 
		reader(std::move(_reader)), writer(std::move(_writer)) {}

	TAnalysisProcess(const TAnalysisProcess& rhs) : TAnalysisProcess{} {
		rhs.Clone(*this);
	}

	TAnalysisProcess& operator=(const TAnalysisProcess& rhs) {
		rhs.Clone(*this);
		return *this;
	}

	/* Move ops cannot be defaulted due to std::atomic<T> */
	TAnalysisProcess(TAnalysisProcess&& rhs) :
		q        {},
		_running ( false                  ), 
		_do_write( rhs._do_write          ),
		_thread  ( std::move(rhs._thread) ),
		_proc    ( std::move(rhs._proc)   ),
		info     ( std::move(rhs.info)    ),
		reader   ( std::move(rhs.reader)  ),
		writer   ( std::move(rhs.writer)  ) {}
	
	TAnalysisProcess& operator=(TAnalysisProcess&& rhs) noexcept {
		if(this == &rhs) return *this;
		_running  = false                 ;  
		_do_write = rhs._do_write         ; 
		_thread   = std::move(rhs._thread); 
		_proc     = std::move(rhs._proc)  ; 
		info      = std::move(rhs.info)   ; 
		reader    = std::move(rhs.reader) ; 
		writer    = std::move(rhs.writer) ; 
		return *this;
	}

	~TAnalysisProcess() = default;

	/* Adding a new proc mutates the type of the object. */

	/** 
	 * Reference-qualified: take an l/rvalue ref of an existing process object and
	 * eat up the object. Now we own it. Moves *this* object into a new one. 
	 */
	template <typename W,
		typename U = std::decay_t<W>
	> auto push_process(W&& w) && -> TAnalysisProcess<Ts..., U> {
		auto new_proc = std::tuple_cat(
			std::move(_proc),
			std::tuple<U>(std::move(w))
		);
		return TAnalysisProcess<Ts..., U> ( 
			std::move(new_proc), std::move(info), 
			std::move(reader), std::move(writer)
		);
	}
	
	/** 
	 * Emplace-style construction, again move *this* object into a new one. 
	 */
	template <
		typename U,      /* U = child of TProcessor<Out(Ins...)> */
		typename Out,    /* Output container type. Deduced */
		typename... Args 
	> auto emplace_process(Out&& out, Args&&... args) && -> TAnalysisProcess<Ts..., U> {
		/* In the varargs following, load the input containers, *before* constructing the processor. */
		(..., LoadTOnceInputs(args));

		auto new_proc = std::tuple_cat (
			std::move(_proc),
			std::make_tuple( U(std::forward<Out>(out), std::forward<Args>(args)... ) )
		);
		return TAnalysisProcess<Ts..., U> ( 
			std::move(new_proc), std::move(info), 
			std::move(reader), std::move(writer)
		);
	}

	/**
	 * A true independent copy of the object, meant to populate the pool singleton
	 * with clones of the initial processor object. 
	 */
	void Clone(TAnalysisProcess& dest) const { /* Only clone from the original object. */
		auto* p = std::get_if<std::unique_ptr<RExp2::RNTupleParallelWriter>>(&writer.pwriter);
		/*^^^ type: std::unique_ptr<..> *  */
		if(!p) ERROR("Calling clone but original processor object is either unitialized or set to wrong state. "
				"State = %zu, 0 = Empty; 1 = Owning pointer; 2 = Raw pointer. Should be: " EMPH(1\n), writer.pwriter.index());
		//if(this->reader.index() == 2)
		//	ERROR("Cannot use multithreaded op with TTree readers. Not implemented yet! Compile singlethreaded please!");

		dest._proc = this->_proc;
		dest.info  = this->info;
		dest.writer.pwriter = p->get();
		
		dest.Setup();
	}

	bool IsStopped() const noexcept { return ! _running && ! _thread.joinable(); }

	/**
	 * Returns the pointer to type 'TProcessor<Out(Ins..)>',
	 * at the tuple index 'I'
	 */
	template<std::size_t I>
	auto* GetProcess() noexcept {
		static_assert(I < Size(), "Request for process index outside of the tuple size.");
		return static_cast<base_t<I>*>(&std::get<I>(_proc));
	}
	template<std::size_t I>
	auto* GetProcess() const noexcept {
		static_assert(I < Size(), "Request for process index outside of the tuple size.");
		return static_cast<const base_t<I>*>(&std::get<I>(_proc));
	}
	// Runtime version, can throw.
	TProcessorBase* GetProcess(size_t i) {
		if(i >= Size()) ERROR("Request for worker index outside of the tuple size.");
		static constexpr auto table = make_getter_table(std::make_index_sequence<Size()>{});

		return table[i](this);
	}

	/**
	 * Return a table of processes, upcasted to the common parent.
	 */
	std::array<TProcessorBase*, Size()> GetProcesses() {
		std::array<TProcessorBase*, Size()> rv{};
		for(size_t i=0; i<Size(); ++i)
			rv[i] = this->GetProcess(i);
		return rv;
	}

	Int_t GetEntry(Long64_t entry) const { 
		if(mnd::IsEmpty(reader))
			ERROR("Empty input TTree/RNTuple. Invalid");
		else if(std::holds_alternative<mnd::RNPerThreadReader>(reader)) {
			std::get<mnd::RNPerThreadReader>(reader)
				._reader->LoadEntry(entry);
			return 0;
		}
		else {
			return std::get<mnd::TTreePerThreadReader>(reader)
			._tree->GetEntry(entry);
		}
	}

	u64 GetEntries() const { 
		if(mnd::IsEmpty(reader)) 
			ERROR("Empty input TTree/RNTuple in GetEntries call. Invalid");
		else if(std::holds_alternative<mnd::RNPerThreadReader>(reader)) {
			return static_cast<u64>( std::get <
				mnd::RNPerThreadReader
			> (reader)._reader->GetNEntries());
		}
		else {
			return static_cast<u64> (
				std::get<mnd::TTreePerThreadReader>(reader)._tree->GetEntries()
			);
		}
	}
	
	/* Run the setup. */
	void Setup() { SetupReader(); SetupWriter(); }

	void Start() noexcept { /* ERROR(...) will call std::abort, so it's sane to mark it noexcept. */
		if(_running) 
			ERROR("Start called but worker thread is still marked as running? (%s)", _SELF_TYPE_CSTR);
		if(int v = writer.GetValidity(); v != 0) 
			ERROR("Calling start but writer handle isn't valid (v != 0). v = 0x%02x. Check API for GetValidity().", v); 
		if(int r = mnd::GetValidity(reader); r != 0)
			ERROR("Calling start but reader handle isn't valid (r != 0). r = 0x%02x. Check API for mnd::GetValidity().", r); 
		
		_running = true;
		
		_thread = std::thread (
			[this] {
				mnd::Job j;
				while(_running.load(std::memory_order_relaxed)) {
					if( q.pop(j) ) {
						for(u64 evId = j.first; evId < j.last; ++evId) {
							this->GetEntry( static_cast<Long64_t>(evId) );
							
							std::apply([](auto&... ps) {
								(..., ps.ProcessEntry()); 
							}, this->_proc);
					
							if(this->_do_write)
								this->writer.ctx->Fill(*this->writer.entry);
						}
					} else {
#ifdef __HAS_SMALL_INTEL_SPIN
						_mm_pause(); /* Short pause, 100-150 clock cycles. */
#else
						{}           /* Do nothing; don't yield or reschedule. */
#endif
					}
				}

				/* On shutdown, drain the remainder of the queue. */
				while( q.pop(j) ) {
					for(u64 evId = j.first; evId < j.last; ++evId) {
						this->GetEntry( static_cast<Long64_t>(evId) );
						
						std::apply([](auto&... ps) {
							(..., ps.ProcessEntry()); 
						}, this->_proc);

						if(this->_do_write)
							this->writer.ctx->Fill(*this->writer.entry);	
					}
				}
			}
		);
	}

	void Stop() {
		_running.store(false, std::memory_order_relaxed);
		if(_thread.joinable()) _thread.join();
	}

	template<u32 N>
	auto MakePool(u32 NSlice) && -> TAnalysisPool<N, Ts...> {
		return TAnalysisPool<N, Ts...>( *this, NSlice );	
	}

	void Collect(const TAnalysisProcess& rhs) {
		std::tuple <
			std::pair<Ts&, const Ts&>...
		> pairs = mnd::zip_refs(this->_proc, rhs._proc);
		
		std::apply([](auto&... pr) {
				(..., pr.first.Collect( pr.second ));
			}, pairs
		);
	}
	
private:
	
	/* Each of the instances' writer is a slave to the initial one, who
	 * holds the true unique pointer handle. */
	void SetupWriter() {
		if(info.out.fname.empty() || info.out.out_rnname.empty())
			ERROR("Cannot proceed with setting up the writer if output (file,rnname) string information isn't given. (%s)", _SELF_TYPE_CSTR);
		if(writer.ctx || writer.entry) 
			ERROR("RNTupleWriter, context or entry already given (non-null)? (%s)", _SELF_TYPE_CSTR);

		RExp2::RNTupleParallelWriter* pwriter_raw;
		
		/* Writer can be set up either from the original, which means we set up the bare model. */
		if(mnd::IsEmpty(writer.pwriter)) {
			auto model = RExp::RNTupleModel::CreateBare();
			
			mnd::for_each_in_tuple(this->_proc, [this, &model](auto& p /* TProcessor<Out(In...)>) */ ) 
				{
					const char* name = p.out.GetName();
					if(strlen(name) == 0) ERROR("Setting up the writer but an output container is unnamed. (%s)", _SELF_TYPE_CSTR);

					model->MakeField<typename decltype(p.out)::inner_type>( name );
				}
			);
			writer.pwriter = RExp2::RNTupleParallelWriter::Recreate(std::move(model), info.out.out_rnname, info.out.fname);
			pwriter_raw = std::get<std::unique_ptr<RExp2::RNTupleParallelWriter>> (writer.pwriter).get();
			if(!pwriter_raw)
				ERROR("RNTupleParallelWriter switched to original state, correct. But pointer is null after creation?");
		}
		else { /* ... Or it can be called from the clone. Initial `Clone()` call will set the raw pointer upfront. Here just check if valid. */
			if( ! std::holds_alternative<RExp2::RNTupleParallelWriter*>(writer.pwriter) )
				ERROR("RNTupleParallelWriter isn't the original, but clone not switched to raw pointer handle. (%s)", _SELF_TYPE_CSTR);
			
			pwriter_raw = std::get<RExp2::RNTupleParallelWriter*>(writer.pwriter);
			if(!pwriter_raw)
				ERROR("RNTupleParallelWriter switched to clone state, correct. But pointer is null?");
		}

		writer.ctx = pwriter_raw->CreateFillContext();
		writer.entry = writer.ctx->CreateEntry();

		mnd::for_each_in_tuple(this->_proc, [this](auto& p /* TProcessor<Out(In...)>) */ ) 
			{
				p.out._inner = this->writer.entry
					->template GetPtr< typename decltype(p.out)::inner_type >( p.out.GetName() );
			}
		);
	}

	void SetupReader() {
		if(info.in.fname.empty())
			ERROR("Info given, but input file name empty. (%s)", _SELF_TYPE_CSTR);

		/* Reader object must be in empty state, */
		if(! mnd::IsEmpty(reader)) 
			ERROR("Trying to setup a fresh reader, but the object is already created with index: %zu. "
				"1 => RNReader; 2 => TTreeReader. (%s)", reader.index(), _SELF_TYPE_CSTR);
		
		std::string obj_name; bool is_ttree{};
		auto _file = std::make_unique<TFile>( info.in.fname.c_str() , "READ");
		if(!_file || _file->IsZombie() || !_file->IsOpen())
			ERROR("Setting up the reader but unable to make a TFile hook for \'%s\'. (%s)", info.in.fname.c_str(), _SELF_TYPE_CSTR);

		/* Try finding the read container. Can be either TTree or RNTuple. */
		for(TObject* _k : *_file->GetListOfKeys()) {
			TKey* k = dynamic_cast<TKey*>(_k);
			if(!k) continue;
			TClass* cl = TClass::GetClass(k->GetClassName());
			if(cl && cl->InheritsFrom(ROOT::RNTuple::Class()) ) {
				obj_name = std::string( k->GetName() ); is_ttree = false;
				break;
			} else if(cl &&  cl->InheritsFrom(TTree::Class()) ) {
				obj_name = std::string( k->GetName() ); is_ttree = true;
				break;
			}
		}

		if(obj_name.empty())
			ERROR("File %s opened fine, but cannot find a 'ROOT::RNTuple' or 'TTree' object inside? (%s)", 
				info.in.fname.c_str(), _SELF_TYPE_CSTR);
		
		_file.reset(nullptr);

		if(! is_ttree) { /* RNTuple version. */
			reader = mnd::RNPerThreadReader();
			auto& r = std::get<mnd::RNPerThreadReader>(reader);
			r._model = RExp::RNTupleModel::Create();
			
			/* For RNTupleModel' API, we use version 6.24.
			 * Newer ROOT 6.26 API could be different? */
			mnd::for_each_in_tuple(this->_proc, [this, &r](auto& p /* TProcessor<Out(In...)>) */ )
				{
					mnd::for_each_in_tuple(p.in, [this, &r](auto& cont /* In : TContainer / TRawContainer */ )
						{
							/* Compile out the block if it's not a TContainer. */
							using ContType = typename std::remove_reference_t<decltype(cont)>;
							
							if constexpr( mnd::is_base_of_template<TContainer, ContType>::value ) {
								if(! strlen(cont.GetName()) )
									ERROR("Input container (RN-meant) unnamed? (%s)", mnd::type_name<ContType>().c_str());
								/* If the column has already been mapped, MakeField throws a 'RExp::RException'.
								 * In this case, just retrieve it and map it to the inner. */
								try {
									cont._inner = r._model->MakeField <
										typename ContType::inner_type
									> ( cont.GetName() );
								} catch(std::exception const& e) {
									cont._inner = r._model->GetDefaultEntry().GetPtr <
										typename ContType::inner_type
									> ( cont.GetName() );	
								} catch(...) {
									ERROR("Unknown exception caught? When trying to accessing RNTuple column. (%s)",
										mnd::type_name<ContType>().c_str());
								}
							}
						}
					);
				}
			);

			r._reader = RExp::RNTupleReader::Open(std::move(r._model), obj_name, info.in.fname);
		}

		else { /* TTree version. */
			(void)TClass::GetClass("TTree");
			(void)TClass::GetClass("TBranch");
			(void)TClass::GetClass("TBasket");
			
			this->reader = mnd::TTreePerThreadReader();
			auto& r = std::get<mnd::TTreePerThreadReader>(reader);

			r._file = std::make_unique<TFile>( info.in.fname.c_str() , "READ");
			if(!r._file || r._file->IsZombie() || !r._file->IsOpen())
				ERROR("Unable to make a TFile hook for \'%s\'. (%s)", info.in.fname.c_str(), _SELF_TYPE_CSTR);

			r._tree = dynamic_cast<TTree*>(r._file->Get(obj_name.c_str()));
			if(!r._tree || r._tree->IsZombie())
				ERROR("Unable to make a TTree hook for \'%s\'. "
					"Even though TTree verified with name \'%s\'. (%s)", info.in.fname.c_str(), obj_name.c_str(), _SELF_TYPE_CSTR);
			
			r._tree->SetCacheSize(64*1024*1024);

			
			 /* TTree API: once we map a branch via `tree->SetBranchAddress(name, &ptr)`, then we can retrieve the pointer's
			 * address via: tree->GetBranch(name)->GetAddress() . The return type is `char**`. 
			 * Type safety isn't checked at runtime. Basically the other argument is `void**`somewhere down the line. 
			 * Checking the type does indeed work, if the underlying type isn't templated.
			 * It *should* demangle correctly. C++ ABI can change, but it must reflect the same in Cling! */

			mnd::for_each_in_tuple(this->_proc, [this, &r](auto& p /* TProcessor<Out(In...)>) */ )
				{
					mnd::for_each_in_tuple(p.in, [this, &r](auto& cont /* In : TRawContainer */ )
						{
							/* Compile out the block if it's not a TRawContainer. */
							using ContType = typename std::remove_reference_t<decltype(cont)>;

							if constexpr( mnd::is_base_of_template<TRawContainer, ContType>::value ) {
								if(! strlen(cont.GetName()) )
									ERROR("Input container (TTree-meant) unnamed? (%s)", _SELF_TYPE_CSTR);
								
								TBranch* b = r._tree->GetBranch( cont.GetName() );
								if(!b) ERROR("File: \'%s\', TTree: \'%s\', branch \'%s\' not found. (%s)",
									this->info.in.fname.c_str(), r._tree->GetName(), cont.GetName(), _SELF_TYPE_CSTR);
								
								using T = typename ContType::inner_type;
								std::string c_type = mnd::type_name<T>();

								if(b->GetAddress()) { /* Means the branch is mapped already. */
									/* Try to check if types match. */
									const char* b_type = b->GetClassName();

									if(strcmp(b_type, c_type.c_str()) != 0)
										WARN("Setting up the container \'%s\'. "
											"File: \'%s\', TTree: \'%s\', branch \'%s\'. Types mismatch: "
											"TTree inspection gives branch type \'%s\', but we're requesting type \'%s\'. "
											"Is this correct? (%s)\n",
											cont.GetName(), this->info.in.fname.c_str(), r._tree->GetName(),
											b->GetName(), b_type, c_type.c_str(), _SELF_TYPE_CSTR);
									
									cont._inner = reinterpret_cast<T*>( *(void**)b->GetAddress() );
								} 
								else { // Branch isn't mapped yet. Map it now.
									/* Important to set to null. Else ROOT will not do anything, as it already
									 * dereferences to a valid T. 🤷 */
									cont._inner = nullptr; 
									
									(void)TClass::GetClass(c_type.c_str());
									
									r._tree->SetBranchAddress( cont.GetName(), &cont._inner);
									b->SetAutoDelete(kFALSE);
								}
							} // if constexpr
						}
					); // fold over input containers, per subprocess
				}
			); // fold over subprocesses
		
			/* Warm-up. */
			Long64_t _n_entries = std::min(10LL, r._tree->GetEntries());
			for(Long64_t i=0; i<_n_entries; ++i) r._tree->GetEntry(i);

		} // TTree version end
	} // void SetupReader()

	template<typename T,
		typename std::enable_if<mnd::is_base_of_template<TContainer, T>::value>::type* = nullptr
	> void LoadTOnceInputs(const T& cont) {
		const char* fname = info.in.fname.c_str();
		auto f = std::make_unique<TFile>(fname, "READ");
		if(!f)            ERROR("Bad input file handle: %s", fname); 
		if(f->IsZombie()) ERROR("Input file %s can be read, but is zombied. You sure path is correct? Or is it open somewhere else, or the disk is misbehaving?", fname);
		if(!f->IsOpen())  ERROR("Input file %s can be read, but isn't opened. Is it used somewhere else?", fname);

		if(g_loaded_containers.find(cont._name) == g_loaded_containers.end()) {
			g_loaded_containers.insert(cont._name);
			for(auto& base : cont._vc)
				base->Load( f.get() );
		}

		 /* When the clones are created, they will just share the pointer to these objects.
		 * Namely, each thread has a view over the object to conserve memory. 
		 * These objects aren't really flagged as const, and users should abhold this 'contract' */

		/* It does re-open a ROOT file, but this is done on order of ~10 times, which is insignificant overhead
		 * in the setup. */
	}

	/* All other non-TContainer overloads default to a no-op. */
	template<typename T,
		typename std::enable_if<!mnd::is_base_of_template<TContainer, T>::value>::type* = nullptr
	> void LoadTOnceInputs(const T& cont) {
		(void)cont;
	}

}; // TAnalysisProcess

/* Number of available threads via nproc. Optionally taken from `common.mk` 
 * This directive just warns that capacity is over-the top.
 * No point raising a compilation error, since binaries could be just moved
 * or executed from another machine. */
#if !defined(POOL_MAX_THREADS_)
#	define POOL_MAX_THREADS_ 1'234'567U /* Just some arbitrary number. */
#endif

template <
	u32 N,
	typename... Processors
> struct TAnalysisPool final {
	static_assert(N >= 1, "TAnalysisProcess template parameter [1] size < 1? To run singlethreaded, "
		"put first integer template parameter to 1.");
	static_assert(!(N & (N-1)), "TAnalysisProcess template parameter [1] (N-processes) must be a power of two.");

	/* Only allowed ctor. */ 
	explicit TAnalysisPool(TAnalysisProcess<Processors...>& base, u32 _NSlice) : 
		pool{}, NSlice(_NSlice) 
	{
		if(N > POOL_MAX_THREADS_) 
			WARN("TAnalysisProcess template instantiated with capacity %d, which is over-the-top capacity %d. " 
				"Is fine if was compiled on machine X and running on machine Y.", N, POOL_MAX_THREADS_);
		assert((NSlice > 63) &&  "TAnalysisProcess constructor parameter [2] (slice size) must be bigger than 63 to be efficient.");
		
		pool[0] = std::move(base);
		pool[0].Setup();
		for(u32 i=1; i<N; ++i)
			pool[0].Clone(pool[i]);

		/* Once created, force reading of first few events to poke cling again. */
		auto& w = this->Ref();
		w._do_write = false;
		
		u64 nLast = std::min (
			w.GetEntries(), (u64)10
		);
		
		volatile int sink = 0;
		for(u64 evId = 0; evId < nLast; ++evId) {
			w.GetEntry( static_cast<Long64_t>(evId) );
			sink += 1;
		}

		w._do_write = true;
	}

	/* On destructor sweep, write the single objects directly to the file. */
	~TAnalysisPool() { Collect(); Write(); g_loaded_containers.clear(); }

	/**
	 * Perform the dyadic fold of the Output containers, of each full process. 
	 * Process indexed with [0] contains the complete fold. Others are half-folded,
	 * and shouldn't be used any more. Idempotent function. 
	 */
	void Collect() {
		if(_is_collected) return;

		/* Check that all threads are safely merged. */
		for(const auto& p : pool)
			if(! p.IsStopped()) ERROR("Subthread isn't stopped while collector of the pool wants to run.");

		std::vector <
			TAnalysisProcess<Processors...> *
		> refs {};

		for(u32 i=0; i<N; ++i) refs.push_back( &pool[i] );
		dyadic_fold(std::move(refs));

		_is_collected = true;
	}
	
	/**
	 * Write the collected single- wise objects into the output TFile. Idempotent function. 
	 */
	void Write() {
		if( _is_written) return;

		/* Writing out the types for clarity. */
		TAnalysisProcess<Processors...>& process = Ref();
		mnd::IOInfo& info = process.info;
		
		/* First write the RNTuple. Gotta loop over all workers to close their
		 * RNTupleFillContext handles, only then destruct the RNTupleParallelWriter. */
		for(auto it = std::rbegin(pool); it != std::rend(pool); ++it) 
			it->writer.Reset();

		std::unique_ptr<TFile> f = std::make_unique<TFile>( info.out.fname.c_str(), "UPDATE");
		if(!f) 
			ERROR("Cannot open output file at end to write the single- wise objects. %s", info.out.fname.c_str());
		if(f->IsZombie()) 
			ERROR("Opened output file at end to write the single- wise objects is zombied?. %s", info.out.fname.c_str());
		if(!f->IsOpen()) 
			ERROR("Opened output file at end to write the single- wise objects, but is not `IsOpen()` %s", info.out.fname.c_str());
		if(!f->IsWritable())
			ERROR("Opened output file at end to write the single- wise objects, but is not writable? %s", info.out.fname.c_str());
		
		/* Fold the Write call over all subprocesses in pool[0] */
		mnd::for_each_in_tuple(process._proc, [&f](auto& subprocess)
			{
				/* subprocess: TProcessor<Out(Ins...)>& */
				for(auto& o : subprocess.out.GetTOnceVec()) {
					if(!o) ERROR("Requesting to write an object, but is nullptr? Subprocess type: %s",
						mnd::type_name<decltype(subprocess)>().c_str());
					o->Write( f.get() );
				}
			}
		);
		_is_written = true;
	}

	/**
	 * Send a batch of identical `NBatch` number of first entries 
	 * to each of the threads, to set up some initial parameters.
	 */
	void SendOneBatch(u64 startingIndex = 0, u32 NBatch = 0) {
		if(NBatch == 0) NBatch = NSlice;
		
		u64 nLast = std::min (
			pool[0].GetEntries(),
			(u64)NBatch + startingIndex
		);

		ROOT::EnableThreadSafety();
		for(auto& w : pool) {
			w._do_write = false;
			w.Start();
		}

		for(u32 i=0; i<N; ++i) {
			int n_tries = 0;
			auto& w = pool[i];
			mnd::Job job { .first = startingIndex, .last = nLast };
			while(! w.q.push(job) ) {
				std::this_thread::yield();
				++n_tries;
			}
			/* Inspect and warn if somehow pushing a single job needed retries.. */
			if(n_tries != 0) WARN("SendOneBatch: needed %d retries for worker #%u\n", n_tries, i);
		}

		Stop(); /* This blocks until all the threads are joined. */
		for(auto& w : pool) 
			w._do_write = true;
	}

	void Start (
#ifdef __HAS_INDICATORS 
		indicators::ProgressBar& bar,
#endif
		const u64 max_entries = static_cast<u64>(-1)
	) {
		auto& ref_process = Ref();

		u64 nentries = std::min (
			ref_process.GetEntries(),
			max_entries
		);
		WARN("Starting the analysis with " EMPH1(%lu) 
			" entries, split over " EMPH1(%u) " workers.\n", nentries, N);

		ROOT::EnableThreadSafety();
		for(auto& w : pool)
			w.Start();

#ifdef __HAS_INDICATORS
		indicators::show_console_cursor(false);
#endif
		auto* w = &pool[0]; u32 next = 0;
		for(u64 i = 0; i < nentries; i+=NSlice) {
			mnd::Job j {
				.first = i,                             // Included.
				.last  = std::min(i + NSlice, nentries) // Excluded.
			};

			/* Choose a process thread; round-robin. 
			 * If currently selected thread has capped queue, do a short spin and try next one. */
			for(; w = &pool[next], !w->q.push(j); ++next, next %= N) {
#ifdef __HAS_SMALL_INTEL_SPIN
				_mm_pause(); /* Short pause, 100-150 clock cycles. */
#else
				{}           /* Do nothing; don't yield or reschedule - this is ~100 us latency. */
#endif
			}
#ifdef __HAS_INDICATORS
			mnd::PrintProgress(bar, j.last, nentries, NSlice-1);
#endif
		}

		Stop();

#ifdef __HAS_INDICATORS
		bar.mark_as_completed();
		indicators::show_console_cursor(true);
#endif

	} // void Start(...)

	void Stop() {
		for(auto& w : pool) w.Stop(); 
	}
	
	decltype(auto) GetPool()       noexcept { return ( this->pool ); }
	decltype(auto) GetPool() const noexcept { return ( this->pool ); }
	
	TAnalysisProcess<Processors...>&       Ref()       noexcept { return pool[0]; }
	TAnalysisProcess<Processors...> const& Ref() const noexcept { return pool[0]; }

private:
	std::array <
		TAnalysisProcess<Processors...>, N
	> pool;
	u32 NSlice;

	bool _is_collected { false };
	bool _is_written   { false };

	template<typename T>
	void dyadic_fold(std::vector<T*>&& v) {
		size_t Nv = v.size();
		if(Nv & (Nv-1)) ERROR("Dyadic fold container size ill-formed, is %zu, but should be power of 2.", Nv);

		if(Nv == 1) return;
		
		const size_t half = Nv / 2;
		std::vector<T*> next(half);

		for(size_t i=0; i<half; ++i) {
			v[ 2*i ] -> Collect( (const T&)(*v[2*i + 1]) );
			next[i] = v[2*i];
		}

		dyadic_fold(std::move(next));
	}
};

/* ===============================================================
 * =============================================================== */
/* Invariant API for both multithreaded and singlethreaded modes.  */
/* NSlice field is not really used, but still keep it to keep identical calls across the
 * specialization. */

template<typename... Processors>
struct TAnalysisPool<1, Processors...> final {
	explicit TAnalysisPool(TAnalysisProcess<Processors...>& base, u32 _NSlice) : 
		pool{}, NSlice(_NSlice) 
	{
		pool[0] = std::move(base);
		pool[0].Setup();
	}

	/* On destructor sweep, write the single objects directly in the file. */
	~TAnalysisPool() { Collect(); Write(); }
	
	void Collect() {}

	/**
	 * Write the collected single- wise objects into the output TFile. Idempotent function. 
	 */
	void Write() {
		if( _is_written) return;

		/* Writing out the types for clarity. */
		TAnalysisProcess<Processors...>& process = Ref();
		mnd::IOInfo& info = process.info;
		
		/* First write the RNTuple. */
		process.writer.Reset();	

		std::unique_ptr<TFile> f = std::make_unique<TFile>( info.out.fname.c_str(), "UPDATE");
		if(!f || f->IsZombie()) 
			ERROR("Cannot open output file at end to write the single- wise objects. %s", info.out.fname.c_str());
		if(!f->IsOpen()) 
			ERROR("Opened output file at end to write the single- wise objects, but is not `IsOpen()` %s", info.out.fname.c_str());
		if(!f->IsWritable())
			ERROR("Opened output file at end to write the single- wise objects, but is not writable %s", info.out.fname.c_str());
		
		/* Fold the Write call over all subprocesses in pool[0] */
		mnd::for_each_in_tuple(process._proc /* tuple<TProcessor<Out(Ins..)> */, [&f](auto& subprocess)
			{
				/* subprocess: TProcessor<Out(Ins...)>& */
				for(auto& o : subprocess.out.GetTOnceVec()) {
					if(!o) ERROR("Requesting to write an object, but is nullptr? Subprocess type: %s",
						mnd::type_name<decltype(subprocess)>().c_str());
					o->Write( f.get() );
				}
			}
		);
		_is_written = true;
	}
	
	/**
	 * Send a batch of identical `NBatch` number of first entries 
	 * to the underlying process, to set up some initial parameters.
	 */
	void SendOneBatch(u64 startingIndex = 0, u32 NBatch = 0) {
		if(NBatch == 0) NBatch = (NSlice > 0) ? NSlice : 2048;
		auto& process = Ref();
		u64 nLast = std::min (
			process.GetEntries(),
			(u64)NBatch + startingIndex
		);

		for(u64 evId = startingIndex; evId < nLast; ++evId) {
			process.GetEntry( static_cast<Long64_t>(evId) );

			std::apply([](auto&... ps) {
					(..., ps.ProcessEntry()); 
				}, process._proc
			);
			/* Don't fill. */
		}
	}

	/* In singlethreaded case, don't poke the underlying std::thread of the process,
	 * just call ProcessEntry() directly. */
	void Start (
#ifdef __HAS_INDICATORS 
		indicators::ProgressBar& bar,
#endif
		const u64 max_entries = static_cast<u64>(-1)
	) {
		auto& process = Ref();

		u64 nentries = std::min (
			process.GetEntries(),
			max_entries
		);

		WARN("Starting the singlethreaded analysis with " EMPH1(%lu) " entries.\n", nentries);
		u64 n_print_every = ((NSlice > 0) ? NSlice : 512); 
		for(u64 evId = 0; evId < nentries; ++evId) {
			process.GetEntry( static_cast<Long64_t>(evId) );

#ifdef __HAS_INDICATORS
			mnd::PrintProgress(bar, evId, nentries, n_print_every);
#endif
			std::apply([](auto&... ps) {
					(..., ps.ProcessEntry()); 
				}, process._proc
			);
			process.writer.ctx->Fill( *process.writer.entry );
		}
	}
	
	void Stop() { Ref().Stop(); } /* Effectively no-op; define it anyway to keep API invariance. */

	decltype(auto) GetPool()       { return ( this->pool ); }
	decltype(auto) GetPool() const { return ( this->pool ); }

	TAnalysisProcess<Processors...>&       Ref()       noexcept { return pool[0]; }
	TAnalysisProcess<Processors...> const& Ref() const noexcept { return pool[0]; }
	
private:
	std::array <
		TAnalysisProcess<Processors...>, 1
	> pool;
	u32 NSlice;

	/* bool _is_collected { false }; */
	bool _is_written   { false };
};

#endif /* __MONAD_INCLUDE_HXX__ */
