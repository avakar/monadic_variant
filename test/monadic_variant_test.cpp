#include <avakar/monadic_variant.h>
#include <mutest/test.h>

#include "mockobject.h"

using namespace avakar;

TEST("monadic_variant<> has exception_ptr as the last type")
{
	chk monadic_variant<>::exception_pos == 0;
}

TEST("monadic_variant<int> has exception_ptr as the last type")
{
	chk monadic_variant<int>::exception_pos == 1;
}

TEST("monadic_variant<> default constructs to index 0")
{
	monadic_variant<int> v;
	chk v.index() == 0;
}

TEST("monadic_variant<int> default constructs to index 0")
{
	monadic_variant<int> v;
	chk v.index() == 0;
}

TEST("monadic_variant<int> default constructs with zero-initialized value")
{
	monadic_variant<int> v;
	chk get<int>(v) == 0;
}

TEST("monadic_variant can convert from value implicitly")
{
	monadic_variant<int> v = 1;
	chk v.index() == 0;
	chk get<0>(v) == 1;
}

TEST("monadic_variant can be constructed with explicit type")
{
	monadic_variant<int, long> v{ in_place_type_t<int>(), 2 };
	chk v.index() == 0;
	chk get<int>(v) == 2;
	chk_exc(bad_variant_access, get<long>(v));
}

TEST("monadic_variant can be constructed with explicit type")
{
	monadic_variant<int, long> v{ in_place_type_t<long>(), 2 };
	chk v.index() == 1;
	chk_exc(bad_variant_access, get<int>(v));
	chk get<long>(v) == 2;
}

TEST("monadic_variant can contain references")
{
	int var = 0;
	monadic_variant<int &> v{ in_place_type_t<int &>(), var };
	chk &get<0>(v) == &var;
}

TEST("monadic_variant can contain void")
{
	monadic_variant<void> v;
	chk v.index() == 0;
	get<0>(v);
}

TEST("monadic_variant can be constructed with explicit index")
{
	monadic_variant<int, int> v{ in_place_index_t<0>(), 2 };
	chk v.index() == 0;
	chk get<0>(v) == 2;
	chk_exc(bad_variant_access, get<1>(v));
}

TEST("monadic_variant can be constructed with explicit index")
{
	monadic_variant<int, int> v{ in_place_index_t<1>(), 2 };
	chk v.index() == 1;
	chk_exc(bad_variant_access, get<0>(v));
	chk get<1>(v) == 2;
}

TEST("get<type> throws when accessing invalid variant index")
{
	monadic_variant<int, long> v;
	chk_exc(bad_variant_access, get<long>(v));
}

TEST("monadic_variant correctly destroys contained objects")
{
	int counter = 0;

	{
		monadic_variant<mockobject> v{ in_place_index_t<0>(), &counter };
		chk counter == 1;
	}

	chk counter == 0;
}

TEST("monadic_variant contains exceptions during default construction")
{
	monadic_variant<defaultthrow_mock> v;
	chk v.index() == v.exception_pos;
	chk_exc(mockobject_error, std::rethrow_exception(get<std::exception_ptr>(v)));
}

TEST("monadic_variant catches exceptions during copy")
{
	monadic_variant<mockobject> v{ in_place_type_t<mockobject>(), mockobject_throw, 1 };
	chk v.index() == 0;

	monadic_variant<mockobject> v2 = v;
	chk v2.index() == v2.exception_pos;
}

TEST("monadic_variant<void> can be copied")
{
	monadic_variant<void> v;
	chk v.index() == 0;

	monadic_variant<void> v2 = v;
	chk v2.index() == 0;
}

TEST("monadic_variant<int &> can be copied")
{
	int var = 0;

	monadic_variant<int &> v = var;
	chk v.index() == 0;
	chk &get<0>(v) == &var;

	monadic_variant<int &> v2 = v;
	chk v2.index() == 0;
	chk &get<0>(v2) == &var;
}

TEST("monadic_variant can be moved")
{
	monadic_variant<mockobject> v;
	chk v.index() == 0;
	chk get<0>(v).value == 3;

	monadic_variant<mockobject> v2 = std::move(v);
	chk v.index() == 0;
	chk get<0>(v).value == -1;
	chk v2.index() == 0;
	chk get<0>(v2).value == 3;
}

TEST("monadic_variant<void> can be moved")
{
	monadic_variant<void> v;
	chk v.index() == 0;

	monadic_variant<void> v2 = std::move(v);
	chk v2.index() == 0;
}

TEST("monadic_variant<int &> can be moved")
{
	int var = 0;

	monadic_variant<int &> v = var;
	chk v.index() == 0;
	chk &get<0>(v) == &var;

	monadic_variant<int &> v2 = std::move(v);
	chk v2.index() == 0;
	chk &get<0>(v2) == &var;
}
