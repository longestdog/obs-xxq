#include "BEAlgorithmManager.h"
#include "AlgorithmTask/BEFaceAlgorithmTask.h"
#include "AlgorithmTask/BEFaceAttrAlgorithmTask.h"
#include "AlgorithmTask/BEFaceImageAlgorithmTask.h"
#include "AlgorithmTask/BEFaceVerifyAlgorithmTask.h"
#include "AlgorithmTask/BEHandAlgorithmTask.h"
#include "AlgorithmTask/BEHairParseAlgorithmTask.h"
#include "AlgorithmTask/BEGazeEstimationAlgorithmTask.h"
#include "AlgorithmTask/BEHeadSegAlgorithmTask.h"
#include "AlgorithmTask/BELightClsAlgorithmTask.h"
#include "AlgorithmTask/BEC1AlgorithmTask.h"
#include "AlgorithmTask/BEC2AlgorithmTask.h"
#include "AlgorithmTask/BEVideoClsAlgorithmTask.h"
#include "AlgorithmTask/BESkeletonAlgorithmTask.h"
#include "AlgorithmTask/BEPortraitMattingAlgorithmTask.h"
#include "AlgorithmTask/BEPetFaceAlgorithmTask.h"
#include "AlgorithmTask/BEChromaKeyingAlgorithmTask.h"
#include "AlgorithmTask/BETaskFactory.h"

BYTED_EFFECT::IAlgorithmManager* __stdcall createAlgorithmManager(bool imageMode) {
	return new BYTED_EFFECT::BEAlgorithmManager(imageMode);
}

void __stdcall destroyAlgorithmManager(BYTED_EFFECT::IAlgorithmManager* manager) {
	if (manager) {
		delete manager;
		manager = nullptr;
	}
}

namespace BYTED_EFFECT {
	BEAlgorithmManager::BEAlgorithmManager(bool bImageMode)
		:m_outputInfo(nullptr),
		m_inputInfo(nullptr),
		m_resourceProvider(nullptr),
		m_bImageMode(bImageMode) {

	}

	BEAlgorithmManager::~BEAlgorithmManager() {

	}


	void BEAlgorithmManager::destroy() {
		m_bDestroy = true;
		if (m_outputInfo) {
			delete m_outputInfo;
			m_outputInfo = nullptr;
		}

		if (m_inputInfo) {
			delete m_inputInfo;
			m_inputInfo = nullptr;
		}

		for (auto& it : m_taskKeyMap) {
			if (it.first != nullptr) {
				delete it.first;
			}
			if (it.second != nullptr) {
				it.second->destroyTask();
				delete it.second;
			}
		}
		m_taskKeyMap.clear();
	}

	void BEAlgorithmManager::addAlgorithm(BE_FEATURE algId) {
		std::lock_guard<std::mutex> al(m_taskMutex);
		BETask* pTask = nullptr;
		switch (algId)
		{
		case BE_FEATURE_FACE: {
			BETaskKey* pKey = BEFaceAlgorithmTask::face106Key();
			getAndCreateTask(pKey);
		}
				      break;
		case BE_FEATURE_FACE_EXTRA: {
			BETask* pFaceTask = getTask(BEFaceAlgorithmTask::face106Key());
			if (pFaceTask) {
				pFaceTask->addConfig(BEFaceAlgorithmTask::face280Key(), true);
			}
		}
					    break;
		case BE_FEATURE_FACE_ATTRIBUTE: {
			BETaskKey* pKey = BEFaceAttrAlgorithmTask::faceAttrKey();
			getAndCreateTask(pKey);
		}
						break;
		case BE_FEATURE_SKELETON: {
			BETaskKey* pKey = BESkeletonAlgorithmTask::skeletonKey();
			getAndCreateTask(pKey);
		}
					  break;
		case BE_FEATURE_HAND_DETECT: {
			BETaskKey* pKey = BEHandAlgorithmTask::handKey();
			getAndCreateTask(pKey);
		}
					     break;
		case BE_FEATURE_HAIRPARSE: {
			BETaskKey* pKey = BEHairParseAlgorithmTask::hairKey();
			getAndCreateTask(pKey);
		}
					   break;
		case BE_FEATURE_PORTRAIT_MATTING: {
			BETaskKey* pKey = BEPortraitMattingAlgorithm::portraitMattingKey();
			getAndCreateTask(pKey);
		}
						  break;
		case BE_FEATURE_FACE_VERIFY: {
			m_inputInfo->m_faceImageInfo = nullptr;
			BETaskKey* pKey = BEFaceVerifyAlgorithmTask::faceVerifyKey();
			getAndCreateTask(pKey);
		}
					     break;
		case BE_FEATURE_LIGHT_CLASSIFY: {
			BETaskKey* pKey = BELightClsAlgorithmTask::lightClsKey();
			getAndCreateTask(pKey);
		}
						break;
		case BE_FEATURE_FACE_MOUSE_MASK: {
			BETask* pFaceTask = getTask(BEFaceAlgorithmTask::face106Key());
			if (pFaceTask) {
				pFaceTask->addConfig(BEFaceAlgorithmTask::mouthMaskKey(), true);
			}
		}
						 break;
		case BE_FEATURE_FACE_TEETH_MASK: {
			BETask* pFaceTask = getTask(BEFaceAlgorithmTask::face106Key());
			if (pFaceTask) {
				pFaceTask->addConfig(BEFaceAlgorithmTask::teethMaskKey(), true);
			}
		}
						 break;
		case BE_FEATURE_FACE_MASK: {
			BETask* pFaceTask = getTask(BEFaceAlgorithmTask::face106Key());
			if (pFaceTask) {
				pFaceTask->addConfig(BEFaceAlgorithmTask::faceMaskKey(), true);
			}
		}
					   break;
		case BE_FEATURE_HEAD_SEG: {
			BETaskKey* pKey = BEHeadSegAlgorithmTask::headKey();
			getAndCreateTask(pKey);
		}
					  break;
		case BE_FEATURE_C1: {
			BETaskKey* pKey = BEC1AlgorithmTask::c1Key();
			getAndCreateTask(pKey);
		}
				    break;
		case BE_FEATURE_C2: {
			BETaskKey* pKey = BEC2AlgorithmTask::c2Key();
			getAndCreateTask(pKey);
		}
				    break;
		case BE_FEATURE_VIDEO: {
			BETaskKey* pKey = BEVideoClsAlgorithmTask::videoClsKey();
			getAndCreateTask(pKey);
		}
				       break;
		case BE_FEATURE_GAZE_ESTIMATION: {
			BETaskKey* pKey = BEGazeEstiMationAlgorithmTask::gazeKey();
			getAndCreateTask(pKey);
		}
						 break;
		case BE_FEATURE_PET_FACE: {
			BETaskKey* pKey = BEPetFaceAlgorithmTask::petFaceKey();
			getAndCreateTask(pKey);
		}
					  break;
		case BE_FEATURE_CHROMAKEYING: {
			BETaskKey* pKey = BEChromaKeyingAlgorithmTask::chromaKeingKey();
			getAndCreateTask(pKey);
		}
					      break;
		case BE_FEATURE_CHROMAKEYING_SMOOTH: {
			BETask* pChromaTask = getTask(BEChromaKeyingAlgorithmTask::chromaKeingKey());
			if (pChromaTask) {
				pChromaTask->addConfig(BEChromaKeyingAlgorithmTask::chromaGreenKeepModeKey(), true);
			}
		}
		default:
			break;
		}
	}

	void BEAlgorithmManager::removeAlgorithm(BE_FEATURE algId) {
		std::lock_guard<std::mutex> al(m_taskMutex);
		BETask* pTask = nullptr;
		switch (algId)
		{
		case BE_FEATURE_FACE: {
			BETaskKey* pKey = BEFaceAlgorithmTask::face106Key();
			removeTask(pKey);
		}
				      break;
		case BE_FEATURE_FACE_EXTRA: {
			BETask* pFaceTask = getTask(BEFaceAlgorithmTask::face106Key());
			if (pFaceTask) {
				pFaceTask->removeConfig(BEFaceAlgorithmTask::face280Key());
			}
		}
					    break;
		case BE_FEATURE_FACE_ATTRIBUTE: {
			BETaskKey* pKey = BEFaceAttrAlgorithmTask::faceAttrKey();
			removeTask(pKey);
		}
						break;
		case BE_FEATURE_SKELETON: {
			BETaskKey* pKey = BESkeletonAlgorithmTask::skeletonKey();
			removeTask(pKey);
		}
					  break;
		case BE_FEATURE_HAND_DETECT: {
			BETaskKey* pKey = BEHandAlgorithmTask::handKey();
			removeTask(pKey);
		}
					     break;
		case BE_FEATURE_HAIRPARSE: {
			BETaskKey* pKey = BEHairParseAlgorithmTask::hairKey();
			removeTask(pKey);
		}
					   break;
		case BE_FEATURE_PORTRAIT_MATTING: {
			BETaskKey* pKey = BEPortraitMattingAlgorithm::portraitMattingKey();
			removeTask(pKey);
		}
						  break;
		case BE_FEATURE_FACE_VERIFY: {
			BETaskKey* pKey = BEFaceVerifyAlgorithmTask::faceVerifyKey();
			removeTask(pKey);
		}
					     break;
		case BE_FEATURE_LIGHT_CLASSIFY: {
			BETaskKey* pKey = BELightClsAlgorithmTask::lightClsKey();
			removeTask(pKey);
		}
						break;
		case BE_FEATURE_FACE_MOUSE_MASK: {
			BETask* pFaceTask = getTask(BEFaceAlgorithmTask::face106Key());
			if (pFaceTask) {
				pFaceTask->removeConfig(BEFaceAlgorithmTask::mouthMaskKey());
			}
		}
						 break;
		case BE_FEATURE_FACE_TEETH_MASK: {
			BETask* pFaceTask = getTask(BEFaceAlgorithmTask::face106Key());
			if (pFaceTask) {
				pFaceTask->removeConfig(BEFaceAlgorithmTask::teethMaskKey());
			}
		}
						 break;
		case BE_FEATURE_FACE_MASK: {
			BETask* pFaceTask = getTask(BEFaceAlgorithmTask::face106Key());
			if (pFaceTask) {
				pFaceTask->removeConfig(BEFaceAlgorithmTask::faceMaskKey());
			}
		}
					   break;
		case BE_FEATURE_HEAD_SEG: {
			BETaskKey* pKey = BEHeadSegAlgorithmTask::headKey();
			removeTask(pKey);
		}
					  break;
		case BE_FEATURE_C1: {
			BETaskKey* pKey = BEC1AlgorithmTask::c1Key();
			removeTask(pKey);
		}
				    break;
		case BE_FEATURE_C2: {
			BETaskKey* pKey = BEC2AlgorithmTask::c2Key();
			removeTask(pKey);
		}
				    break;
		case BE_FEATURE_VIDEO: {
			BETaskKey* pKey = BEVideoClsAlgorithmTask::videoClsKey();
			removeTask(pKey);
		}
				       break;
		case BE_FEATURE_GAZE_ESTIMATION: {
			BETaskKey* pKey = BEGazeEstiMationAlgorithmTask::gazeKey();
			removeTask(pKey);
		}
						 break;
		case BE_FEATURE_PET_FACE: {
			BETaskKey* pKey = BEPetFaceAlgorithmTask::petFaceKey();
			removeTask(pKey);
		}
					  break;
		case BE_FEATURE_CHROMAKEYING: {
			BETaskKey* pKey = BEChromaKeyingAlgorithmTask::chromaKeingKey();
			removeTask(pKey);
		}
					      break;
		case BE_FEATURE_CHROMAKEYING_SMOOTH: {
			BETask* pChromaTask = getTask(BEChromaKeyingAlgorithmTask::chromaKeingKey());
			if (pChromaTask) {
				pChromaTask->removeConfig(BEChromaKeyingAlgorithmTask::chromaGreenKeepModeKey());
			}
		}
		default:
			break;
		}
	}

	void BEAlgorithmManager::removeAllAlgorithm() {
		//if (auto& it : m_taskKeyMap) {
		//	//itFind->second->addConfig(pTaskKey, false);
		//	//itFind->second->setProcess(false);
		//}
		std::lock_guard<std::mutex> al(m_taskMutex);
		for (auto& it : m_taskKeyMap) {
			it.second->resetAllConfig();
			it.second->setProcess(false);
		}
	}

	BEAlgorithmTask* BEAlgorithmManager::getTask(BETaskKey* pTaskKey) {
		//std::lock_guard<std::mutex> lockGuard(m_taskMutex);
		auto itFind = m_taskKeyMap.find(pTaskKey);
		if (itFind != m_taskKeyMap.end()) {
			return dynamic_cast<BEAlgorithmTask*>(itFind->second);
		}
		return nullptr;
	}

	BEAlgorithmTask* BEAlgorithmManager::getAndCreateTask(BETaskKey* pTaskKey) {
		//std::lock_guard<std::mutex> lockGuard(m_taskMutex);
		auto itFind = m_taskKeyMap.find(pTaskKey);
		if (itFind != m_taskKeyMap.end()) {
			itFind->second->addConfig(pTaskKey, true);
			itFind->second->setProcess(true);
			m_bNeedRebuildChain = true;
			return dynamic_cast<BEAlgorithmTask*>(itFind->second);
		}

		BETask* pTask = BETaskFactory::instance()->create(pTaskKey);
		pTask->addConfig(pTaskKey, true);
		addTask(pTaskKey, pTask);
		m_bNeedRebuildChain = true;
		return dynamic_cast<BEAlgorithmTask*>(pTask);
	}

	void BEAlgorithmManager::addTask(BETaskKey* pTaskKey, BETask* pTask) {
		BEAlgorithmTask* pAlgTask = dynamic_cast<BEAlgorithmTask*>(pTask);
		pAlgTask->setResourceProvider(m_resourceProvider);
		pAlgTask->setImageMode(m_bImageMode);
		pAlgTask->initTask();
		pAlgTask->setProcess(true);
		m_taskKeyMap.insert(std::make_pair(pTaskKey, pAlgTask));
	}

	void BEAlgorithmManager::removeTask(BETaskKey* pTaskKey) {
		//std::lock_guard<std::mutex> lockGuard(m_taskMutex);
		/*auto itFind = m_taskKeyMap.find(pTaskKey);
		if (itFind != m_taskKeyMap.end()) {
		    m_taskKeyMap.erase(itFind);
		}*/
		auto itFind = m_taskKeyMap.find(pTaskKey);
		if (itFind != m_taskKeyMap.end()) {
			itFind->second->addConfig(pTaskKey, false);
			itFind->second->setProcess(false);
			itFind->second->reset();
		}
		m_bNeedRebuildChain = true;
	}

	void BEAlgorithmManager::processAlgorithm() {
		std::lock_guard<std::mutex> al(m_taskMutex);
		m_inputInfo->clear();
		m_outputInfo->clear();
		if (m_bNeedRebuildChain) {
			rebuildChain();
			m_bNeedRebuildChain = false;
		}
		for (auto& task : m_algorithmTaskList) {
			task->process(m_inputInfo, m_outputInfo);
		}
	}

	void BEAlgorithmManager::initProcessInfo() {
		if (m_outputInfo == nullptr) {
			m_outputInfo = new BEProcessOutput();
		}
		if (m_inputInfo == nullptr) {
			m_inputInfo = new BEProcessInput();
		}
	}

	void BEAlgorithmManager::initResourceFinder(const std::string& licensePath,
		const std::string& modelDir) {
		if (m_resourceProvider == nullptr) {
			m_resourceProvider = new BEAlgorithmResourceProvider(modelDir, licensePath);
		}
	}

	BEProcessInput* BEAlgorithmManager::getInputInfo() {
		return m_inputInfo;
	}

	BEProcessOutput* BEAlgorithmManager::getOutputInfo() {
		return m_outputInfo;
	}

	void BEAlgorithmManager::rebuildChain() {
		m_algorithmTaskList.clear();

		std::list<BETaskKey*> allDependsTask;
		for (auto& it : m_taskKeyMap) {
			// 插入所有的dependency
			if (!it.second->hasConfig(it.first)) {
				// 先把所有依赖的算法项设置为不处理
				it.second->setProcess(false);
			}

			if (it.second->getProcess() == false) {
				continue;
			}

			std::list<BETaskKey*> dependsTask = it.second->getDependency();



			for (auto& curDependsKey : dependsTask) {
				/*bool bHasSameKey = false;
				for (auto& dependsKey : allDependsTask) {
				    if (dependsKey == curDependsKey) {
					bHasSameKey = true;
					break;
				    }
				}
				if (!bHasSameKey) {
				    allDependsTask.push_back(curDependsKey);
				}*/
				allDependsTask.push_back(curDependsKey);
			}

		}

		for (auto& dependsKey : allDependsTask) {
			auto itKey = m_taskKeyMap.find(dependsKey);
			if (itKey == m_taskKeyMap.end()) {
				BETask* pTask = BETaskFactory::instance()->create(dependsKey);

				pTask->addConfig(dependsKey, false);

				BEAlgorithmTask* pAlgTask = dynamic_cast<BEAlgorithmTask*>(pTask);
				pAlgTask->setResourceProvider(m_resourceProvider);
				pAlgTask->initTask();
				pAlgTask->setImageMode(m_bImageMode);
				pAlgTask->setProcess(true);
				m_taskKeyMap.insert(std::make_pair(dependsKey, pAlgTask));
			}
			else {
				itKey->second->setProcess(true);
			}
		}

		for (auto& it : m_taskKeyMap) {
			if (it.second->getProcess()) {
				m_algorithmTaskList.push_back(it.second);
			}
		}


		// 重新按照优先级排序
		m_algorithmTaskList.sort([](BEAlgorithmTask* pTask1, BEAlgorithmTask* pTask2)
		{
			return pTask1->getPriority() < pTask2->getPriority();
		});

	}

	int BEAlgorithmManager::setFaceVerifySource(unsigned char* image,
		int width,
		int height,
		int format) {
		std::lock_guard<std::mutex> al(m_taskMutex);
		if (image == nullptr ||
			width <= 0 ||
			height <= 0) {
			return -1;
		}

		BEProcessInput* inputInfo = new BEProcessInput();
		inputInfo->m_buffer = image;
		inputInfo->m_width = width;
		inputInfo->m_height = height;
		inputInfo->m_bytePerRow = width * 4;
		inputInfo->m_format = (bef_ai_pixel_format)format;
		inputInfo->m_ori = BEF_AI_CLOCKWISE_ROTATE_0;


		std::list<BEAlgorithmTask*> algorithmList;

		// 添加人脸检测算法
		BETaskKey* pFaceImageKey = BEFaceImageAlgorithmTask::faceImageKey();
		BEAlgorithmTask* pFaceImageTask = getAndCreateTask(pFaceImageKey);
		algorithmList.push_back(pFaceImageTask);

		// 添加faceverify算法
		BETaskKey* pKey = BEFaceVerifyAlgorithmTask::faceVerifyKey();
		BEAlgorithmTask* pFaceVerifyTask = getAndCreateTask(pKey);
		algorithmList.push_back(pFaceVerifyTask);

		pFaceVerifyTask->addConfig(BEFaceVerifyAlgorithmTask::faceVerifyFeatureKey(), true);
		// process
		for (auto& task : algorithmList) {
			task->process(inputInfo, m_outputInfo);
		}

		int faceCount = 0;
		pFaceVerifyTask->addConfig(BEFaceVerifyAlgorithmTask::faceVerifyFeatureKey(), false);
		if (m_outputInfo->m_faceImageInfo) {
			faceCount = m_outputInfo->m_faceImageInfo->face_count;
		}

		delete inputInfo;

		return faceCount;
	}

	int BEAlgorithmManager::changeGreenKeepMode(bool isGreenKeepOn)
	{
		if (auto task = getTask(BEChromaKeyingAlgorithmTask::chromaKeingKey())) {
			task->addConfig(BEChromaKeyingAlgorithmTask::chromaGreenKeepModeKey(), isGreenKeepOn);
		}
		return 0;
	}

}
