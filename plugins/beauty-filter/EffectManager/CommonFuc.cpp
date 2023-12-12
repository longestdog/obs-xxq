#include "CommonFuc.h"

#if defined WIN32 || _WINDOWS
#include <Windows.h>

#pragma warning(disable: 4996)
#endif

#if defined WIN32 || _WINDOWS
bool CommonFunc::isWin7lowerSystem() {
	//判断win8.1以下的版本
	SYSTEM_INFO info;
	GetSystemInfo(&info);
	OSVERSIONINFOEX os;
	os.dwOSVersionInfoSize = sizeof(OSVERSIONINFOEX);
	if (GetVersionEx((OSVERSIONINFO*)&os))
	{
		if (os.dwMajorVersion > 6 ||
			(os.dwMajorVersion == 6 && os.dwMinorVersion >= 2)) {
			return false;
		}
	}
	return true;
}
#endif
