#ifndef AVAKAR_MONADIC_VARIANT_H
#define AVAKAR_MONADIC_VARIANT_H

#include <avakar/meta.h>
#include <type_traits>
#include <exception>
#include <stdlib.h>

namespace avakar {

struct bad_variant_access
	: std::exception
{
	char const * what() const noexcept override;
};

template <size_t I, typename T>
struct variant_alternative;

template <size_t I, typename T>
using variant_alternative_t = typename variant_alternative<I, T>::type;

template <typename T>
struct in_place_type_t
{
};

template <size_t I>
struct in_place_index_t
{
};

template <typename... Types>
struct monadic_variant
{
	using types = meta::list<Types..., std::exception_ptr>;
	static constexpr size_t exception_pos = sizeof...(Types);

	template <bool _dummy = false, typename = typename std::enable_if<_dummy
		|| std::is_constructible<meta::sub_t<types, 0>>::value
		|| std::is_void<meta::sub_t<types, 0>>::value
		>::type>
	monadic_variant() noexcept;

	template <typename T, typename = meta::choose_overload_t<T, types>>
	monadic_variant(T && t) noexcept;

	template <typename T, typename... Args, typename = typename std::enable_if<
		meta::contains_unique<T, types>::value
		&& (std::is_void<T>::value || std::is_constructible<T, Args...>::value)
		>::type>
	monadic_variant(in_place_type_t<T>, Args &&... args) noexcept;

	template <size_t I, typename... Args, typename = typename std::enable_if<
		std::is_void<meta::sub_t<types, I>>::value
		|| std::is_constructible<meta::sub_t<types, I>, Args...>::value
		>::type>
	monadic_variant(in_place_index_t<I>, Args &&... args) noexcept;

	monadic_variant(monadic_variant const & o) noexcept;
	monadic_variant(monadic_variant && o) noexcept;

	~monadic_variant();

	size_t index() const noexcept;

private:
	size_t index_;
	meta::aligned_storage_t<types> storage_;

	friend struct _variant_access;
};

template <typename T, typename... Types>
auto get(monadic_variant<Types...> const & v)
	-> typename std::enable_if<meta::contains_unique<T, meta::list<Types..., std::exception_ptr>>::value, T>::type;

template <size_t I, typename... Types>
auto get(monadic_variant<Types...> const & v)
	-> variant_alternative_t<I, monadic_variant<Types...>>;

}

#include "../../src/monadic_variant_impl.h"

#endif // AVAKAR_LIBAWAIT_MONADIC_VARIANT_H
