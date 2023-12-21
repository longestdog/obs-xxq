
//#include "be_defines.h"
#include "be_util.h"

#include <memory>
#include <time.h>
#include <mutex>




bool GetEnableProfileTimeCost()
{
#ifdef ENABLE_PROFILE
	return true;
#else
	return false;
#endif
}

long GetCurrentMillieconds() {
	return clock();
}


ScopeTimer::ScopeTimer() {

}

ScopeTimer::ScopeTimer(const char* msg) {
	m_msg = std::string(msg);
	m_time = GetCurrentMillieconds();
}

ScopeTimer::~ScopeTimer() {
	if (GetEnableProfileTimeCost() && !m_finished)
	{
		long interval = GetCurrentMillieconds() - m_time;
		BYTE_TIME_COST_PRINT(m_msg.c_str(), interval);
	}
}

void ScopeTimer::finish() {
	if (GetEnableProfileTimeCost() && !m_finished)
	{
		m_finished = true;
		long interval = GetCurrentMillieconds() - m_time;
		BYTE_TIME_COST_PRINT(m_msg.c_str(), interval);
	}
}

void ScopeTimer::reset(const char* msg) {
	m_finished = false;
	m_msg = std::string(msg);
	m_time = GetCurrentMillieconds();
}

//////////////////////////////////////////////////
ScopeTimer ScopeTimerManager::m_scopTimer;

void ScopeTimerManager::startTimer(const char* msg) {
	m_scopTimer.reset(msg);
}

void ScopeTimerManager::stopTimer() {
	m_scopTimer.finish();
}