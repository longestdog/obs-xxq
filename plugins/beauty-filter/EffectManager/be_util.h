#ifndef BE_UTIL_H
#define BE_UTIL_H


#include <stdio.h>
#include <string>
#include "bef_effect_ai_public_define.h"

#define CHECK_BEF_AI_RET_SUCCESS(ret, mes) \
if(ret != 0){\
    printf("error id = %d,  %s !\n",ret, mes); \
    return ret;\
}

#define CHECK_BEF_AI_RET_SUCCESS_NO_RETURN(ret, mes)\
if (ret != 0)\
{\
    printf("error id = %d,  %s !\n",ret, mes); \
}

#define BE_DELETE_POINTER(ptr)\
if (ptr != nullptr)\
{\
    delete ptr;\
}

#define BE_DELETE_POINTER_ARRAY(ptr)\
if (ptr != nullptr)\
{\
delete[] ptr; \
}

#define BYTE_TIME_COST_PRINT(func,time) do{ \
    if(1){\
        printf("Profile %s estimation time %lu\n", func, time);\
    }\
} while(0)

long GetCurrentMillieconds();


bool GetEnableProfileTimeCost();


typedef struct BEImage
{
	unsigned char* buffer;
	int width;
	int height;
	int stride;
	int channel;
	bef_ai_pixel_format format;
	bef_ai_rotate_type ori;
} BEImage;


class ScopeTimer {
public:
	ScopeTimer();
	ScopeTimer(const char* msg);
	~ScopeTimer();
	void reset(const char* msg);
	void finish();
private:
	long m_time = 0;
	std::string m_msg = "";
	bool m_finished = false;
};

class ScopeTimerManager {
private:
	ScopeTimerManager() {}
	~ScopeTimerManager() {}

public:
	static void startTimer(const char* msg);
	static void stopTimer();
private:
	static ScopeTimer m_scopTimer;
};

#define START_ALG_TIMER(detectName) \
ScopeTimerManager::startTimer(detectName);

#define STOP_ALG_TIMER() \
ScopeTimerManager::stopTimer();

#endif // !BE_UTIL_H
