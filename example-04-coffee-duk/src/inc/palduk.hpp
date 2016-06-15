#pragma once

#include "duktape.h"

/// arkane wrapped function pusher that stuffs some "self" object into a wrapped call.
template<typename T>
inline void duk_push_method(duk_context* ctx, duk_idx_t object, T* userdata, int(*callback)(duk_context* ctx, T* userdata), duk_idx_t nargs)
{
	object = duk_normalize_index(ctx, object);

	// stack -> ... ;
	duk_push_c_function(ctx, [](duk_context *ctx)
	{
		// stack -> ... ;
		duk_push_current_function(ctx);
		// stack -> ... ; function() ;
		duk_get_prop_string(ctx, -1, "\xFF" "userdata");
		// stack -> ... ; function() ; userdata* ;
		T* userdata = reinterpret_cast<T*>(duk_to_pointer(ctx, -1));
		// stack -> ... ; function() ; userdata* ;
		duk_pop(ctx);
		// stack -> ... ; function() ;
		duk_get_prop_string(ctx, -1, "\xFF" "callback");
		// stack -> ... ; function() ; callback* ;
		auto callback = reinterpret_cast<int(*)(duk_context* ctx, T* userdata)>(duk_to_pointer(ctx, -1));
		// stack -> ... ; function() ; callback* ;
		duk_pop(ctx);
		// stack -> ... ; function() ;
		duk_pop(ctx);
		// stack -> ... ;
		return callback(ctx, userdata);
	}, nargs);
	// stack -> ... ; wrapper() ;
	duk_dup(ctx, object);
	// stack -> ... ; wrapper() ; {object} ;
	duk_put_prop_string(ctx, -2, "\xFF" "object");
	// stack -> ... ; wrapper() ;
	duk_push_pointer(ctx, userdata);
	// stack -> ... ; wrapper() ; userdata* ;
	duk_put_prop_string(ctx, -2, "\xFF" "userdata");
	// stack -> ... ; wrapper() ;
	duk_push_pointer(ctx, callback);
	// stack -> ... ; wrapper() ; callback* ;
	duk_put_prop_string(ctx, -2, "\xFF" "callback");
	// stack -> ... ; wrapper() ;
}

/// lets me use a pointer as a string
template<char prefix>
class pointy_key
{
	char _text[1 + sizeof(void*) * 8 + 1];
public:
	template<typename T = void>
	pointy_key(T* ptr)
	{
		auto str = reinterpret_cast<char*>(memset(_text, 0x00000000, sizeof(_text)));

		uint8_t* raw = reinterpret_cast<uint8_t*>(&(ptr));
		str[0] = prefix;
		for (auto i = 0; i < sizeof(void*); ++i)
		{
			uint8_t p = raw[i];
			for (auto j = 0; j < 8; ++j)
			{
				str[1 + (i * 8) + j] = 'a' + (p & 0xF);
				p = p >> 4;
			}
		}
	}

	template<typename T = void>
	pointy_key& operator=(T* ptr) { return *new (this)pointy_key<prefix>(ptr); }

	operator const char*(void) const { return _text; }
};

/// hides a single value
class stash
{
	duk_context* _ctx;
public:
	stash(const stash&) = delete;
	stash& operator= (const stash&) = delete;

	stash(duk_context* ctx, duk_idx_t idx) : _ctx(ctx)
	{
		idx = duk_normalize_index(_ctx, idx);

		// stack -> ... ;
		duk_push_global_stash(_ctx);
		// stack -> ... ; {stash} ;
		duk_push_pointer(_ctx, this);
		// stack -> ... ; {stash} ; this* ;
		duk_dup(_ctx, idx);
		// stack -> ... ; {stash} ; this* ; value ;
		duk_put_prop(_ctx, -3);
		// stack -> ... ; {stash} ;
		duk_pop(_ctx);
		// stack -> ... ;
	}

	~stash(void)
	{
		// stack -> ... ;
		duk_push_global_stash(_ctx);
		// stack -> ... ; {stash} ;
		duk_push_pointer(_ctx, this);
		// stack -> ... ; {stash} ; this* ;
		duk_del_prop(_ctx, -2);
		// stack -> ... ; {stash} ;
		duk_pop(_ctx);
		// stack -> ... ;
	}

	void push(void)
	{
		// stack -> ... ;
		duk_push_global_stash(_ctx);
		// stack -> ... ; {stash} ;
		duk_push_pointer(_ctx, this);
		// stack -> ... ; {stash} ; this* ;
		duk_get_prop(_ctx, -2);
		// stack -> ... ; {stash} ; value ;
		duk_remove(_ctx, -2);
		// stack -> ... ; value ;
	}
};

/// hides some values
struct sneak
{
	duk_context* _ctx;

	sneak(const sneak&) = delete;
	sneak& operator= (const sneak&) = delete;

	sneak(duk_context* ctx, duk_idx_t idx) : _ctx(ctx)
	{
		// stack -> ... ;
		duk_dup(_ctx, idx);
		// stack -> ... ; {self} ;
		duk_push_global_stash(_ctx);
		// stack -> ... ; {self} ; {stash} ;
		duk_swap_top(_ctx, -2);
		// stack -> ... ; {stash} ; {self} ;
		duk_put_prop_string(_ctx, -2, pointy_key<'\xFF'>(this));
		// stack -> ... ; {stash} ;
		duk_pop(_ctx);
	}

	sneak(duk_context* ctx) : _ctx(ctx)
	{
		// stack -> ... ;
		duk_push_global_stash(_ctx);
		// stack -> ... ; {stash} ;
		duk_push_object(_ctx);
		// stack -> ... ; {stash} ; {sneak} ;
		duk_put_prop_string(_ctx, -2, pointy_key<'\xFF'>(this));
		// stack -> ... ; {stash} ;
		duk_pop(_ctx);
	}

	void push(void)
	{
		// stack -> ... ;
		duk_push_global_stash(_ctx);
		// stack -> ... ; {stash} ;
		duk_get_prop_string(_ctx, -1, pointy_key<'\xFF'>(this));
		// stack -> ... ; {stash} ; {sneak} ;
		duk_remove(_ctx, -2);
		// stack -> ... ; {sneak} ;
	}

	void set(const char* key)
	{
		// stack -> ... ; value ;
		push();
		// stack -> ... ; value ; {sneak} ;
		duk_swap_top(_ctx, -2);
		// stack -> ... ; {sneak} ; value ;
		duk_put_prop_string(_ctx, -2, key);
		// stack -> ... ; {sneak} ;
		duk_pop(_ctx);
		// stack -> ... ;
	}

	void get(const char* key)
	{
		// stack -> ... ;
		duk_push_string(_ctx, key);
		// stack -> ... ; "key" ;
		push();
		// stack -> ... ; "key" ; {sneak} ;
		duk_swap_top(_ctx, -2);
		// stack -> ... ; {sneak} ; "key" ;
		duk_get_prop(_ctx, -2);
		// stack -> ... ; {sneak} ; value ;
		duk_remove(_ctx, -2);
		// stack -> ... ; value ;
	}
};
