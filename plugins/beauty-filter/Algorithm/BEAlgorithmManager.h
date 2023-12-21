#ifndef BEALGORITHMMANAGER_H
#define BEALGORITHMMANAGER_H


#include "AlgorithmDefine.h"
#include "AlgorithmTask/BEAlgorithmTask.h"
#include "BEProcessOutput.h"
#include "IAlgorithmManager.h"
#include <mutex>
#include <map>

namespace BYTED_EFFECT {
	class BEAlgorithmManager : public IAlgorithmManager {
	public:
		BEAlgorithmManager(bool bImageMode);
		~BEAlgorithmManager();

		void destroy();
	public:
		void initProcessInfo() override;
		void initResourceFinder(const std::string& licensePath,
			const std::string& modelDir) override;
		void addAlgorithm(BE_FEATURE algId) override;
		void removeAlgorithm(BE_FEATURE algId) override;

		void removeAllAlgorithm() override;

		void processAlgorithm() override;

		BEProcessInput* getInputInfo() override;
		BEProcessOutput* getOutputInfo() override;

	public:
		// 人脸比对源图片检测
		int setFaceVerifySource(unsigned char* image,
			int width,
			int height,
			int format) override;
		// 切换绿色保留模式
		int changeGreenKeepMode(bool isGreenKeepOn) override;

	private:
		BEAlgorithmTask* getTask(BETaskKey* pTaskKey);

		BEAlgorithmTask* getAndCreateTask(BETaskKey* pTaskKey);
		void addTask(BETaskKey* pTaskKey, BETask* pTask);
		void removeTask(BETaskKey* pTaskKey);

		// 重新刷新处理链
		void rebuildChain();

	private:
		std::list<BEAlgorithmTask*> m_algorithmTaskList;
		std::unordered_map<BETaskKey*, BEAlgorithmTask*> m_taskKeyMap;
		std::mutex m_taskMutex;
		//std::mutex m_mutex;

		BEProcessOutput* m_outputInfo = nullptr;
		BEProcessInput* m_inputInfo = nullptr;
		BEAlgorithmResourceProvider* m_resourceProvider = nullptr;

		BEAlgorithmTask* m_faceImageTask = nullptr;

		bool m_bImageMode;
		bool m_bNeedRebuildChain;

		bool m_bDestroy = false;
	};

}

#endif // BEALGORITHMMANAGER_H
