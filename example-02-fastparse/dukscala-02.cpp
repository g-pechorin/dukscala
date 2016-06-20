#include <iostream>

#include <duktape.h>

#define imperative(condition) do { if (!(condition)) { std::cerr << "The imperative `" #condition "` has failed @ " __FILE__ ":" << __LINE__ << std::endl; exit(EXIT_FAILURE); } } while (false)

int main(int argc, char* argv[])
{
	const char* fastOptJS = CMAKE_SOURCE_DIR "/target/scala-2.10/dukscala-example-02-fastopt.js";

	auto ctx = duk_create_heap_default();
	//	duk_push_object(ctx);
	//	duk_set_global_object(ctx);

	if (duk_peval_file_noresult(ctx, fastOptJS) != 0) {
		std::cerr << "evaluation of scala file failed: " << duk_safe_to_string(ctx, -1) << std::endl;
		duk_pop(ctx);
		return EXIT_FAILURE;
	}

#define HAOYI_TEXT(STRING, NUMBER) do {\
	if (DUK_ERR_NONE != duk_peval_string(ctx, "com.peterlavalle.dukscala.Haoyi()")) { \
		std::cerr << "execute function failed: " << duk_safe_to_string(ctx, -1) << std::endl; \
		duk_pop(ctx); \
		return EXIT_FAILURE; } \
	imperative(1 == duk_get_top(ctx)); \
	duk_get_prop_string(ctx, 0, "eval"); \
	duk_swap(ctx, 0, 1); \
	duk_push_string(ctx, STRING); \
	std::cout << "Calling the method with `" << STRING << "` = " << NUMBER << std::endl; \
	if (DUK_ERR_NONE != duk_pcall_method(ctx, 1)) { \
		std::cerr << "execute function failed: " << duk_safe_to_string(ctx, -1) << std::endl; \
		duk_pop(ctx); \
		return EXIT_FAILURE; } \
	imperative(1 == duk_get_top(ctx)); \
	imperative(NUMBER == duk_to_int(ctx, 0)); \
	duk_pop(ctx); \
	std::cout << "Done with the method with `" << STRING << "` = " << NUMBER << std::endl; } while (false)

	HAOYI_TEXT("2+2", 4);
	HAOYI_TEXT("7*7", 49);
	HAOYI_TEXT("0-9", -9);
	HAOYI_TEXT("3/4", 3 / 4);

	HAOYI_TEXT("6/2*(1+2)", 6 / 2 * (1 + 2));


	return EXIT_SUCCESS;
}



//"C:\Users\pal\Desktop\dukscala\example-02\target\scala-2.10\scala-js-tutorial-fastopt.js"
