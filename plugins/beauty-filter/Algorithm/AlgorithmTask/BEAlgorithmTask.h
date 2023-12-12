#ifndef ALGORITHM_TASK_H
#define ALGORITHM_TASK_H

#include "BETask.h"
#include "BEProcessOutput.h"


class BEAlgorithmResourceProvider {
public:
	BEAlgorithmResourceProvider(const std::string& modelDir, const std::string& licensePath);
	~BEAlgorithmResourceProvider();


	const std::string& getModelDir() { return m_modelDir; }
	const std::string& getLicensePath() { return m_licensePath; }
protected:
	std::string m_modelDir;
	std::string m_licensePath;
};


class BEAlgorithmTask : public BETask {
public:
	BEAlgorithmTask();
	virtual ~BEAlgorithmTask();

public:
	virtual void initTask() = 0;
	void setResourceProvider(BEAlgorithmResourceProvider* provider);

	const std::string& getLicensePath();
	void setImageMode(bool v);
	bool getImageMode();
	void setProcess(bool v);
	bool getProcess();

	virtual void process(BEProcessInput* input, BEProcessOutput* output);
protected:
	BEAlgorithmResourceProvider* m_provider = nullptr;
	bool m_bImageMode = false;
	bool m_bNeedProcess = false;
};

#endif