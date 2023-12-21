#include "BEFEffectThread.h"

NAMESPACE_BEF_EFFECT_FRAMEWORK_BEGIN

void Semaphore::wait() {
	std::unique_lock<std::mutex> lock{ m_mutex };
	if (--m_count < 0)
		m_condition.wait(lock);
}

void Semaphore::signal() {
	std::lock_guard<std::mutex> lock{ m_mutex };
	if (++m_count <= 0)
		m_condition.notify_one();
}

BEFEffectThread::BEFEffectThread() {
	m_threadStatus = ThreadStatus::THREAD_STATUS_IDLE;
}

BEFEffectThread::~BEFEffectThread() {
	stop();
}

void BEFEffectThread::start() {
	m_threadStatus = ThreadStatus::THREAD_STATUS_WORKING;
	m_thread = std::thread(&BEFEffectThread::run, this);
}

void BEFEffectThread::stop() {
	if (m_threadStatus == ThreadStatus::THREAD_STATUS_WORKING) {
		m_threadStatus = ThreadStatus::THREAD_STATUS_STOP;
		m_conditionRender.signal();
		m_thread.join();
	}
}

BEFEffectFrame BEFEffectThread::commit(const BEFEffectFrame& input, void* userData) {
	std::lock_guard<std::mutex> lockGuard(m_inputMutex);
	if (m_threadStatus != ThreadStatus::THREAD_STATUS_WORKING)
		return input;
	m_input = input;
	m_userData = userData;
	m_conditionRender.signal();
	m_conditionFrame.wait();

	return m_output;
}

void BEFEffectThread::run() {
	init();
	m_conditionBegin.signal();
	while (m_threadStatus != ThreadStatus::THREAD_STATUS_STOP) {
		m_conditionRender.wait();
		if (m_threadStatus == ThreadStatus::THREAD_STATUS_STOP) break;
		render(m_userData);
		m_conditionFrame.signal();
	}
	destroy();
	m_threadStatus = ThreadStatus::THREAD_STATUS_EXIT;
}

NAMESPACE_BEF_EFFECT_FRAMEWORK_END
