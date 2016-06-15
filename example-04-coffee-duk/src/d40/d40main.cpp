
#include <iostream>
#include <sstream>
struct ThreeOut
{
	std::ostream& out;
	std::ostream& err;
	std::ostream& log;
	void(*end)(void);
};

#include <buzzbird.hpp>
#include <d40-duk.hpp>




int main(int argc, char* argv[])
{
	std::ostringstream log;
	ThreeOut threeOut = {
		std::cout,
		std::cout,
		log,
		[](void) { exit(EXIT_FAILURE); }
	};

	std::cout << "Well hello Mr Fancy Pants" << std::endl;
	auto ctx = duk_create_heap_default();

	underlay::install(ctx);

	underlay::Log::get(ctx)._ioc = &threeOut;

	auto& buzz = buzzbird<>::grab(ctx);

	buzz._perk.invoke_file(ctx, CMAKE_SOURCE_DIR "/src/var/Buzard.coffee");

	auto& dude = buzz.create("flobbidy");
	auto& some = dude.attach("Buzard");

	some.detach();
	buzz.flush();

	dude.remove();
	buzz.flush();

	buzz.flush();

	duk_destroy_heap(ctx);
	std::cout << "Groovy" << std::endl;
	return EXIT_FAILURE;
}

underlay::Log::Log(void)
{}
underlay::Log::~Log(void)
{}

void underlay::Log::warn(scad40::duk_string& message)
{
	_ioc->err << "[WARN] " << message << std::endl;
	_ioc->log << "[WARN] " << message << std::endl;
}

void underlay::Log::info(scad40::duk_string& message)
{
	_ioc->out << "[INFO] " << message << std::endl;
}

void underlay::Log::fail(scad40::duk_string& message)
{
	_ioc->err << "[FAIL] " << message << std::endl;
	_ioc->log << "[FAIL] " << message << std::endl;
	_ioc->end();
}

void underlay::Log::note(scad40::duk_string& message)
{
	_ioc->out << "[NOTE] " << message << std::endl;
	_ioc->log << "[NOTE] " << message << std::endl;
}



underlay::Pawn::Pawn()
{
	_ioc = nullptr;
	_onDed = [this](void)
	{
		_ioc = nullptr;
	};
	_ioc->_ondeds.emplace_front(&(_onDed));
}

underlay::Pawn::~Pawn()
{
	assert(_ioc);

	_ioc->_ondeds.remove(&(_onDed));
}

void underlay::Pawn::detach()
{
	assert(_ioc);

	_ioc->detach();
	_ioc = nullptr;
}

void underlay::Pawn::notify(scad40::duk_string& message)
{
	assert(_ioc);

	_ioc->notify(message.c_str(), 0);
}



underlay::Soul::Soul()
{
	_ioc = nullptr;
	_onDed = [this](void)
	{
		_ioc = nullptr;
	};
	_ioc->_ondeds.emplace_front(&(_onDed));
}

underlay::Soul::~Soul()
{
	assert(_ioc);

	_ioc->_ondeds.remove(&(_onDed));
}

void underlay::Soul::remove()
{
	assert(_ioc);

	_ioc->remove();
	_ioc = nullptr;
}



underlay::Owner::Owner(void)
{}

underlay::Owner::~Owner(void)
{}

scad40::duk_native<underlay::Soul> underlay::Owner::ofPawn(scad40::duk_native<underlay::Pawn>& pawn)
{
	auto soulWrapper = underlay::Soul::New(Host());

	soulWrapper->_ioc = &(pawn->_ioc->_soul);

	return soulWrapper;
}