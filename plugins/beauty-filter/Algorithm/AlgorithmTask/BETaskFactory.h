#ifndef BETASK_FACTORY_H
#define BETASK_FACTORY_H

#include "BETask.h"
#include <unordered_map>

using TaskCreator = BETask * (*)();

class BETaskFactory {
public:
	static BETaskFactory* instance() {
		static BETaskFactory s_taskFactory;
		return &s_taskFactory;
	}
	BETask* create(BETaskKey* key);
	void registerTask(BETaskKey* key, TaskCreator creator);

private:
	std::unordered_map<BETaskKey*, TaskCreator> m_taskRegisterMap;
};

#define TASK_REGISTER(key, className) \
BETaskFactory::instance()->registerTask(key, []() -> BETask* { return new className();});

#define TASK_KEY_CREATE_AND_REGISTER(name, className) \
BETaskKey* _key = nullptr; \
_key = BETaskKeyFactory::instance()->create(#name); \
_key->setIsTask(true); \
BETaskFactory::instance()->registerTask(_key, []() -> BETask* { return new className();}); \
return _key;


#endif