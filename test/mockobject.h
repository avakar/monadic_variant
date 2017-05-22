#ifndef AVAKAR_AWAIT_TEST_MOCKOBJECT_H
#define AVAKAR_AWAIT_TEST_MOCKOBJECT_H

#include <type_traits>
#include <utility>

struct mockobject_throw_t {};
static const mockobject_throw_t mockobject_throw{};

struct mockobject_error
{
};

struct defaultthrow_mock
{
	defaultthrow_mock()
	{
		throw mockobject_error();
	}
};

inline void _mock_object_throw_if_enabled(std::true_type, int & counter)
{
	if (counter != 0 && --counter == 0)
		throw mockobject_error();
}

inline void _mock_object_throw_if_enabled(std::false_type, int & counter)
{
}

template <typename T, bool throw_on_move = false>
struct basic_mockobject
{
	basic_mockobject(mockobject_throw_t, int throw_counter)
		: throw_counter(throw_counter), value(7), counter_(nullptr)
	{
		if (throw_counter == 0)
			throw mockobject_error();
	}

	basic_mockobject(int * counter, int a = 1, int b = 2)
		: counter_(counter), value(a + b)
	{
		++*counter_;
	}

	basic_mockobject(int a = 1, int b = 2)
		: counter_(nullptr), value(a + b)
	{
	}

	basic_mockobject(basic_mockobject && o) noexcept(!throw_on_move)
		: counter_(o.counter_), value(o.value), throw_counter(o.throw_counter)
	{
		o.counter_ = nullptr;
		o.value = -1;
		o.throw_counter = 0;

		_mock_object_throw_if_enabled(std::integral_constant<bool, throw_on_move>(), throw_counter);
	}

	basic_mockobject(basic_mockobject const & o)
		: counter_(o.counter_), value(o.value), throw_counter(o.throw_counter)
	{
		if (counter_)
			++*counter_;
		if (throw_counter != 0 && --throw_counter == 0)
			throw mockobject_error();
	}

	template <typename U,
		typename = std::enable_if_t<std::is_convertible<U, T>::value>>
	basic_mockobject(basic_mockobject<U> const & o)
		: counter_(o.counter_), value(o.value), throw_counter(o.throw_counter)
	{
		if (counter_)
			++*counter_;
		if (throw_counter != 0 && --throw_counter == 0)
			throw mockobject_error();
	}

	~basic_mockobject()
	{
		if (counter_)
			--*counter_;
	}

	basic_mockobject & operator=(basic_mockobject && o) noexcept
	{
		counter_ = o.counter_;
		value = o.value;
		throw_counter = o.throw_counter;
		o.value = -1;
		o.throw_counter = 0;

		if (counter_)
			++*counter_;
		return *this;
	}

	basic_mockobject & operator=(basic_mockobject const & o)
	{
		counter_ = o.counter_;
		value = o.value;
		throw_counter = o.throw_counter;

		if (counter_)
			++*counter_;
		if (throw_counter != 0 && --throw_counter == 0)
			throw mockobject_error();
		return *this;
	}

	int * counter_;
	int value;
	int throw_counter = 0;
};

using mockobject = basic_mockobject<int>;

#endif // AVAKAR_AWAIT_TEST_MOCKOBJECT_H
