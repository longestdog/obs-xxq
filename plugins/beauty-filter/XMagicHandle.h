#pragma once
#include "IXmagic.h"
#include <thread>
#include <condition_variable>
#include <mutex>
#include <atomic>
#include <tuple>

#define STR_EMPTY "empty"

class Semaphore {
public:
	Semaphore(int value = 0) : m_count{value} {}
	void wait();
	void signal();

private:
	int m_count;
	std::mutex m_mutex;
	std::condition_variable m_condition;
};

class QOffscreenSurface;
class QOpenGLContext;

class XMagicHandle {
public:
	XMagicHandle(int nWidth, int nHeight);
	~XMagicHandle();

public:
	//video buffer in / out
	void setInput(uint8_t *data, int width, int height);
	uint8_t *getOutput();

	void addBeauty(const std::string &effectKey, int effectValue);                     //美颜
	void addMakeUp(const std::string resPath, int effectValue, bool isFilter = false); //化妆
	void addFilter(const std::string &resPath, int effectValue);                       //滤镜
	void addStrick(const std::string &resPath, int effectValue);                       //贴纸

	//only for test
	void updateProperty();

protected:
	//operator action
	void operatorBeauty(); //美颜
	void operatorMakeUp(); //化妆
	void operatorFilter(); //滤镜
	void operatorStrick(); //贴纸
private:
	void checklicense();
	void initializeGL();
	void UninitializeGL();
	void processFrame();
	std::string getFmtDTStr();
	void restoreMakeup();

private:
	//base info
	std::string exeDirectory;
	IXmagic *m_pXmagic = nullptr;
	int m_nWidth;
	int m_nHeight;
	int m_newWidth;
	int m_newHeight;
	uint8_t *m_pSrc;
	uint8_t *m_pDest;

	//process thread
	std::thread m_processThread;
	std::atomic_bool m_bProcess{false};
	std::atomic_bool m_bExit{false};
	std::condition_variable m_cv;
	std::mutex m_processMutex;
	Semaphore m_conditionFrame;

	//Beauty operator
	std::unordered_map<std::string /*effectKey*/, int /*effectValue*/> m_beautyMap;
	std::mutex m_beautyMutex;

	//Filter
	std::string m_filterPath = STR_EMPTY;
	int m_filterValue = 0;
	std::mutex m_filterMutex;

	//MakeUp operator
	std::string m_makeUpPath = STR_EMPTY;
	int m_makeValue = 0;
	int m_makeFilterValue = 0;
	std::atomic_bool m_makeUpclear{false};

	//std::list<std::tuple<std::string, int, int>> m_makeUpList;
	std::tuple<std::string, int, int> m_lastMakeUp = {STR_EMPTY, 0, 0};

	std::mutex m_makeUpMutex;

	//Sticker operator
	std::string m_stickerPath = STR_EMPTY;
	std::string m_lastStickerPath = STR_EMPTY; //设置美妆nullptr会影响贴纸
	std::atomic_bool m_stickerClear{false};
	//std::list<std::string> m_stickerList;
	std::mutex m_stickerMutex;

	//QOffscreenSurface* m_Surface;
	//QOpenGLContext* m_QOpenGLContext;
};
