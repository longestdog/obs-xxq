#include "BETask.h"


BETask::BETask() {

}

BETask::~BETask() {

}

bool BETask::hasConfig(BETaskKey* key) {
	auto itFind = m_configMap.find(key);
	if (itFind == m_configMap.end()) {
		return false;
	}

	return itFind->second;
}

void BETask::addConfig(BETaskKey* key, bool value) {
	m_configMap[key] = value;

	rebuild();
}

void BETask::removeConfig(BETaskKey* key) {
	auto itFind = m_configMap.find(key);
	if (itFind != m_configMap.end()) {
		m_configMap.erase(itFind);
	}
	rebuild();
}

void BETask::resetAllConfig() {
	for (auto& it : m_configMap) {
		it.second = false;
	}
}

void BETask::setConfig(std::unordered_map<BETaskKey*, bool>& configMap) {
	m_configMap = std::move(configMap);
}

std::list<BETaskKey*>& BETask::getDependency() {
	return m_dependencys;
}

void BETask::reset() {

}