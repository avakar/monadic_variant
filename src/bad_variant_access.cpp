#include <avakar/monadic_variant.h>
using namespace avakar;

char const * bad_variant_access::what() const noexcept
{
	return "bad_variant_access";
}
