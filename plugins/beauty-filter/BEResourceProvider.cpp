#include "BEResourceProvider.h"

BEResourceProvider::BEResourceProvider() {

}

BEResourceProvider& BEResourceProvider::getInstance() {
	static BEResourceProvider s_instance;
	return s_instance;
}

void BEResourceProvider::initialize(const std::string& appDir) {
	setResourceDir(appDir + "beautyresource_v2/");
	m_licenseDir = m_resourceDir + "license/";
	m_modelDir = m_resourceDir + "ModelResource.bundle";
	m_filterDir = m_resourceDir + "FilterResource.bundle/Filter/";
	m_faceCacheDir = m_resourceDir + "faceCache/";
	m_stickerDir = m_resourceDir + "stickers_resource/";
	m_composerNodeDir = m_resourceDir + "ComposeMakeup.bundle/ComposeMakeup/";
}

std::string& BEResourceProvider::licenseDirPath() {
	return m_licenseDir;
}

std::string& BEResourceProvider::modelDirPath() {
	return m_modelDir;
}

std::string& BEResourceProvider::filterPath() {
	return m_filterDir;
}

std::string& BEResourceProvider::stickerPath() {
	return m_stickerDir;
}

std::string& BEResourceProvider::composerNodePath() {
	return m_composerNodeDir;
}

std::string& BEResourceProvider::getFaceCacheDir() {
	return m_faceCacheDir;
}

void BEResourceProvider::setResourceDir(const std::string& resDir) {
	m_resourceDir = resDir;
	if (m_resourceDir.length() == 0) {
		printf("Error: invalid model dir : %s \n", m_resourceDir.c_str());
		return;
	}
	if (m_resourceDir.rfind("/") != m_resourceDir.length() - 1) {
		m_resourceDir.append("/");
	}
}
