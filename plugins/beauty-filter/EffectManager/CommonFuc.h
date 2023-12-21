#ifndef BE_COMMONFUNC_H
#define BE_COMMONFUNC_H

class CommonFunc {
public:
#if defined WIN32 || _WINDOWS
	static bool isWin7lowerSystem();
#endif

};

#endif
