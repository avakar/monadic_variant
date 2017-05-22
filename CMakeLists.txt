cmake_minimum_required(VERSION 3.0)
project(avakar_monadic_variant)

include(deps.cmake)

set(CMAKE_CXX_STANDARD 14)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_library(avakar_monadic_variant
	include/avakar/monadic_variant.h
	src/bad_variant_access.cpp
	src/monadic_variant_impl.h
	)
target_include_directories(avakar_monadic_variant PUBLIC include)
target_link_libraries(avakar_monadic_variant PUBLIC avakar_metapp)

add_executable(avakar_monadic_variant_test
	test/monadic_variant_test.cpp
	)
target_link_libraries(avakar_monadic_variant_test avakar_monadic_variant mutest)

source_group(include REGULAR_EXPRESSION include/.*)
source_group(src REGULAR_EXPRESSION src/.*)
source_group(test REGULAR_EXPRESSION test/.*)
