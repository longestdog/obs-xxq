#include "BEAlgorithmTask.h"

BEAlgorithmResourceProvider::BEAlgorithmResourceProvider(const std::string& modelDir,
	const std::string& licensePath) {
	m_modelDir = modelDir;
	m_licensePath = licensePath;
}

BEAlgorithmResourceProvider::~BEAlgorithmResourceProvider() {

}



///////////////////////////////////////////////////////////////

BEAlgorithmTask::BEAlgorithmTask() {

}

BEAlgorithmTask::~BEAlgorithmTask() {

}

void BEAlgorithmTask::setResourceProvider(BEAlgorithmResourceProvider* provider) {
	m_provider = provider;
}

const std::string& BEAlgorithmTask::getLicensePath() {
	if (m_provider) {
		return m_provider->getLicensePath();
	}

	return "";
}

void BEAlgorithmTask::setImageMode(bool v) {
	m_bImageMode = v;
}

bool BEAlgorithmTask::getImageMode() {
	return m_bImageMode;
}

void BEAlgorithmTask::setProcess(bool v) {
	m_bNeedProcess = v;
}

bool BEAlgorithmTask::getProcess() {
	return m_bNeedProcess;
}

void BEAlgorithmTask::process(BEProcessInput* input, BEProcessOutput* output) {

}