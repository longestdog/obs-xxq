#include "BEFPlatformDefine.h"
#include <thread>
#include <condition_variable>

#ifndef BEF_EFFECTTHREAD_H
#define BEF_EFFECTTHREAD_H

NAMESPACE_BEF_EFFECT_FRAMEWORK_BEGIN

struct BEFEffectFrame {
	int width = 0;
	int height = 0;
	unsigned char *buffer = nullptr;
};

class Semaphore {
public:
	Semaphore(int value = 0) : m_count{value} {}
	void wait();
	void signal();

private:
	int m_count;
	std::mutex m_mutex;
	std::condition_variable m_condition;
};

class BEFEffectThread {
public:
	enum class ThreadStatus : int { THREAD_STATUS_IDLE = 0, THREAD_STATUS_WORKING, THREAD_STATUS_STOP, THREAD_STATUS_EXIT };

	BEFEffectThread();
	virtual ~BEFEffectThread();

	void start();
	void stop();
	BEFEffectFrame commit(const BEFEffectFrame &input, void *userData = nullptr);

protected:
	BEFEffectFrame m_input;
	BEFEffectFrame m_output;
	void *m_userData = nullptr;
	std::mutex m_inputMutex;
	Semaphore m_conditionBegin;
	Semaphore m_conditionRender;
	Semaphore m_conditionFrame;
	ThreadStatus m_threadStatus;

private:
	void run();
	virtual void init() = 0;
	virtual void render(void *userData = nullptr) = 0;
	virtual void destroy() = 0;
	std::thread m_thread;
};

NAMESPACE_BEF_EFFECT_FRAMEWORK_END

#endif // BEF_EFFECTTHREAD_H
