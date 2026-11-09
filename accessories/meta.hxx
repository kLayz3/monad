#pragma once

#include <type_traits>
#include <array>
#include <iterator>
#include <functional>
#include <utility>

#include "typedefs.hxx"

/* Here we stash all the nice metaprogramming utilities of monad.
 * Common type_traits do not go into the meta:: namespace. */

namespace mnd {

#if __cplusplus >= 202002L /* Mirrors STL-terminology */
	template<typename T>
	using remove_cvref = std::remove_cvref<T>;
#else
	template<typename T>
	struct remove_cvref {
		using type = std::remove_cv_t<std::remove_reference_t<T>>;
	};
#endif

template<typename T>
using remove_cvref_t = typename remove_cvref<T>::type;

template< template<typename...> typename Base, typename Derived>
struct _is_base_of_template_impl {
	template<typename... Ts>
	static constexpr std::true_type test(Base<Ts...>& );
	static constexpr std::false_type test(...);
	using type = decltype(test(std::declval<Derived&>()));
};

template< template<typename...> typename Base, typename Derived,
	typename Bare = remove_cvref_t<Derived>>
using is_base_of_template = typename _is_base_of_template_impl<Base, Bare>::type;


namespace meta {

template<typename T>
struct is_an_array : std::false_type {};
template<typename T, std::size_t N>
struct is_an_array<T[N]> : std::true_type {
	using underlying_type = T;
	static constexpr std::size_t size = N;
};
template<typename T, std::size_t N>
struct is_an_array<std::array<T,N>> : std::true_type {
	using underlying_type = T;
	static constexpr std::size_t size = N;
};
template<typename T>
constexpr bool is_an_array_v = is_an_array<T>::value;

template<typename T>
struct is_std_array : std::false_type{};
template<typename T, std::size_t N>
struct is_std_array<std::array<T, N>> : std::true_type {};

template<typename T>
struct ToStdArray { using type = T; };
template<typename T, std::size_t N>
struct ToStdArray<T[N]> { using type = std::array<typename ToStdArray<T>::type, N>; };
template<typename T>
using ToStdArray_t = typename ToStdArray<T>::type;

/* Range is simply defined as any type that has a begin() and end() iterator */
template<typename T, typename = void>
struct is_range : std::false_type {};
template<typename T>
struct is_range<T, std::void_t <
	decltype( std::begin(std::declval<T&>()) ),
	decltype( std::end(std::declval<T&>()) ),
	typename std::iterator_traits<
		decltype(std::begin(std::declval<T&>()))
	>::value_type
>> : std::true_type {
	using underlying_type = typename std::iterator_traits<
		decltype( std::begin(std::declval<T&>()) )
	>::value_type;
};

template<typename, typename = void>
struct is_sized_range : std::false_type {};
template<typename T>
struct is_sized_range <
	T,
	std::void_t <
		typename is_range<T>::underlying_type,
		decltype(std::size(std::declval<T&>()))
	>
> : std::true_type {
	using underlying_type = typename is_range<T>::underlying_type;
};

template<typename, typename = void>
struct is_indexable_range : std::false_type {};
template<typename T>
struct is_indexable_range <
	T,
	std::void_t <
		typename is_sized_range<T>::underlying_type,
		decltype(std::declval<T&>()[std::declval<std::size_t>()])
	>
> : std::true_type {
	using underlying_type = typename is_range<T>::underlying_type;
};

template<typename, typename = std::void_t<>>
struct has_value_type : std::false_type {};
template<typename U>
struct has_value_type<U, std::void_t<typename U::value_type>> : std::true_type {};
template<typename T, typename = void>
struct has_add_unary_op : std::false_type {};
template<typename T>
struct has_add_unary_op<T, std::void_t<
	decltype( std::declval<T&>().operator+=(std::declval<const T&>() ) )
>> : std::true_type {};

template<typename T, typename = void>
struct has_add_binary_op : std::false_type {};
template<typename T>
struct has_add_binary_op<T, std::void_t<
	decltype( std::declval<T>() + std::declval<T>() )
>> : std::bool_constant<
	std::is_convertible_v<
		decltype( std::declval<T>() + std::declval<T>() ),
		T
	>
> {};
template<typename T, typename = void>
struct has_subt_binary_op : std::false_type {};
template<typename T>
struct has_subt_binary_op<T, std::void_t<
	decltype( std::declval<T>() - std::declval<T>() )
>> : std::bool_constant<
	std::is_convertible_v<
		decltype( std::declval<T>() - std::declval<T>() ),
		T
	>
> {};
template<typename T, typename = void>
struct has_mult_binary_op : std::false_type {};
template<typename T>
struct has_mult_binary_op<T, std::void_t<
	decltype( std::declval<T>() * std::declval<T>() )
>> : std::bool_constant<
	std::is_convertible_v<
		decltype( std::declval<T>() * std::declval<T>() ),
		T
	>
> {};

template<typename T, typename = void>
struct has_add_div_unary_op : std::false_type {};
template<typename T>
struct has_add_div_unary_op<T, std::void_t <
	std::conjunction <
		decltype( std::declval<T&>().operator+=(std::declval<const T&>() ) ) ,
		decltype( std::declval<T&>().operator/=(std::declval<const int>() ) )
	>
>> : std::true_type {};

} // namespace meta

template<
	typename T,
	typename Bare = remove_cvref_t<T>
> constexpr std::size_t size() noexcept {
	static_assert(meta::is_an_array_v<Bare>, "Passed type must either be an array reference `T (&)[N]` or `std::array<T,N>&` .");
	return meta::is_an_array<Bare>::size;
}
template<
	typename T,
	typename Bare = remove_cvref_t<T>
> constexpr std::size_t size(const T& ) noexcept {
	static_assert(meta::is_an_array_v<Bare>, "Passed type must either be an array reference `T (&)[N]` or `std::array<T,N>&` .");
	return meta::is_an_array<Bare>::size;
}
template<
	typename T,
	typename Bare = remove_cvref_t<T>
> constexpr std::size_t len() noexcept {
	static_assert(meta::is_an_array_v<Bare>, "Passed type must either be an array reference `T (&)[N]` or `std::array<T,N>&` .");
	return meta::is_an_array<Bare>::size;
}
template<
	typename T,
	typename Bare = remove_cvref_t<T>
> constexpr std::size_t len(const T& ) noexcept {
	static_assert(meta::is_an_array_v<Bare>, "Passed type must either be an array reference `T (&)[N]` or `std::array<T,N>&` .");
	return meta::is_an_array<Bare>::size;
}

/* Next few routines serve to violently unroll loops.
 * Namely, GCC sometimes even in -03 refuses to unroll small loops (I don't understand why).
 * The pragma unroll only works if the loop size is known at preproc time.
 * ... Also unrolling over a static-sized container. :-) */
template<std::size_t Offset, typename Sequence, bool reversed>
struct offset_sequence;

template<std::size_t Offset, std::size_t... Is>
struct offset_sequence<Offset, std::index_sequence<Is...>, false> {
	using type = std::index_sequence<(Offset + Is)...>;
	// a sequence: Offset, Offset+1, ..., Offset-1 + sizeof...(Is)
};
template<std::size_t Offset, std::size_t... Is>
struct offset_sequence<Offset, std::index_sequence<Is...>, true> {
	using type = std::index_sequence<(Offset - Is)...>;
	// a sequence: Offset, Offset-1, ..., Offset+1 - sizeof...(Is)
};

template<
	std::size_t Begin,
	std::size_t End
> using make_index_range =
	typename offset_sequence <
		Begin,
		std::make_index_sequence <
			(Begin <= End) ? (End-Begin) : (Begin-End)
		>,
		(Begin > End)
	>::type; /*     std::index_sequence<Begin, Begin+1, ..., End-1> */
	         /* or: std::index_sequence<Begin, Begin-1, ..., End+1> */

template <
	typename Callable,
	std::size_t... Is
> constexpr void _static_for_impl(Callable&& f, std::index_sequence<Is...>)
	noexcept((std::is_nothrow_invocable <
		Callable&, std::integral_constant<std::size_t, Is>
	>::value && ...))
{
	(..., f(std::integral_constant<std::size_t, Is>{}));
}
template <
	std::size_t Begin,
	std::size_t End,
	typename Callable
> constexpr void static_for(Callable&& f)
	noexcept( noexcept(_static_for_impl(
		std::forward<Callable>(f),
		make_index_range<Begin, End>{}
	)))
{
	_static_for_impl(std::forward<Callable>(f), make_index_range<Begin, End>{});
}

/* Example:                                          .LCO: .string "%zu, "
 * static_for<2,6>([](auto I) {                      sub     rsp, 8
     constexpr std::size_t i = decltype(I)::value;   mov     esi, 4
     printf("%zu, ", i*i);                           mov     edi, OFFSET FLAT:.LCO
 * });                                               xor     eax, eax
 * // 4, 9, 16, 25,                                  call    printf
 *                                                   mov     esi, 9 ... */

/* Functor invocation simply gets folded over the entire static-sized container. */
template <
	typename Arr,
	typename Callable,
	std::size_t... Is
> constexpr void _static_for_each_impl(Arr&& arr, Callable&& f, std::index_sequence<Is...>)
	noexcept((noexcept(std::invoke(
		std::declval<Callable&>(),
		std::declval<Arr>()[Is]
	)) && ...))
{
	(..., f(std::forward<Arr>(arr)[Is]));
}

template <
	typename Arr,
	typename Callable
> constexpr void static_for_each(Arr&& arr, Callable&& f)
	noexcept( noexcept(_static_for_each_impl(
		std::forward<Arr>(arr),
		std::forward<Callable>(f),
		std::make_index_sequence <
			mnd::size<Arr>()
		>{}
	)))
{
	constexpr size_t N = mnd::size<Arr>();
	_static_for_each_impl(std::forward<Arr>(arr), std::forward<Callable>(f),
		std::make_index_sequence<N>{});
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

template<typename>
inline constexpr bool always_false_v = false;

using Unroll = BinaryOpt;
template <
	Unroll U = Unroll::No,
	typename ResultType = void, // If left unspecified, result type is the underlying type of the array. */
	// .. all types below this line deduced :-)
	typename Arr,
	typename T = typename meta::is_an_array<remove_cvref_t<Arr>>::underlying_type,
	typename R = std::conditional_t<std::is_void_v<ResultType>, T, ResultType>
> constexpr R sum(const Arr& arr) noexcept /* We don't explicitly check if its noexcept.. TODO. */
{
	static_assert(meta::has_add_binary_op<R>::value, "Underlying type must have binary addition operator well defined.");
	if constexpr(U == Unroll::No) {
		R result = static_cast<R>(0);
		for(const auto& value : arr)
			result = std::move(result) + static_cast<R>(value);
		return result;
	} else {
		R sum = static_cast<R>(0);
		static_for_each(arr, [&sum](const T& val) { sum = std::move(sum) + static_cast<R>(val); });
		return sum;
	}
}

template <
	Unroll U = Unroll::No,
	typename ResultType = void, /* Check API above. */
	// .. all types below this line deduced :-)
	typename Arr,
	typename R = decltype( mnd::sum<U,ResultType>(std::declval<const Arr&>()) )
> constexpr R mean(const Arr& arr) noexcept {
	return mnd::sum<U, ResultType>(arr) / static_cast<R>(mnd::size<Arr>());
}

/* The mnd::median returns by *value*, as is normally used with basic arithmetic
 * types. For custom types that have defined operators it will also work, but the
 * noexcept specifier might be an overkill... */
template<typename T,
	typename U = remove_cvref_t<T>,
	std::size_t N = meta::is_an_array<U>::size,
	char(*)[N % 2] = nullptr
> constexpr auto median(const T& sorted_arr) noexcept(
	std::is_arithmetic_v<typename meta::is_an_array<U>::underlying_type>
) {
	return sorted_arr[N/2];
}

/* Assume it doesn't overflow. */
template<typename T,
	typename U = remove_cvref_t<T>,
	std::size_t N = meta::is_an_array<U>::size,
	char(*)[!(N % 2)] = nullptr
> constexpr auto median(const T& sorted_arr) noexcept(
	std::is_arithmetic_v<typename meta::is_an_array<U>::underlying_type>
) {
	static_assert(N > 0, "mnd::median requires a nonempty array.");
	return (sorted_arr[N/2] + sorted_arr[N/2 - 1]) / 2;
}

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

/* Make a constexpr array constructor, filled with a default value, other than `{}` */
template<typename T, std::size_t... I>
constexpr std::array<T, sizeof...(I)>
_make_filled_array_impl(T value, std::index_sequence<I...>) {
	return { ((void)I, value)... };
}
template<typename T, std::size_t N>
constexpr std::array<T, N> make_filled_array(T value) {
	return _make_filled_array_impl<T>(value, std::make_index_sequence<N>{});
}

/* Maps a dynamic range of underlying type <T> via the functor fn: T -> U, and collects
 * the results as std::vector<U>. This is *not* a lazy evaluator! */
template<
	typename Range,
	typename Callable,
	typename std::enable_if_t<!meta::is_an_array_v<Range>>* = nullptr
> auto map(const Range& input, Callable&& fn) {
	static_assert(meta::is_range<const Range>::value, "Type T must support a const range iteration.");
	using U = remove_cvref_t<
		std::invoke_result_t<Callable&, decltype(*std::begin(input))>
	>;

	std::vector<U> output;
	if constexpr(meta::is_sized_range<const Range>::value) {
		output.reserve(std::size(input));
	}

	for(auto&& value : input)
		output.emplace_back(
			std::invoke(fn, std::forward<decltype(value)>(value))
		);
	return output;
}

namespace detail {
template<typename Arr, typename Callable>
using map_array_value_t = std::decay_t<
	std::invoke_result_t<Callable&, decltype(*std::begin(
		std::declval<const Arr&>()))>
>;

template<typename Arr, typename Callable, std::size_t... I>
constexpr auto map_array(
	const Arr& input,
	Callable& fn,
	std::index_sequence<I...>
) noexcept(noexcept(
	std::array<map_array_value_t<Arr, Callable>, sizeof...(I)>{
		fn(input[I])...
	}
)) {
	using U = map_array_value_t<Arr, Callable>;
	return std::array<U, sizeof...(I)>{ fn(input[I])... };
}
} // namespace detail

/* Maps a static range of size N and underlying type <T> via the functor fn: T -> U, and collects
 * the results as std::array<U,N>. This is *not* a lazy evaluator! */
template<
	typename Arr,
	typename Callable,
	typename std::enable_if_t<meta::is_an_array_v<Arr>>* = nullptr
> constexpr auto map(const Arr& input, Callable&& fn) noexcept(
	noexcept(detail::map_array(
		input, fn,
		std::make_index_sequence<meta::is_an_array<Arr>::size>{}
	))
) {
	return detail::map_array(input, fn,
		std::make_index_sequence<meta::is_an_array<Arr>::size>{}
	);
}

template<typename T,
	typename U = remove_cvref_t<T>
> constexpr int FindIndex(const T& arr, const typename meta::is_an_array<U>::underlying_type& val) noexcept(
	noexcept(static_cast<bool>(arr[0] == val))
) {
	static_assert(meta::is_an_array_v<U>, "Type T must be a C-style array or 'std::array<T,N>'");
	constexpr std::size_t N = meta::is_an_array<U>::size;
	for(std::size_t i=0; i < N; ++i)
		if(arr[i] == val) return i;
	return -1;
}

namespace detail {
template<typename Ordering>
constexpr bool is_ordered(Ordering&& )  noexcept { return true; }
template<typename Ordering, typename T>
constexpr bool is_ordered(Ordering&& , T&& ) noexcept { return true; }

template<typename Ordering, typename T, typename U, typename... Rest>
constexpr bool is_ordered(Ordering&& o, T&& a0, U&& a1, Rest&&... rest) {
    return ( o(std::forward<T>(a0), a1) )
        && is_ordered(o, std::forward<U>(a1), std::forward<Rest>(rest)...);
}
}; // namespace detail

/* Check that a sequence of arguments: a0,a1,a2,... aN is strictly ascending:
 * a0 < a1 < a2 < ... < aN . Requires less-than < operator*/
template<
	typename... Ts,
	typename std::enable_if_t<
		sizeof...(Ts) != 1 ||
		!(meta::is_range<remove_cvref_t<Ts>>::value || ...)
	>* = nullptr
> constexpr bool is_strictly_ascending(Ts&&... ts) {
    return detail::is_ordered(
        [](const auto& lhs, const auto& rhs) -> bool { return lhs < rhs; },
        std::forward<Ts>(ts)...
    );
}
/* Check that a sequence of arguments: a0,a1,a2,... aN is strictly descending:
 * a0 > a1 > a2 > ... > aN . Requires greater-than > operator. */
template<
	typename... Ts,
	typename std::enable_if_t<
		sizeof...(Ts) != 1 ||
		!(meta::is_range<remove_cvref_t<Ts>>::value || ...)
	>* = nullptr
> constexpr bool is_strictly_descending(Ts&&... ts) {
    return detail::is_ordered(
        [](const auto& lhs, const auto& rhs) -> bool { return lhs > rhs; },
        std::forward<Ts>(ts)...
    );
}

template <
	typename Range,
	typename = std::enable_if_t<meta::is_range<Range>::value>
> constexpr bool is_strictly_ascending(const Range& container) {
	/* Loop over via iterators. STL-one std::is_sorted is not constexpr in C++17. */
	auto first = std::begin(container);
	auto last = std::end(container);
	if(first != last) {
        auto next = first;
        while(++next != last)
        {
            if( !(*first < *next))
                return false;
            first = next;
        }
    }
    return true;
}
template <
	typename Range,
	typename = std::enable_if_t<meta::is_range<Range>::value>
> constexpr bool is_strictly_descending(const Range& container) {
	/* Loop over via iterators. STL-one std::is_sorted is not constexpr in C++17. */
	auto first = std::begin(container);
	auto last = std::end(container);
	if(first != last) {
        auto next = first;
        while(++next != last)
        {
            if( !(*first > *next))
                return false;
            first = next;
        }
    }
    return true;
}


} // namespace mnd
