#ifndef IALGORITHM_H
#define IALGORITHM_H

#include "BEProcessOutput.h"
#include <string>

namespace BYTED_EFFECT {

	class ALGORITHM_CPP_API IAlgorithmManager {
	public:
		IAlgorithmManager() {}
		virtual ~IAlgorithmManager() {}

	public:
		virtual void initProcessInfo() = 0;
		virtual void initResourceFinder(const std::string& licensePath,
			const std::string& modelDir) = 0;
		virtual void addAlgorithm(BE_FEATURE algId) = 0;
		virtual void removeAlgorithm(BE_FEATURE algId) = 0;

		virtual void removeAllAlgorithm() = 0;

		virtual void processAlgorithm() = 0;

		virtual int setFaceVerifySource(unsigned char* image,
			int width,
			int height,
			int format) = 0;

		virtual int changeGreenKeepMode(bool isGreenKeepOn) = 0;

		virtual BEProcessInput* getInputInfo() = 0;
		virtual BEProcessOutput* getOutputInfo() = 0;
	};

};

BYTED_EFFECT::IAlgorithmManager* __stdcall createAlgorithmManager(bool imageMode);
void __stdcall destroyAlgorithmManager(BYTED_EFFECT::IAlgorithmManager* manager);

#endif
