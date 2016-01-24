
//
// this file demonstrates implementing the declared (but not defined) methods of ScaD40


#include <peterlavalle.diskio.hpp>

#include "peterlavalle.diskio-stupid_mock.hpp"

peterlavalle::diskio::Reading::Reading(void) :
	_path(Host())
{
	_path = "my/path.spot";
	_number = 1983.3f;
	stupid_mock::get(Host()).soft_call(__FUNCTION__);
}

void peterlavalle::diskio::Reading::close(void)
{
	std::stringstream log;
	log << __FUNCTION__ << "(" << ")";

	stupid_mock::get(Host()).soft_call(log);
}

int8_t peterlavalle::diskio::Reading::read(void)
{
	std::stringstream log;
	log << __FUNCTION__ << "(" << ")";

	stupid_mock::get(Host()).soft_call(log);

	return 14;
}

bool peterlavalle::diskio::Reading::endOfFile(void)
{
	std::stringstream log;
	log << __FUNCTION__ << "(" << ")";

	stupid_mock::get(Host()).soft_call(log);

	return true;
}

peterlavalle::diskio::Disk::Disk(void) :
	_pwd(Host())
{
	std::stringstream log;
	log << __FUNCTION__ << "(" << ")";

	stupid_mock::get(Host()).soft_call(log);
}

peterlavalle::diskio::Reading::~Reading(void)
{
	stupid_mock::get(Host()).soft_call(__FUNCTION__);
}

void peterlavalle::diskio::Disk::subscribe(const scad40::duk_string& path, const scad40::duk_ptr<ChangeListener>& listener)
{
	std::stringstream log;

	log << __FUNCTION__ << "(" << path << ", " << "ChangeListener@" << (size_t)(&listener) << ")";

	stupid_mock::get(Host()).soft_call(log);
}

void peterlavalle::diskio::Disk::unsubscribe(const scad40::duk_string& path, const scad40::duk_ptr<ChangeListener>& listener)
{
	stupid_mock::get(Host()).soft_call(__FUNCTION__);
}

peterlavalle::diskio::Disk::~Disk(void)
{
	stupid_mock::get(Host()).soft_call(__FUNCTION__);
}

/// this need some extra fiddlin so it's done here
scad40::duk_native<peterlavalle::diskio::Reading> peterlavalle::diskio::Disk::open(const scad40::duk_string& path)
{
	stupid_mock::get(Host()).soft_call((std::string(__FUNCTION__ "(") + (const char*)path + ")").c_str());

	auto result = peterlavalle::diskio::Reading::New(Host());

	result->_path = path;

	return result;
}

void peterlavalle::diskio::Disk::foobar(const scad40::duk_string& text)
{
	std::stringstream log;

	log << __FUNCTION__ << "(" << (const char*)text << ")";

	stupid_mock::get(Host()).soft_call(log.str().c_str());
}
