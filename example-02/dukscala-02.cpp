#include <iostream>

#include <duktape.h>

#define imperative(condition) do { if (!(condition)) { std::cerr << "The imperative `" #condition "` has failed @ " __FILE__ ":" << __LINE__ << std::endl; exit(EXIT_FAILURE); } } while (false)

int main(int argc, char* argv[])
{
	const char* fastOptJS = CMAKE_SOURCE_DIR "/target/scala-2.10/scala-js-tutorial-fastopt.js";

	auto ctx = duk_create_heap_default();
	//	duk_push_object(ctx);
	//	duk_set_global_object(ctx);

	if (duk_peval_file_noresult(ctx, fastOptJS) != 0) {
		std::cerr << "evaluation of scala file failed: " << duk_safe_to_string(ctx, -1) << std::endl;
		duk_pop(ctx);
		return EXIT_FAILURE;
	}

	//
	// run the whole thing in JS at once
	if (duk_peval_string(ctx, "com.peterlavalle.dukscala.DumbCompiler().doThing('3+2')") != 0) {
		std::cerr << "execute function failed: " << duk_safe_to_string(ctx, -1) << std::endl;
		duk_pop(ctx);
		return EXIT_FAILURE;
	}
	imperative(1 == duk_get_top(ctx));
	imperative(duk_is_number(ctx, 0));
	std::cout << "It is `" << duk_to_number(ctx, 0) << "`" << std::endl;


	//
	// run part of the thing in JS
	if (duk_peval_string(ctx, "com.peterlavalle.dukscala.DumbCompiler().doThing") != 0) {
		std::cerr << "lookup function failed: " << duk_safe_to_string(ctx, -1) << std::endl;
		duk_pop(ctx);
		return EXIT_FAILURE;
	}

	imperative(1 == duk_get_top(ctx));
	imperative(duk_is_callable(ctx, 0));



	duk_push_string(ctx, "2 + 2");
	imperative(0 != duk_pcall(ctx, 1));

	imperative(1 == duk_get_top(ctx));

	std::cout << ">" << duk_get_type(ctx, 0) << std::endl;
	imperative(duk_is_number(ctx, 0));

	std::cout << "It is `" << duk_to_number(ctx, 0) << "`" << std::endl;


	return EXIT_SUCCESS;
}



//"C:\Users\pal\Desktop\dukscala\example-02\target\scala-2.10\scala-js-tutorial-fastopt.js"