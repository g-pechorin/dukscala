#include <iostream>

#include "gtest/gtest.h"

TEST(scad40Test, a_thing)
{
	EXPECT_EQ(4, (2 + 2)) << "Obvious";
}

#include "duktape.h"

TEST(scad40Test, duk_context)
{
	duk_context* ctx = duk_create_heap_default();

	ASSERT_TRUE(nullptr != ctx) << "Couldn't create Duktape";

	duk_destroy_heap(ctx);
}

#include "D40.hpp"

TEST(scad40Test, scad40)
{
	duk_context* ctx = duk_create_heap_default();

	ASSERT_TRUE(nullptr != ctx) << "Couldn't create Duktape";

	peterlavalle::diskio::install(ctx);

	duk_destroy_heap(ctx);
}
