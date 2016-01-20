#include <iostream>

#if 0
#include <duktape.h>
#include <peterlavalle.diskio.hpp>
#endif
#define HEREDOC(TEXT) (#TEXT)

#include <Windows.h>
#include <direct.h>

#include <assert.h>
#include <functional>

bool scan(const char* root, std::function<void(const char*)> lambda)
{
	assert(nullptr != root);

	WIN32_FIND_DATA FindFileData;
	const HANDLE hFind = FindFirstFile(root, &FindFileData);

	if (INVALID_HANDLE_VALUE == hFind)
	{
		const auto last_error = GetLastError();

		if (ERROR_FILE_NOT_FOUND == last_error)
		{
			return true;
		}

		std::cerr << "FindFirstFile failed " << last_error << std::endl;
		return false;
	}

	do
	{
		lambda(FindFileData.cFileName);
	} while (FindNextFile(hFind, &FindFileData));


	const auto last_error = GetLastError();
	FindClose(hFind);
	if (ERROR_NO_MORE_FILES == last_error)
	{
		return true;
	}

	std::cerr << "FindNextFile failed " << last_error << std::endl;
	return false;
}

bool scan(std::function<void(const char*)> lambda)
{
	char buffer[_MAX_PATH];

	return scan(strcat(_getcwd(buffer, _MAX_PATH), "\\.."), lambda);
}




int main(int argc, char* argv[])
{
	std::string text;
	scan([&](const char* name)
	{
		text = text + ">" + name + "\n";

		scan( [](const char* name)
		{
			std::cout << name << std::endl;
		});
	});
	std::cout << text.c_str() << std::endl;

	return EXIT_SUCCESS;
}

#if 0
	duk_context* ctx = duk_create_heap_default();
	peterlavalle::diskio::install(ctx);



	auto y = duk_get_top(ctx);
	assert(0 == y);

	{
		auto& g = peterlavalle::diskio::Disk::get(ctx);

		auto w = duk_get_top(ctx);
		assert(0 == w);

		g.foobar("thing");
	}

	duk_eval_string_noresult(ctx, "peterlavalle.diskio.Disk.foobar('hamster')");

	{
		auto ref = peterlavalle::diskio::Reading::New(ctx);

		assert(nullptr != ref.operator->());

		auto n = ref->_number;
	}

	{
		auto source = HEREDOC(new (function() {
			this.fileChanged = function(path)
			{
				peterlavalle.diskio.Disk.foobar('fileChanged(path = `' + path + '`)');
			}
		})());

		duk_eval_string(ctx, source);

		std::cout << "peterlavalle::diskio::ChangeListener::As(ctx, -1) = " << (peterlavalle::diskio::ChangeListener::As(ctx, -1) ? "true" : "false") << std::endl;

		auto changeListener = peterlavalle::diskio::ChangeListener::To(ctx, -1);

		auto ck = changeListener;

		changeListener->fileChanged("Somethiung");
	}

	duk_destroy_heap(ctx);

	std::cout << "MeatSOup!" << std::endl;
	return EXIT_SUCCESS;
}

peterlavalle::diskio::Reading::Reading(void) :
	_path(Host())
{
}

peterlavalle::diskio::Reading::~Reading(void)
{
}

peterlavalle::diskio::Disk::Disk(void) :
	_pwd(Host())
{
}

peterlavalle::diskio::Disk::~Disk(void)
{
}

void peterlavalle::diskio::Disk::subscribe(const scad40::duk_str& path, const scad40::duk_ptr<ChangeListener>& listener)
{
}

void peterlavalle::diskio::Disk::unsubscribe(const scad40::duk_str& path, const scad40::duk_ptr<ChangeListener>& listener)
{
}

scad40::duk_ref<peterlavalle::diskio::Reading> peterlavalle::diskio::Disk::open(const scad40::duk_str& path)
{
	scad40::duk_ref<peterlavalle::diskio::Reading> reading = peterlavalle::diskio::Reading::New(Host());

	reading->_path = path;

	return reading;
}

void peterlavalle::diskio::Disk::foobar(const scad40::duk_str& text)
{
	std::cout << text << std::endl;
}
#endif