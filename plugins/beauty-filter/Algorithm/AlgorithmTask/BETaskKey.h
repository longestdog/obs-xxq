#ifndef BETASK_KEY_H
#define BETASK_KEY_H

#include <atomic>
#include <string>
#include <unordered_map>

enum class ALGORITHM_TYPE {
	ALGORITHM_NONE = 0,
	ALGORITHM_FACE = 2,
	ALGORITHM_FACE_EXTRA = 3,
	ALGORITHM_FACE_ATTRIBUTE = 4,
	ALGORITHM_SKELETON = 5,
	ALGORITHM_HAND_DETECT = 6,
	ALGORITHM_HAND_GESTURE = 7,
	ALGORITHM_HAND_SEG = 8,
	ALGORITHM_HAND_KEY_POINT = 9,
	ALGORITHM_HAND_BOX = 10,
	ALGORITHM_HAIRPARSE = 11,
	ALGORITHM_PORTRAIT_MATTING = 12,
	ALGORITHM_FACE_VERIFY = 13,
	ALGORITHM_LIGHT_CLASSIFY = 14,
	ALGORITHM_FACE_MOUSE_MASK = 15,
	ALGORITHM_FACE_TEETH_MASK = 16,
	ALGORITHM_FACE_MASK = 17,
	ALGORITHM_HEAD_SEG = 18,
	ALGORITHM_C1 = 19,
	ALGORITHM_C2 = 20,
	ALGORITHM_VIDEO = 21,
	ALGORITHM_GAZE_ESTIMATION = 22,
	ALGORITHM_PET_FACE = 23,

};

class BETaskKey {
public:
	BETaskKey();
	~BETaskKey();


public:
	void init(int taskId);
	void init(int taskId, const std::string& name);
	BETaskKey* dependency();

	int getTaskId();
	void setIsTask(bool v) { m_isTask = v; }
	bool getIsTask() { return m_isTask; }

private:
	int m_taskId;
	bool m_isTask;
	std::string m_keyName;
};


class BETaskKeyFactory {
public:
	static BETaskKeyFactory* instance() {
		static BETaskKeyFactory s_taskKeyFactory;
		return &s_taskKeyFactory;
	}
	BETaskKey* create();
	BETaskKey* create(std::string& name);
	BETaskKey* create(const char* name);
private:
	BETaskKeyFactory();
private:
	std::atomic<int> m_keyIndex;
	std::unordered_map<std::string, BETaskKey*> m_taskKeyMap;
};

#define TASK_KEY_CREATE(name) \
BETaskKey* _key = nullptr; \
_key = BETaskKeyFactory::instance()->create(#name); \
_key->setIsTask(false); \
return _key;

#define TASK_KEY_CREATE_TRUE(name) \
BETaskKey* _key = nullptr; \
_key = BETaskKeyFactory::instance()->create(#name); \
_key->setIsTask(true); \
return _key;


#endif
