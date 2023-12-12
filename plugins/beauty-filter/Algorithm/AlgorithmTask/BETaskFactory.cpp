#include "BETaskFactory.h"

BETask* BETaskFactory::create(BETaskKey* key) {
	auto itFind = m_taskRegisterMap.find(key);
	if (itFind == m_taskRegisterMap.end()) {
		return nullptr;
	}

	auto creator = itFind->second;
	return creator();
}

void BETaskFactory::registerTask(BETaskKey* key, TaskCreator creator) {
	m_taskRegisterMap[key] = creator;
}