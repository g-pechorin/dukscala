#include <iostream>

#include "liebeck.hpp"

#include <buzzbird.hpp>

int main(int argc, char* argv[])
{
	auto ctx = duk_create_heap_default();

	buzzbird<>::grab(ctx);

	///
	// Round 1 ; evaluate some JS code to see if it spits out expected gibberish
	{
		duk_eval_string(ctx, "print('heya')");

		std::cout << "Foo >>>" << std::endl;
		if (0 != duk_peval_string(ctx, "Foo = (function() {\n  function Foo() {}\n\n\n  return Foo;\n\n})();\n; behave(Foo)")) {
			std::cerr << "eval failed: " << duk_safe_to_string(ctx, -1) << std::endl;
			exit(EXIT_FAILURE);
		}
		std::cout << "Foo <<<\n" << std::endl;

		std::cout << "Bar >>>" << std::endl;
		if (0 != duk_peval_string(ctx, "function Bar() {}; behave(Bar)")) {
			std::cerr << "eval failed: " << duk_safe_to_string(ctx, -1) << std::endl;
			exit(EXIT_FAILURE);
		}
		std::cout << "Bar <<<\n" << std::endl;


		duk_pop_n(ctx, duk_get_top(ctx));
	}


	// Round 2 ; load script
	{
		auto& duck = buzzbird<>::grab(ctx);
		const char* path = CMAKE_SOURCE_DIR "/src/var/Calz.js";

		assert(0 == duk_get_top(ctx));
		duk_compile_file(ctx, DUK_COMPILE_EVAL, path);
		assert(1 == duk_get_top(ctx));
		assert(duk_is_callable(ctx, 0));

		if (0 != duk_pcall(ctx, 0)) {
			std::cerr << "eval failed: " << duk_safe_to_string(ctx, -1) << std::endl;
			exit(EXIT_FAILURE);
		}
		else {

			assert(1 == duk_get_top(ctx));
			duck._behaves.get("Calz");
			assert(2 == duk_get_top(ctx));
			assert(duk_is_callable(ctx, 1));

			duk_pop_2(ctx);
		}
	}

	// Round 3 ; check that noaded script does what I think it should
	std::cout << ">> Round 3 >>" << std::endl;
	{
		auto& duck = buzzbird<>::grab(ctx);
		assert(0 == duk_get_top(ctx));
		duck._behaves.get("Calz");
		assert(1 == duk_get_top(ctx));
		assert(duk_is_callable(ctx, 0));

		duk_new(ctx, 0);
		assert(1 == duk_get_top(ctx));

		duk_enum(ctx, -1, DUK_ENUM_INCLUDE_NONENUMERABLE);

		while (duk_next(ctx, -1 /*enum_index*/, 0 /*get_value*/)) {
			/* [ ... enum key ] */
			printf("-> key %s\n", duk_get_string(ctx, -1));
			duk_pop(ctx);  /* pop_key */
		}

		duk_pop_2(ctx);  /* pop enum object and real object */


	}
	std::cout << "<< Round 3 <<\n" << std::endl;

	// Round 4 ; create a soul and attach Calz
	std::cout << ">> Round 4 >>" << std::endl;
	{
		auto& duck = buzzbird<>::grab(ctx);
		assert(0 == duk_get_top(ctx));

		auto& soul = duck.create("fooBar");
		auto& pawn = soul.attach("Calz");

		assert(0 == duk_get_top(ctx));

		std::cout << "buzzbird<>::grab(ctx).signal('move') >>>" << std::endl;
		buzzbird<>::grab(ctx).signal("move", 0);
		std::cout << "buzzbird<>::grab(ctx).signal('move') <<<" << std::endl;
	}
	std::cout << "<< Round 4 <<\n" << std::endl;

	// Round 5 ; loadup pre-compiled a CoffeeScript one and do that
	std::cout << ">> Round 5 >>" << std::endl;
	{
		assert(0 == duk_get_top(ctx));
		duk_compile_file(ctx, DUK_COMPILE_EVAL, CMAKE_SOURCE_DIR "/src/var/Animal.js");
		assert(1 == duk_get_top(ctx));
		assert(duk_is_callable(ctx, 0));

		if (0 != duk_pcall(ctx, 0)){
			std::cerr << "eval failed: " << duk_safe_to_string(ctx, -1) << std::endl;
			exit(EXIT_FAILURE);
		}
		duk_pop(ctx);
		assert(0 == duk_get_top(ctx));

		auto& soul = buzzbird<>::grab(ctx).create("good-boy");
		auto& pawn = soul.attach("Animal");

		assert(0 == duk_get_top(ctx));

		std::cout << "buzzbird<>::grab(ctx).signal('move', 3.14) >>>" << std::endl;
		duk_push_number(ctx, 3.14);
		buzzbird<>::grab(ctx).signal("move", 1);
		std::cout << "buzzbird<>::grab(ctx).signal('move', 3.14) <<<" << std::endl;
	}
	std::cout << "<< Round 5 <<\n" << std::endl;

	// Round 6 ; compile CoffeeScript JIT
	std::cout << ">> Round 6 >>" << std::endl;
	assert(0 == duk_get_top(ctx));
	{
		auto& dude = buzzbird<>::grab(ctx);

		dude.import("Cherished");

		auto& soul = buzzbird<>::grab(ctx).create("this-boy");
		auto& pawn = soul.attach("Cherished");

		std::cout << "buzzbird<>::grab(ctx).signal('bump', 3.14) >>>" << std::endl;
		duk_push_number(ctx, 3.14);
		buzzbird<>::grab(ctx).signal("bump", 1);
		std::cout << "buzzbird<>::grab(ctx).signal('bump', 3.14) <<<" << std::endl;
	}
	assert(0 == duk_get_top(ctx));
	std::cout << "<< Round 6 <<\n" << std::endl;

	// Round 7 ; compile unamed CoffeeScript JIT
	std::cout << ">> Round 7 >>" << std::endl;
	assert(0 == duk_get_top(ctx));
	{
		auto& dude = buzzbird<>::grab(ctx);

		dude.import("Beloved");

		auto& soul = dude.create("last-up");
		auto& pawn = soul.attach("Beloved");

		std::cout << "buzzbird<>::grab(ctx).signal('done', 3.14, '1983') >>>" << std::endl;
		duk_push_number(ctx, 3.14);
		duk_push_string(ctx, "1983");
		buzzbird<>::grab(ctx).signal("done", 2);
		std::cout << "buzzbird<>::grab(ctx).signal('done', 3.14, '1983') <<<" << std::endl;
	}
	assert(0 == duk_get_top(ctx));
	std::cout << "<< Round 7 <<\n" << std::endl;



	duk_destroy_heap(ctx);
	std::cout << "Quack Quack .." << std::endl;

	return EXIT_SUCCESS;
}
