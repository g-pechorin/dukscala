#include <iostream>


#include <duktape.h>
#include <peterlavalle.diskio.hpp>

int main(int argc, char* argv[])
{
	duk_context* ctx = duk_create_heap_default();

	duk_eval_string(ctx, "(function(o) { o.single = 3.14; return o.single; })");
	duk_push_object(ctx);
	duk_push_string(ctx, "single");
	duk_push_c_function(ctx, [](duk_context* ctx) -> duk_ret_t
	{
		// getter(key, ???, ???)
		auto r0 = duk_to_string(ctx, 0);
		auto r1 = duk_to_string(ctx, 1);
		auto r2 = duk_to_string(ctx, 2);

		duk_push_this(ctx);
		auto my = duk_to_string(ctx, 3);

		duk_push_string(ctx, "fella");

		return 1;
	}, 3);
	duk_push_c_function(ctx, [](duk_context* ctx) -> duk_ret_t
	{
		// setter(new val, key, ???)
		auto r0 = duk_to_string(ctx, 0);
		auto r1 = duk_to_string(ctx, 1);
		auto r2 = duk_to_string(ctx, 2);

		duk_push_this(ctx);
		auto my = duk_to_string(ctx, 3);

		assert(std::string("3.14") == r0);

		return 0;
	}, 3);
	duk_def_prop(ctx, -4, DUK_DEFPROP_HAVE_GETTER | DUK_DEFPROP_HAVE_SETTER | DUK_DEFPROP_HAVE_ENUMERABLE);
	duk_call(ctx, 1);
	assert(std::string("fella") == duk_to_string(ctx, 0));
	duk_pop(ctx);

#if 0
	size_t f = (size_t)duk_push_fixed_buffer(ctx, 4);


	size_t r = (size_t)duk_to_buffer(ctx, -1, nullptr);
	duk_pop(ctx);

	duk_push_object(ctx);


	duk_push_c_function(ctx, [](duk_context* ctx) -> duk_ret_t
	{
		std::cout << "Hello World" << std::endl;

		return -1;

	}, 3);
	duk_put_prop_string(ctx, 0, "\xFF" "[[PutValue]]");


	duk_push_string(ctx, "foo");
	duk_put_prop_string(ctx, 0, "bar");
#endif



	peterlavalle::diskio::install(ctx);


	std::cout << "Hello World" << std::endl;
	return EXIT_SUCCESS;
	}

peterlavalle::diskio::Disk::Disk(void) :
_pwd(Host())
{

}