#pragma once

#include "duktape.h"

#include <functional>

#include <assert.h>


#define DUK_DO_ELSE(call,args) if (DUK_EXEC_SUCCESS != (call args))

#ifdef _DEBUG
#include <iostream>

#define PAL_FAIL_STUB(MESSAGE) do { assert(false); std::cerr << MESSAGE << "\n\t@" << __LINE__ << "\n\t" __FILE__ << std::endl; exit(EXIT_FAILURE); } while(false)
#define PAL_TODO_STUB(MESSAGE) do { static bool seen = false; if (!seen) { seen = true; std::cerr << MESSAGE << "\n\t@" << __LINE__ << "\n\t" __FILE__ << std::endl; } } while(false)

#else
dude - errors!
#endif

class percolator
{
	duk_context* _ctx;
public:
	percolator(const char* path = CMAKE_SOURCE_DIR "/src/lib/coffee-script.js")
	{
		_ctx = duk_create_heap_default();

		// stack -> ;
		duk_eval_file(_ctx, path);
		// stack -> ; ??? ;
		duk_pop(_ctx);
		// stack -> ;
		if (0 != duk_peval_string(_ctx, "({percolator: function (source) { return CoffeeScript.compile(source, { bare: true }); }})"))
		{
			PAL_FAIL_STUB("eval failed: " << duk_safe_to_string(_ctx, -1));
		}
		assert(1 == duk_get_top(_ctx));
		// stack -> ; {new-global} ;
		duk_set_global_object(_ctx);
		// stack -> ;
	}

	~percolator(void)
	{
		assert(_ctx);
		duk_destroy_heap(_ctx);
		_ctx = nullptr;
	}

	void convert(const char* source, std::function<void(const char* result)> callback)
	{
		assert(0 == duk_get_top(_ctx));

		// stack -> ;
		duk_get_global_string(_ctx, "percolator");
		// stack -> ; percolator() ;
		duk_push_string(_ctx, source);
		// stack -> ; percolator() ; "source.coffee" ;
		duk_call(_ctx, 1);
		// stack -> ; "source.js" ;
		callback(duk_to_string(_ctx, 0));

		duk_pop(_ctx);

		assert(0 == duk_get_top(_ctx));
	}

	void invoke(duk_context *ctx)
	{
		assert(2 <= duk_get_top(ctx));
		assert(duk_is_string(ctx, -2));
		assert(duk_is_string(ctx, -1));

		// stack -> ... ; "source;code.coffee" ; "source/code.coffee";
		convert(duk_to_string(ctx, -2), [=](const char* src)
		{
			// stack -> ... ; "source;code.coffee" ; "source/code.coffee";
			duk_compile_string_filename(ctx, 0, src);
			// stack -> ... ; "source;code.coffee" ; function() ;
			DUK_DO_ELSE(duk_pcall, (ctx, 0))
			{
				PAL_FAIL_STUB("???");
			}
			// stack -> ... ; "source;code.coffee" ; ??? ;
			duk_remove(ctx, -2);
			// stack -> ... ;
		});
		// stack -> ... ; ??? ;
	}

	void invoke_file(duk_context *ctx, const char* path)
	{
		duk_push_string_file_raw(ctx, path, 0);
		duk_push_string(ctx, path);
		invoke(ctx);
	}
};