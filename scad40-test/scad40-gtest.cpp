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

peterlavalle::diskio::Disk::Disk(void)
{
}

scad40::duk_native<peterlavalle::diskio::Reading> peterlavalle::diskio::Disk::open(scad40::duk_string& path)
{
	auto result = peterlavalle::diskio::Reading::New(Host());

	result->_path = path;

	return result;
}

peterlavalle::diskio::Disk::~Disk(void)
{
}


peterlavalle::diskio::Reading::Reading(void) :
	_path(Host(), ""),
	_handle(nullptr)
{
}

int8_t peterlavalle::diskio::Reading::read()
{
	EXPECT_FALSE(true) << "STUB ; method incomplete";
	return 0;
}
void peterlavalle::diskio::Reading::close()
{
	EXPECT_FALSE(true) << "STUB ; method incomplete";
}
bool peterlavalle::diskio::Reading::endOfFile()
{
	EXPECT_FALSE(true) << "STUB ; method incomplete";
	return true;
}

peterlavalle::diskio::Reading::~Reading(void)
{
	EXPECT_FALSE(true) << "STUB ; method incomplete";
}

TEST(scad40Test, scad40)
{
	duk_context* ctx = duk_create_heap_default();

	ASSERT_TRUE(nullptr != ctx) << "Couldn't create Duktape";

	peterlavalle::diskio::install(ctx);

	duk_destroy_heap(ctx);
}
