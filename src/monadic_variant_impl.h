#include <new>
#include <utility>

namespace avakar {

struct _variant_access
{
	template <typename... Types>
	static void * storage(monadic_variant<Types...> & v)
	{
		return &v.storage_;
	}

	template <typename... Types>
	static void const * storage(monadic_variant<Types...> const & v)
	{
		return &v.storage_;
	}
};

struct _monadic_variant_storage
{
	template <typename T, typename... Args>
	static auto construct(meta::item<T>, void * p, Args &&... args) noexcept
		-> typename std::enable_if<std::is_nothrow_constructible<T, Args...>::value && !std::is_reference<T>::value, bool>::type
	{
		new(p) T(std::forward<Args>(args)...);
		return true;
	}

	template <typename T, typename... Args>
	static auto construct(meta::item<T>, void * p, Args &&... args) noexcept
		-> typename std::enable_if<!std::is_nothrow_constructible<T, Args...>::value && !std::is_reference<T>::value, bool>::type
	{
		try
		{
			new(p) T(std::forward<Args>(args)...);
			return true;
		}
		catch (...)
		{
			new(p) std::exception_ptr(std::current_exception());
			return false;
		}
	}

	template <typename T, typename... Args>
	static bool construct(meta::item<T &>, void * p, T & v) noexcept
	{
		new(p) T *(&v);
		return true;
	}

	template <typename... Args>
	static bool construct(meta::item<void>, void * p) noexcept
	{
		return true;
	}

	template <typename T>
	static void destroy(meta::item<T>, void * p) noexcept
	{
		static_cast<T *>(p)->~T();
	}

	template <typename T>
	static void destroy(meta::item<void>, void * p) noexcept
	{
		(void)p;
	}

	template <typename T>
	static void destroy(meta::item<T &>, void * p) noexcept
	{
		(void)p;
	}

	template <typename T>
	static T & get(meta::item<T>, void * p) noexcept
	{
		return *static_cast<T *>(p);
	}

	template <typename T>
	static T const & get(meta::item<T>, void const * p) noexcept
	{
		return *static_cast<T const *>(p);
	}

	template <typename T>
	static T & get(meta::item<T &>, void const * p) noexcept
	{
		return **static_cast<T * const *>(p);
	}

	template <typename T>
	static T & get(meta::item<T &>, void * p) noexcept
	{
		return **static_cast<T * const *>(p);
	}

	static void get(meta::item<void>, void const * p) noexcept
	{
	}

	template <typename Dst, typename Src>
	static bool copy(meta::item<Dst> d, void * dst, meta::item<Src> s, void const * src) noexcept
	{
		return construct(d, dst, get(s, src));
	}

	static bool copy(meta::item<void>, void * dst, meta::item<void>, void const * src) noexcept
	{
		return true;
	}

	template <typename Dst, typename Src>
	static bool move(meta::item<Dst> d, void * dst, meta::item<Src> s, void * src) noexcept
	{
		return construct(d, dst, std::move(get(s, src)));
	}

	template <typename Dst, typename Src>
	static bool move(meta::item<Dst &> d, void * dst, meta::item<Src &> s, void * src) noexcept
	{
		return construct(d, dst, get(s, src));
	}


	static bool move(meta::item<void>, void * dst, meta::item<void>, void * src) noexcept
	{
		return true;
	}
};

template <typename... Types>
template <typename>
monadic_variant<Types...>::monadic_variant() noexcept
	: monadic_variant(in_place_index_t<0>())
{
}

template <typename... Types>
template <typename T, typename>
monadic_variant<Types...>::monadic_variant(T && t) noexcept
	: monadic_variant(in_place_type_t<meta::choose_overload_t<T, types>>(), std::forward<T>(t))
{
}

template <typename... Types>
template <typename T, typename... Args, typename>
monadic_variant<Types...>::monadic_variant(in_place_type_t<T>, Args &&... args) noexcept
	: monadic_variant(in_place_index_t<meta::index_of<T, types>::value>(), std::forward<Args>(args)...)
{
}

template <typename... Types>
template <size_t I, typename... Args, typename>
monadic_variant<Types...>::monadic_variant(in_place_index_t<I>, Args &&... args) noexcept
	: index_(I)
{
	using T = meta::sub_t<types, I>;
	if (!_monadic_variant_storage::construct(meta::item<T>(), &storage_, std::forward<Args>(args)...))
		index_ = exception_pos;
}

template <typename... Types>
monadic_variant<Types...>::monadic_variant(monadic_variant const & o) noexcept
	: index_(o.index_)
{
	meta::visit<types>(index_, [this, &o](auto m) {
		if (!_monadic_variant_storage::copy(m, &storage_, m, &o.storage_))
			index_ = exception_pos;
	});
}

template <typename... Types>
monadic_variant<Types...>::monadic_variant(monadic_variant && o) noexcept
	: index_(o.index_)
{
	meta::visit<types>(index_, [this, &o](auto m) {
		if (!_monadic_variant_storage::move(m, &storage_, m, &o.storage_))
			index_ = exception_pos;
	});
}

template <typename... Types>
monadic_variant<Types...>::~monadic_variant()
{
	meta::visit<types>(index_, [this](auto m) {
		_monadic_variant_storage::destroy(m, &storage_);
	});
}

template <typename... Types>
size_t monadic_variant<Types...>::index() const noexcept
{
	return index_;
}

template <typename T, typename... Types>
auto get(monadic_variant<Types...> const & v)
	-> typename std::enable_if<meta::contains_unique<T, meta::list<Types..., std::exception_ptr>>::value, T>::type
{
	if (v.index() != meta::index_of<T, meta::list<Types..., std::exception_ptr>>::value)
		throw bad_variant_access();

	return *reinterpret_cast<T const *>(_variant_access::storage(v));
}

template <size_t I, typename... Types>
auto get(monadic_variant<Types...> const & v)
	-> variant_alternative_t<I, monadic_variant<Types...>>
{
	using T = variant_alternative_t<I, monadic_variant<Types...>>;

	if (v.index() != I)
		throw bad_variant_access();

	return _monadic_variant_storage::get(meta::item<T>(), _variant_access::storage(v));
}

template <size_t I, typename... Types>
struct variant_alternative<I, monadic_variant<Types...>>
{
	using type = meta::sub_t<meta::list<Types..., std::exception_ptr>, I>;
};

template <size_t I, typename T>
struct variant_alternative<I, T const>
{
	using type = typename std::add_const<typename variant_alternative<I, T>::type>::type;
};

template <size_t I, typename T>
struct variant_alternative<I, T volatile>
{
	using type = typename std::add_volatile<typename variant_alternative<I, T>::type>::type;
};

template <size_t I, typename T>
struct variant_alternative<I, T const volatile>
{
	using type = typename std::add_cv<typename variant_alternative<I, T>::type>::type;
};

}
