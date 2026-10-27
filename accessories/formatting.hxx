#pragma once

#include "typedefs.hxx"

#include <memory>
#include <vector>
#include <array>
#include <map>
#include <unordered_map>
#include <unordered_set>
#include <queue>
#include <stack>
#include <typeinfo>
#include <cstdlib>
#include <deque>
#include <list>
#include <ostream>
#include <set>
#include <sstream>
#include <string>
#include <tuple>
#include <type_traits>
#include <utility>

#ifndef _MSC_VER
#	include <cxxabi.h>
#endif

namespace mnd {

/* Returns the name of the type passed, also adding
 * ref-cv qualifiers. Demangles templated types, too :-)
 * To print type of an entity `a`, just do:
 *   mnd::type_name<decltype(a)>();
 * This works only for entities, not for expressions!
 * https://en.cppreference.com/cpp/language/decltype §1 */
template<typename T>
MND_NEVER_INLINE std::string type_name() {
	using U = std::remove_reference_t<T>;

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
	if constexpr(std::is_const_v<U>)
		r += " const";
	if constexpr(std::is_volatile_v<U>)
		r += " volatile";
	if constexpr(std::is_lvalue_reference_v<T>)
		r += "&";
	else if constexpr(std::is_rvalue_reference_v<T>)
		r += "&&";
	return r;
}

#define MND_SELF_TYPE_CSTR \
	::mnd::type_name<std::remove_cv_t<std::remove_reference_t<decltype(*this)>>>().c_str()

namespace fmt {
namespace detail {

template<typename T>
struct range_format {
	static constexpr bool enabled = false;
	static constexpr bool keyed = false;
};
template<typename T, size_t N>
struct range_format<std::array<T, N>> {
	static constexpr bool enabled = true;
	static constexpr bool keyed = false;
};
/* Copy-pasta from https://en.cppreference.com/cpp/container §Iterator invalidation */
#define MND_FMT_SEQUENCE(Container) \
	template<typename... Args> \
	struct range_format<Container<Args...>> { \
		static constexpr bool enabled = true; \
		static constexpr bool keyed = false; \
	}
#define MND_FMT_MAP(Container) \
	template<typename... Args> \
	struct range_format<Container<Args...>> { \
		static constexpr bool enabled = true; \
		static constexpr bool keyed = true; \
	}

MND_FMT_SEQUENCE(std::vector);
MND_FMT_SEQUENCE(std::deque);
MND_FMT_SEQUENCE(std::list);
MND_FMT_SEQUENCE(std::set);
MND_FMT_SEQUENCE(std::multiset);
MND_FMT_MAP(std::map);
MND_FMT_MAP(std::multimap);
MND_FMT_SEQUENCE(std::unordered_set);
MND_FMT_SEQUENCE(std::unordered_multiset);
MND_FMT_MAP(std::unordered_map);
MND_FMT_MAP(std::unordered_multimap);

#undef MND_FMT_MAP
#undef MND_FMT_SEQUENCE

} // namespace detail

/* FWD declarations so nesting works. Long live recursion :) */
template<typename Range,
	typename std::enable_if_t<detail::range_format<Range>::enabled>* = nullptr
> std::ostream& operator<<(std::ostream&, const Range&);

/* Special stack/queue/priority_queue overloads. */
template<typename T, typename Container>
std::ostream& operator<<(std::ostream&, const std::stack<T, Container>&);
template<typename T, typename Container>
std::ostream& operator<<(std::ostream&, const std::queue<T, Container>&);
template<typename T, typename Container, typename Compare>
std::ostream& operator<<(std::ostream&, const std::priority_queue<T, Container, Compare>&);

/* Also general span. */
template<typename T>
std::ostream& operator<<(std::ostream&, mnd::span<T>);

/* Tuple+pair */
template<typename... Ts>
std::ostream& operator<<(std::ostream& , const std::tuple<Ts...>& );
template<typename T1, typename T2>
std::ostream& operator<<(std::ostream& , const std::pair<T1,T2>& );

namespace detail {

template<bool Keyed, typename Iterator>
std::ostream& output_range(
	std::ostream& os,
	Iterator first,
	Iterator last
) {
	if constexpr(Keyed) os << '{';
	else                os << '[';

	if(first != last) {
		if constexpr(Keyed)
			os << first->first << ": " << first->second;
		else
			os << *first;

		while(++first != last) {
			os << ", ";
			if constexpr(Keyed)
				os << first->first << ": " << first->second;
			else
				os << *first;
		}
	}

	if constexpr(Keyed) return os << '}';
	else                return os << ']';
}

template<typename Adaptor, typename GetNext>
std::ostream& output_adaptor(
	std::ostream& os,
	Adaptor copy,
	GetNext get_next
) {
	os << '[';

	if(!copy.empty()) {
		os << get_next(copy);
		copy.pop();
	
		while(!copy.empty()) {
			os << ", " << get_next(copy);
			copy.pop();
		}
	}

	return os << ']';
}

/* Format tuple elements with indices [first, last>. Handles nested tuples, too! */
template<typename... Ts>
std::ostream& print_tuple(
	std::ostream& os,
	const std::tuple<Ts...>& tup,
	size_t first = 0,            // first index to print
	size_t last  = sizeof...(Ts) // first-after-last index to print.
) {
	using Tuple = std::tuple<Ts...>;
	constexpr size_t N = sizeof...(Ts);

	if(first > last || last > N)
		ERROR("Invalid tuple '%s' print range? Requested [%zu, %zu>\n",
			mnd::type_name<Tuple>().c_str(), first, last);

	os << '[';

	size_t i = 0;
	std::apply([&](const auto&... values) {
		([&] {
			if(i >= first && i < last) {
				if(i != first) os << ", ";
				os << values;
			}
			++i;
		}(), ...);
	}, tup);
	return os << ']';
}

} // namespace detail

template<typename Range,
	typename std::enable_if_t<detail::range_format<Range>::enabled>*
> std::ostream& operator<<(std::ostream& os, const Range& range) {
	return detail::output_range<detail::range_format<Range>::keyed>(
		os, range.begin(), range.end()
	);
}
template<typename T>
std::ostream& operator<<(std::ostream& os, mnd::span<T> values) {
	return detail::output_range<false>(
		os, values.begin(), values.end()
	);
}
/* ^^^ Not exposing generic: std::ostream& operator<<(std::ostream& , const T (&)[N] )
 * as it would blow up with most standard overloads, even the "Hello World" breaks then... */

/* Special overloads for stack/queue/prio_queue as these don't expose iteration.
 * We inspect them by taking a copy and popping it down. */
template<typename T, typename Container>
std::ostream& operator<<(std::ostream& os, const std::stack<T, Container>& v) {
	static_assert(std::is_copy_constructible_v<T> && std::is_copy_constructible_v<Container>,
		"Printing the stack<T,C> requires copy-constructible underlying element type T, and container itself.");
	return detail::output_adaptor(os, v,
		[](const auto& a) -> decltype(auto) { return a.top(); }
	);
}
template<typename T, typename Container>
std::ostream& operator<<(std::ostream& os, const std::queue<T, Container>& v) {
	static_assert(std::is_copy_constructible_v<T> && std::is_copy_constructible_v<Container>,
		"Printing the queue<T,C> requires copy-constructible underlying element type T, and container itself.");
	return detail::output_adaptor(os, v,
		[](const auto& a) -> decltype(auto) { return a.front(); }
	);
}
template<typename T, typename Container, typename Compare>
std::ostream& operator<<(
	std::ostream& os,
	const std::priority_queue<T, Container, Compare>& v
) {
	static_assert(std::is_copy_constructible_v<T> && std::is_copy_constructible_v<Container>,
		"Formatting the priority_queue<T,C,Comp> requires copy-constructible underlying element type T, and container itself.");
	return detail::output_adaptor(os, v,
		[](const auto& a) -> decltype(auto) { return a.top(); }
	);
}

template<typename... Ts>
std::ostream& operator<<(std::ostream& os, const std::tuple<Ts...>& tup) {
	return detail::print_tuple(os, tup);
}
template<typename T1, typename T2>
std::ostream& operator<<(std::ostream& os, const std::pair<T1,T2>& p) {
	return os << '[' << p.first << ", " << p.second << ']';
}

} // namespace fmt
using fmt::operator<<;

template<typename T,
	typename = std::void_t<
		decltype(std::declval<std::ostream&>() << std::declval<const T&>())
	>
> std::string streamable(const T& obj) {
	std::ostringstream os;
	os << obj;
	return os.str();
}

/* Concatenates bunch of streamable arguments which can be lvalues, statics, etc. and returns an owned std::string. */
template<typename... Args>
std::string sstrcat(Args&&... args) {
	std::ostringstream oss{};
	(oss << ... << std::forward<Args>(args));
	return oss.str();
}

} // namespace mnd

#ifdef MND_EXPORT_OSTREAM_G_NAMESPACE
using mnd::fmt::operator<<;
#endif
