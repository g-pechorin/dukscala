#include <iostream>


#include <duktape.h>
#include <peterlavalle.diskio.hpp>

int main(int argc, char* argv[])
{
	duk_context* ctx = duk_create_heap_default();

	peterlavalle::diskio::install(ctx);

	peterlavalle::diskio::Disk::get(ctx)
		.foobar("thing");

	duk_eval_string(ctx, "peterlavalle.diskio.Disk.foobar('hamster')");

	{
		auto ref = peterlavalle::diskio::Reading::New(ctx);

		assert(nullptr != ref.operator->());

		auto n = ref->_number;

		std::cout << "?" << std::endl;
	}

	duk_destroy_heap(ctx);

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
//	Stash<NVGcontext*>() = nvgCreateGL3(NVG_STENCIL_STROKES | NVG_DEBUG);
}

peterlavalle::diskio::Disk::~Disk(void)
{
//	nvgDeleteGL3(Stash<NVGcontext*>());
}

void peterlavalle::diskio::Disk::foobar(const scad40::duk_str& text)
{
	std::cout << text << std::endl;
}

void peterlavalle::diskio::Disk::subscribe(const scad40::duk_str& path, const scad40::duk_ref<ChangeListener>& listener)
{
}

void peterlavalle::diskio::Disk::unsubscribe(const scad40::duk_str& path, const scad40::duk_ref<ChangeListener>& listener)
{
}

scad40::duk_ref<peterlavalle::diskio::Reading> peterlavalle::diskio::Disk::open(const scad40::duk_str& path)
{
	scad40::duk_ref<peterlavalle::diskio::Reading> reading = peterlavalle::diskio::Reading::New(Host());

	reading->_path = path;

	return reading;
}