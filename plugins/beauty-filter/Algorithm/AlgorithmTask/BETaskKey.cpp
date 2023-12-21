#include "BETaskKey.h"

BETaskKey::BETaskKey() {

}

BETaskKey::~BETaskKey() {

}


void BETaskKey::init(int taskId) {
	m_taskId = taskId;
}

void BETaskKey::init(int taskId, const std::string& name) {
	m_keyName = name;
	m_taskId = taskId;
}

BETaskKey* BETaskKey::dependency() {
	return nullptr;
}

int BETaskKey::getTaskId() {
	return m_taskId;
}

///////////////////////////////////////////////
BETaskKeyFactory::BETaskKeyFactory() {
	m_keyIndex = 0;
}

BETaskKey* BETaskKeyFactory::create() {
	BETaskKey* pTaskKey = new BETaskKey();
	pTaskKey->init(m_keyIndex++);
	return pTaskKey;
}

BETaskKey* BETaskKeyFactory::create(std::string& name) {
	auto itFind = m_taskKeyMap.find(name);
	if (itFind == m_taskKeyMap.end()) {
		BETaskKey* pTaskKey = new BETaskKey();
		pTaskKey->init(m_keyIndex++, name);
		m_taskKeyMap[name] = pTaskKey;
		return pTaskKey;
	}
	else {
		return itFind->second;
	}
}

BETaskKey* BETaskKeyFactory::create(const char* name) {
	if (name == nullptr) {
		return nullptr;
	}

	return create(std::string(name));
}