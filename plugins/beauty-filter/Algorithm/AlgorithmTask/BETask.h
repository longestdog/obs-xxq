#ifndef BETASK_H
#define BETASK_H

#include "BETaskKey.h"

class BETask {
public:
	BETask();
	virtual ~BETask();

public:
	virtual void initTask() = 0;
	virtual void destroyTask() = 0;
	// 获取处理的优先级
	virtual int getPriority() = 0;
	virtual BETaskKey* getKey() = 0;
	virtual void rebuild() = 0;
	virtual void reset();

	std::list<BETaskKey*>& getDependency();
	bool hasConfig(BETaskKey* key);
	void addConfig(BETaskKey* key, bool value);
	void removeConfig(BETaskKey* key);
	void resetAllConfig();
	virtual void setConfig(std::unordered_map<BETaskKey*, bool>& configMap);
protected:
	std::unordered_map<BETaskKey*, bool> m_configMap;
	std::list<BETaskKey*> m_dependencys;
};


#endif
