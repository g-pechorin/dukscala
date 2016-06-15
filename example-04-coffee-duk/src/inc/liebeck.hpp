#pragma once

#include <iostream>
#include <assert.h>
#include <stdint.h>

#if defined(_WIN32) || defined(_WIN64)
#include <Windows.h>
#else
utime ? maybe

http ://stackoverflow.com/questions/4021479/getting-file-modification-time-on-unix-using-utime-in-c

#endif

#include <time.h>

///
/// returns true IFF (file exists && file last change > last_modified && no error(s) occurred)
/// fills in last_modified with file last change IFF  (file exists && no error(s) occurred)
struct liebeck
{
	static inline bool changed(const char* filename, uint64_t& last_modified)
	{
#if defined(_WIN32) || defined(_WIN64)

		// https://msdn.microsoft.com/en-us/library/windows/desktop/aa363858(v=vs.85).aspx
		const HANDLE filehandle = CreateFileA(filename, GENERIC_READ, FILE_SHARE_READ, nullptr, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, nullptr);

		if (INVALID_HANDLE_VALUE == filehandle)
		{
			auto lastError = GetLastError();

			if (ERROR_FILE_NOT_FOUND != lastError)
			{
				std::cerr << "??? lastError = " << lastError << std::endl;
			}

			return false;
		}

		// https://msdn.microsoft.com/en-us/library/windows/desktop/ms724320(v=vs.85).aspx
		FILETIME filetime;
		const BOOL okayGet = GetFileTime(filehandle, nullptr, nullptr, &filetime);
		if (!okayGet)
		{
			auto lastError = GetLastError();
			std::cerr << "???" << std::endl;
		}

		// https://msdn.microsoft.com/en-us/library/windows/desktop/ms724211(v=vs.85).aspx
		const auto okayClose = CloseHandle(filehandle);
		if (!okayClose)
		{
			auto lastError = GetLastError();
			std::cerr << "???" << std::endl;
		}

		// https://msdn.microsoft.com/en-us/library/windows/desktop/ms724284(v=vs.85).aspx
		static_assert(sizeof(DWORD) == sizeof(uint32_t), "Whoops");
		uint64_t timelast = filetime.dwHighDateTime;
		timelast *= static_cast<uint64_t>(0xFFFFFFFF);
		timelast += static_cast<uint64_t>(filetime.dwLowDateTime);
		timelast /= static_cast<uint64_t>(100);
		timelast -= static_cast<uint64_t>(11644473600L); // http://stackoverflow.com/questions/6161776/convert-windows-filetime-to-second-in-unix-linux#6161842

		if (okayGet && okayClose && last_modified < timelast)
		{
			last_modified = timelast;
			return true;
		}

		assert(!okayGet || !okayClose || last_modified == timelast);

		return false;
#else
		utime ? maybe
#endif
	}
};