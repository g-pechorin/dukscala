#pragma once

#include <percolator.hpp>
#include "palduk.hpp"

#include <assert.h>
#include <list>

template<typename S = std::string, typename A = const char*>
struct buzzbird
{
	struct guid
	{
		const size_t _guid;

		guid(const guid&) = delete;
		guid& operator = (const guid&) = delete;

		guid(void) : _guid(reinterpret_cast<size_t>(this)) {}

		~guid(void)
		{
			assert(reinterpret_cast<size_t>(this) == _guid);
		}
	};

	struct pawn;
	struct soul : guid
	{
		buzzbird<S, A>* const _buzzbird;
	private:
		friend struct buzzbird<S, A>;
		bool _live;
	public:
		std::list<std::function<void(void)>*> _ondeds;

		soul(buzzbird<S, A>* buzzbird_) : _buzzbird(buzzbird_), _live(true)
		{}

		~soul(void)
		{
			assert(!_live);
			for (auto& onded : _ondeds)
				(*onded)();
		}

		template<bool raise_errors = true>
		pawn& attach(const char*);

		void remove(void)
		{
			assert(_live);
			_live = false;
		}
	};

	struct pawn : guid, sneak
	{
		soul& _soul;
		duk_int_t _last_result = DUK_EXEC_SUCCESS;
		S _last_message;
	private:
		friend struct buzzbird<S, A>;
		bool _live;
	public:
		std::list<std::function<void(void)>*> _ondeds;

		pawn(soul& soul_, duk_idx_t idx) : _soul(soul_), sneak(soul_._buzzbird->_ctx, idx), _last_message(""), _live(true)
		{}

		~pawn(void)
		{
			assert(!_live);

			for (auto& onded : _ondeds)
				(*onded)();
		}

		template<bool pop_args = true>
		void notify(const A name, const duk_size_t argc)
		{
			assert(argc >= 0);

			const auto top = duk_get_top(_ctx) - argc;
			// stack -> ... ; [ .. args .. ] ;
			get(static_cast<const char*>(name));
			// stack -> ... ; [ .. args .. ] ; thing() | ??? ;
			if (!duk_is_callable(_ctx, -1))
			{
				duk_pop(_ctx);
				// stack -> ... ; [ .. args .. ] ;
			}
			else
			{
				// stack -> ... ; [ .. args .. ] ; thing() ;
				push();
				// stack -> ... ; [ .. args .. ] ; thing() ; {pawn} ;
				for (duk_size_t i = 0; i < argc; ++i)
				{
					duk_dup(_ctx, top + i);
				}
				// stack -> ... ; [ .. args .. ] ; thing() ; {pawn} ; [ .. args .. ] ;
				if (DUK_EXEC_SUCCESS != (_last_result = duk_pcall_method(_ctx, argc)))
				{
					std::cerr << (_last_message = duk_to_string(_ctx, -1)) << std::endl;
				}
				// stack -> ... ; [ .. args .. ] ; result ;
				duk_pop(_ctx);
				// stack -> ... ; [ .. args .. ] ;
			}
			// stack -> ... ; [ .. args .. ] ;
			if (pop_args && argc)
			{
				duk_pop_n(_ctx, argc);
				// stack -> ... ;
			}
		}

		void detach(void)
		{
			assert(_live);
			_live = false;
		}
	};

	duk_context* const _ctx;
	sneak _behaves;
	std::list<soul> _souls;
	std::list<pawn> _pawns;

	percolator _perk;

	buzzbird(duk_context* ctx, const char* path = CMAKE_SOURCE_DIR "/src/lib/coffee-script.js") : _ctx(ctx), _behaves(ctx), _perk()
	{
		std::cout << "buzzbird(.)" << std::endl;
	}

	~buzzbird(void)
	{
		while (!(_pawns.empty() && _souls.empty()))
		{
			for (auto& pawn : _pawns)
				pawn.detach();
			for (auto& soul : _souls)
				soul.remove();
			flush();
		}
		std::cout << "~buzzbird()" << std::endl;
	}

	/// create a new soul with the passed name
	soul& create(const char*)
	{
		_souls.emplace_front(this);
		return _souls.front();
	}

	/// flush all waiting pawn::detach() and soul::remove()
	void flush(void);

	void import(const char* name_str)
	{
		static const char suffix_str[] = ".coffee";
		static const size_t suffix_len = strlen(suffix_str);

		const size_t name_len = strlen(name_str);

		const char* full = strcat((char*)memcpy(alloca(name_len + suffix_len + 1), name_str, 1 + name_len), suffix_str);

		std::string path = CMAKE_SOURCE_DIR "/src/var/";
		path += full;

		// stack -> ... ;
		duk_push_string_file_raw(_ctx, path.c_str(), 0);
		// stack -> ... ; "source;code.coffee" ;
		duk_push_string(_ctx, full);
		// stack -> ... ; "source;code.coffee" ; "source/code.coffee";
		_perk.invoke(_ctx);
		// stack -> ... ; ??? ;
		duk_pop(_ctx);
		// stack -> ... ;
	}

	template<bool pop_args = true>
	void signal(const A name, const duk_size_t argc)
	{
		assert(argc >= 0);

		// stack -> ... ; [ .. args .. ] ;
		const auto top = duk_get_top(_ctx);

		for (auto& pawn : _pawns)
		{
			assert(duk_get_top(_ctx) == top);
			pawn.notify<false>(name, argc);
			assert(duk_get_top(_ctx) == top);
		}
		assert(duk_get_top(_ctx) == top);
		// stack -> ... ; [ .. args .. ] ;

		if (pop_args && argc)
		{
			duk_pop_n(_ctx, argc);
		}
	}

	static buzzbird& grab(duk_context* ctx);

private:
	static void init(duk_context* ctx)
	{
		// stack -> ... ;
		duk_push_global_object(ctx);
		// stack -> ... ; {global} ;
		duk_push_object(ctx);
		// stack -> ... ; {global} ; {buzzbird} ;
		buzzbird<S, A>* self = new (duk_push_buffer(ctx, sizeof(buzzbird), false)) buzzbird<S, A>(ctx);
		// stack -> ... ; {global} ; {buzzbird} ; buzzbird[*] ;
		duk_put_prop_string(ctx, -2, "\xFF" "this");
		// stack -> ... ; {global} ; {buzzbird} ;
		duk_push_pointer(ctx, self);
		// stack -> ... ; {global} ; {buzzbird} ; self* ;
		duk_put_prop_string(ctx, -2, "\xFF" "self");
		// stack -> ... ; {global} ; {buzzbird} ;
		duk_push_method<buzzbird<S, A>>(ctx, -1, self, [](duk_context* ctx, buzzbird<S, A>* self) -> int
		{
			assert(&(buzzbird<S, A>::grab(ctx)) == self);

			self->~buzzbird<S, A>();

			// TODO ; remove us from global for safety

			return 0;
		}, 0);
		// stack -> ... ; {global} ; {buzzbird} ; ~buzzbird() ;
		duk_set_finalizer(ctx, -2);
		// stack -> ... ; {global} ; {buzzbird} ;
		duk_push_method<buzzbird<S, A>>(ctx, -1, self, [](duk_context* ctx, buzzbird<S, A>* self) -> int
		{
			assert(1 == duk_get_top(ctx));
			assert(duk_is_callable(ctx, 0));

			// stack -> ; class ;
			duk_dup(ctx, -1);
			// stack -> ; class ; class ;
			std::string string = duk_to_string(ctx, -1);
			// stack -> ; class ; "class" ;

			// TODO ; read this normally from the function-object
			const static std::regex pattern("function (\\w+)\\(\\) \\{/\\* ecmascript \\*/\\}");
			if (!std::regex_match(string, pattern))
			{
				PAL_FAIL_STUB("Could not determine name for behaviour");
			}

			auto bind = std::regex_replace(string, pattern, "$1");
			// stack -> ; class ; "class" ;
			duk_pop(ctx);
			// stack -> ; class ;

			if ("_Class" == bind)
			{
				// stack -> ; class ;
				duk_get_prop_string(ctx, -1, "fileName");
				// stack -> ; class ; "file/name.coffee" ;

				assert(duk_is_string(ctx, -1));
				string = duk_to_string(ctx, -1);

				const static std::regex filename("^.*?([^/]+)\\.coffee$");
				assert(std::regex_match(string, filename));
				bind = std::regex_replace(string, filename, "$1");
				duk_pop(ctx);
				// stack -> ; class ;
			}

			// stack -> ; class ;
			self->_behaves.get(bind.c_str());
			// stack -> ; class ; old-binding ;
			if (!duk_is_null_or_undefined(ctx, 1))
			{
				// TODO ; use fileName as a stupidity-test to make sure we're not overwriting ourself
				std::cerr << "You'll need to hot-swap the things" << std::endl;
			}
			// stack -> ; class ; old-binding ;
			duk_pop(ctx);
			// stack -> ; class ;
			self->_behaves.set(bind.c_str());
			// stack -> ;

#ifdef _DEBUG
			self->_behaves.get(bind.c_str());
			assert(duk_is_callable(ctx, -1));
			duk_pop(ctx);
			std::cout << "Behaviour `" << bind.c_str() << "` loaded" << std::endl;
#endif

			return 0;
		}, 1);
		// stack -> ... ; {global} ; {buzzbird} ; behave(type) ;
		duk_put_prop_string(ctx, -3, "behave");
		// stack -> ... ; {global} ; {buzzbird} ;
		duk_push_method<buzzbird<S, A>>(ctx, -1, self, [](duk_context* ctx, buzzbird<S, A>* self) -> int
		{
			// stack -> ; "name" ;
			buzzbird<S, A>::soul* soul = &(self->create(duk_to_string(ctx, 0)));

			// stack -> ; "name" ;
			duk_push_object(ctx);
			// stack -> ; "name" ; {self} ;
			duk_remove(ctx, -2);

			// stack -> ; {self} ;
			duk_push_method<buzzbird<S, A>::soul>(ctx, -1, soul, [](duk_context* ctx, buzzbird<S, A>::soul* soul) -> int
			{
				buzzbird<S, A>* self = soul->_buzzbird;

				// stack -> ; class | "class" ;
				if (!duk_is_string(ctx, 0))
				{
					duk_error(ctx, 23, "I need a behaviour 'name' for now (sorry)");
				}
				// stack -> ; "class" ;
				soul->attach(duk_to_string(ctx, 0));

				// TODO ; stack trace here

				// TODO ; return an interface or something
				return 0;
			}, 1);
			// stack -> ; {self} ; attach(class) ;
			duk_put_prop_string(ctx, -2, "attach");


			duk_push_method<buzzbird<S, A>::soul>(ctx, -1, soul, [](duk_context* ctx, buzzbird<S, A>::soul* soul) -> int
			{
				std::cout << "???" << std::endl;
				return 0;
			}, 0);
			// stack -> ; {self} ; remove() ;
			duk_put_prop_string(ctx, -2, "remove");
			// stack -> ; {self} ;

			return 1;
		}, 1);
		// stack -> ... ; {global} ; {buzzbird} ; create(name) ;
		duk_put_prop_string(ctx, -3, "create");
		// stack -> ... ; {global} ; {buzzbird} ;
		duk_put_prop_string(ctx, -2, "\xFF" "buzzbird");
		// stack -> ... ; {global} ;
		duk_pop(ctx);
		// stack -> ... ;
	}
};

#include <regex>

template<typename S, typename A>
inline void buzzbird<S, A>::flush(void)
{
recur:
	for (auto pawnIt = _pawns.begin(); pawnIt != _pawns.end(); ++pawnIt)
	{
		if (pawnIt->_live)
			continue;

		pawnIt->notify("onDetach", 0);

		PAL_TODO_STUB("Clear out whatnot from the stash");

		_pawns.erase(pawnIt);

		goto recur;
	}

	for (auto soulIt = _souls.begin(); soulIt != _souls.end(); ++soulIt)
	{
		if (soulIt->_live)
			continue;

		bool foundOne = false;
		for (auto pawnIt = _pawns.begin(); pawnIt != _pawns.end(); ++pawnIt)
		{
			if (&(*soulIt) != &(pawnIt->_soul))
				continue;

			pawnIt->detach();

			foundOne = true;
		}

		if (foundOne)
		{
			goto recur;
		}

		_souls.erase(soulIt);

		goto recur;
	}
}

template<typename S, typename A>
inline typename buzzbird<S, A>& buzzbird<S, A>::grab(duk_context* ctx)
{
	// we're going to grab the exising context-wide object and return it
	// ... unless we find that there's no context-wide object ; in which case we'll create one

	// stack -> ... ;
	duk_push_global_object(ctx);
	// stack -> ... ; {global} ;
	duk_get_prop_string(ctx, -1, "\xFF" "buzzbird");
	// stack -> ... ; {global} ; null | {buzzbird} ;
	if (duk_is_null_or_undefined(ctx, -1))
	{
		// stack -> ... ; {global} ; null ;
		duk_pop_2(ctx);
		// stack -> ... ;
		buzzbird<S, A>::init(ctx);
		// stack -> ... ;
		return buzzbird<S, A>::grab(ctx);
	}
	// stack -> ... ; {global} ; {buzzbird} ;
	duk_get_prop_string(ctx, -1, "\xFF" "self");
	// stack -> ... ; {global} ; {buzzbird} ; buzzbird[*] ;
	auto ptr = reinterpret_cast<buzzbird<S, A>*>(duk_to_pointer(ctx, -1));
	assert(ptr);
	duk_pop_3(ctx);
	// stack -> ... ;
	return *ptr;
}

template<typename S, typename A>
template<bool raise_errors>
inline typename buzzbird<S, A>::pawn& buzzbird<S, A>::soul::attach(const char* type)
{
	// stack -> ... ;
	_buzzbird->_behaves.get(type);
	// stack -> ... ; behave() | undef ;
	if (!duk_is_callable(_buzzbird->_ctx, -1))
	{
		// stack -> ... ; undef ;
		duk_pop(_buzzbird->_ctx);
		// stack -> ... ;

		if (raise_errors)
		{
			duk_error(_buzzbird->_ctx, 456, "No behaviour `%s`", type);
		}

		return *reinterpret_cast<buzzbird<S, A>::pawn*>(nullptr);
	}
	// stack -> ... ; behave() ;
	duk_new(_buzzbird->_ctx, 0);
	// stack -> ... ; {pawn} ;
	_buzzbird->_pawns.emplace_front(*this, -1);
	auto& pawn = _buzzbird->_pawns.front();
	// stack -> ... ; {pawn} ;
	duk_push_string(_buzzbird->_ctx, type);
	// stack -> ... ; {pawn} ; "type / class / whatev" ;
	duk_put_prop_string(_buzzbird->_ctx, -2, "\xFF" "type");
	// stack -> ... ; {pawn} ;
	duk_push_pointer(_buzzbird->_ctx, &(_buzzbird->_pawns.front()));
	// stack -> ... ; {pawn} ; pawn* ;
	duk_put_prop_string(_buzzbird->_ctx, -2, "\xFF" "pawn");
	// stack -> ; class ; {pawn} ;

	// TODO : set the owner

	duk_pop(_buzzbird->_ctx);
	// stack -> ... ;
	pawn.notify("onAttach", 0);

	return pawn;
}
