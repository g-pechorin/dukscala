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

	duk_destroy_heap(ctx);

	return EXIT_SUCCESS;
}

peterlavalle::diskio::Disk::Disk(void) :
	_pwd(Host())
{

}

void peterlavalle::diskio::Disk::foobar(const scad40::duk_str& text)
{
	std::cout << text << std::endl;
}