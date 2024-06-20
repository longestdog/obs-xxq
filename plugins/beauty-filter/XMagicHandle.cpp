#include <obs-module.h>
#include <qapplication.h>
#include "XMagicHandle.h"
#include <QOffscreenSurface>
#include <QOpenGLContext>
#include <QDir>
#include <QJsonDocument>
#include <QJsonObject>
#include <QStandardPaths>

#define LICENSE_URL "https://license.vod2.myqcloud.com/license/v2/1259626909_1/v_cube.license"
#define LICENSE_KEY "514d9b028cf43eeaaba084d7d24dc9ac"
#define WIDTH 1920
#define HEIGHT 1080

void Semaphore::wait()
{
	std::unique_lock<std::mutex> lock{m_mutex};
	if (--m_count < 0)
		m_condition.wait(lock);
}

void Semaphore::signal()
{
	std::lock_guard<std::mutex> lock{m_mutex};
	if (++m_count <= 0)
		m_condition.notify_one();
}

XMagicHandle::XMagicHandle(int nWidth, int nHeight) : m_nWidth(nWidth), m_nHeight(nHeight)
{
	m_newWidth = m_nWidth;
	m_newHeight = m_nHeight;

	int nLen = WIDTH * HEIGHT * 4;
	m_pDest = new unsigned char[nLen];
	ZeroMemory(m_pDest, nLen);

	m_pSrc = new unsigned char[nLen];
	ZeroMemory(m_pSrc, nLen);

	checklicense();

	m_processThread = std::thread([=]() {
		if (!m_bProcess) {
			initializeGL();
		}
		while (true) {
			std::unique_lock<std::mutex> lock(m_processMutex);
			m_cv.wait(lock, [=]() { return m_bProcess || m_bExit; });
			if (m_bExit) {
				UninitializeGL();
				break;
			} else {
				if (m_bProcess) {
					if (m_pXmagic && (m_nWidth != m_newWidth || m_nHeight != m_newHeight)) {
						m_nWidth = m_newWidth;
						m_nHeight = m_newHeight;
						m_pXmagic->setRenderSize(m_nWidth, m_nHeight);
					}

					operatorBeauty();
					operatorMakeUp();
					operatorFilter();
					operatorStrick();

					processFrame();
				}
			}
		}
	});
}

XMagicHandle::~XMagicHandle()
{
	{
		{
			std::lock_guard<std::mutex> lock(m_processMutex);
			m_bExit = true;
		}
		m_cv.notify_all();
		if (m_processThread.joinable()) {
			m_conditionFrame.signal();
			m_processThread.join();
		}
	}

	if (m_pSrc) {
		delete[] m_pSrc;
		m_pSrc = nullptr;
	}
	if (m_pDest) {
		delete[] m_pDest;
		m_pDest = nullptr;
	}
}

void XMagicHandle::checklicense()
{
	QString localDataPath = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
	QString appName = QApplication::applicationName();
	QString logPath = QString("%1\\logs\\").arg(localDataPath);
	blog(LOG_INFO, "init XMagic logPath: %s", logPath.toStdString().c_str());
	setXmagicLogPath(logPath.toStdString().c_str());

	auto respCallback = [](int ret, const char *data) -> int {
		blog(LOG_INFO, "checklicense ret: %d,data:%p", ret, data);
		int retCode = ret;
		const char *msg = data;
		return 0;
	};

	setTELicense(LICENSE_URL, LICENSE_KEY, respCallback);
}
void XMagicHandle::addBeauty(const std::string &effectKey, int effectValue)
{
	if (!m_bExit) {
		std::lock_guard<std::mutex> lock(m_beautyMutex);
		m_beautyMap[effectKey] = effectValue;
	}
}

void XMagicHandle::operatorBeauty()
{
	std::lock_guard<std::mutex> lock(m_beautyMutex);
	for (auto it : m_beautyMap) {
		XmagicProperty property;
		property.category = Category::BEAUTY;
		std::string effectKey = it.first;
		std::string effectValue = std::to_string(it.second);
		property.effKey = effectKey.c_str();
		property.effValue = effectValue.c_str();

		if (m_pXmagic) {
			m_pXmagic->setEffect(effectKey.c_str(), std::atoi(effectValue.data()), "", "");
		}
	}
	m_beautyMap.clear();
}

void XMagicHandle::addMakeUp(const std::string resPath, int effectValue, bool isFilter /*= false*/)
{
	{
		std::lock_guard<std::mutex> lock(m_makeUpMutex);
		m_makeUpPath = resPath.empty() ? "" : exeDirectory + resPath;
		if (resPath.empty())
			m_makeUpclear = true;
		if (!isFilter)
			m_makeValue = effectValue;
		else
			m_makeFilterValue = effectValue;
	}
}

void XMagicHandle::operatorMakeUp()
{
	std::string resPath = STR_EMPTY;
	int effectValue = 0;
	int filterValue = 0;
	if (m_makeUpPath != STR_EMPTY) {
		std::lock_guard<std::mutex> lock(m_makeUpMutex);
		std::swap(m_makeUpPath, resPath);
		effectValue = m_makeValue;
		filterValue = m_makeFilterValue;
	}
	if (m_pXmagic && resPath != STR_EMPTY) {
		if (m_makeUpclear.load() && !m_lastMakeUpPath.empty()) {
			TEBeautyManager teBeautyManager;
			teBeautyManager.setMakeupEffect(m_pXmagic, m_lastMakeUpPath.c_str(), 0, 0);
			m_makeUpclear.store(false);
		}
		if (!resPath.empty()) {
			TEBeautyManager teBeautyManager;
			teBeautyManager.setMakeupEffect(m_pXmagic, resPath.empty() ? nullptr : resPath.c_str(), effectValue, filterValue);
			if (!resPath.empty())
				m_lastMakeUpPath = resPath;
		}
	}
}
void XMagicHandle::addFilter(const std::string &resPath, int effectValue)
{
	{
		std::lock_guard<std::mutex> lock(m_filterMutex);
		m_filterPath = resPath.empty() ? resPath : exeDirectory + resPath;
		m_filterValue = effectValue;
	}
}
void XMagicHandle::operatorFilter()
{
	std::string resPath = STR_EMPTY;
	int effectValue = 0;
	if (m_filterPath != STR_EMPTY) {
		std::lock_guard<std::mutex> lock(m_filterMutex);
		std::swap(m_filterPath, resPath);
		std::swap(m_filterValue, effectValue);
	}
	if (m_pXmagic && resPath != STR_EMPTY) {
		QJsonObject extraInfo;
		extraInfo["mergeWithCurrentMotion"] = "true";
		std::string sExtraInfo = QJsonDocument(extraInfo).toJson(QJsonDocument::Compact).toStdString(); //"{\" mergeWithCurrentMotion \":true}"
		m_pXmagic->setEffect(XmagicConstant::EFFECT_LUT, effectValue, resPath.empty() ? nullptr : resPath.c_str(), sExtraInfo.c_str());
	}
}

void XMagicHandle::addStrick(const std::string &resPath, int effectValue)
{
	{
		std::lock_guard<std::mutex> lock(m_stickerMutex);
		m_stickerPath = resPath;
		m_lastStickerPath = m_stickerPath;
		if (resPath.empty())
			m_stickerClear = true;
	}
}

void XMagicHandle::operatorStrick()
{
	std::string resPath = STR_EMPTY;
	if (m_stickerPath != STR_EMPTY) {
		std::lock_guard<std::mutex> lock(m_stickerMutex);
		std::swap(m_stickerPath, resPath);
		if (resPath.empty()) {
			m_lastStickerPath = STR_EMPTY;
			m_stickerClear.store(false);
		}
	}
	if (m_pXmagic && resPath != STR_EMPTY) {
		QJsonObject extraInfo;
		extraInfo["mergeWithCurrentMotion"] = "true";
		std::string sExtraInfo = QJsonDocument(extraInfo).toJson(QJsonDocument::Compact);
		m_pXmagic->setEffect(XmagicConstant::EFFECT_MOTION, 0, nullptr, nullptr);
		if (!resPath.empty())
			m_pXmagic->setEffect(XmagicConstant::EFFECT_MOTION, 0, resPath.empty() ? nullptr : resPath.c_str(), sExtraInfo.c_str());
	}
}

void XMagicHandle::setInput(uint8_t *data, int width, int height)
{
	if (!m_bExit || m_bProcess) {
		if (m_nWidth != width || m_nHeight != height) {
			m_newWidth = width;
			m_newHeight = height;
		}

		memcpy(m_pSrc, data, width * height * 4);
		{
			std::lock_guard<std::mutex> lock(m_processMutex);
			m_bProcess = true;
		}
		m_cv.notify_one();
		m_conditionFrame.wait();
	}
}

uint8_t *XMagicHandle::getOutput()
{
	return m_pDest;
}

void XMagicHandle::processFrame()
{
	YTImagePixelData src, dest;
	src.data = m_pSrc;
	src.width = m_nWidth;
	src.height = m_nHeight;
	src.length = m_nWidth * m_nHeight * 4;
	src.pixelFormat = PixelFormat::PixelFormatRGBA32;
	dest = src;
	dest.data = m_pDest;
	if (m_pXmagic) {
		m_pXmagic->process(&src, &dest);
	}
	m_bProcess = false;
	m_conditionFrame.signal();
}

std::string XMagicHandle::getFmtDTStr()
{
	std::chrono::system_clock::time_point now = std::chrono::system_clock::now();
	std::chrono::system_clock::duration tp = now.time_since_epoch();

	tp -= std::chrono::duration_cast<std::chrono::seconds>(tp);

	time_t tt = std::chrono::system_clock::to_time_t(now);
	tm t = *localtime(&tt);
	char szbuf[36] = {0};
	std::sprintf(szbuf, "%04u%02u%02u_%02u%02u%02u_%03ld", t.tm_year + 1900, t.tm_mon + 1, t.tm_mday, t.tm_hour, t.tm_min, t.tm_sec,
		     (int64_t)std::chrono::duration_cast<std::chrono::milliseconds>(tp).count());

	return szbuf;
}

void XMagicHandle::initializeGL()
{
	/*
	QSurfaceFormat format;
	format.setRenderableType(QSurfaceFormat::OpenGL);
	format.setProfile(QSurfaceFormat::CoreProfile);
	format.setVersion(4, 1);

	m_Surface = new QOffscreenSurface;
	m_Surface->setFormat(format);
	m_Surface->create();

	m_QOpenGLContext = new QOpenGLContext;
	m_QOpenGLContext->setFormat(format);
	m_QOpenGLContext->create();
	m_QOpenGLContext->makeCurrent(m_Surface);
	*/

	QString appPathQstr = QApplication::applicationDirPath();
	std::string appPath = "";
	appPath = appPathQstr.toUtf8().toStdString();
	if (!appPath.rfind("/") != appPath.length() - 1) {
		appPath.append("/");
	}
	exeDirectory = appPath + "beautyresource_v3/";
	if (QDir(QString(exeDirectory.c_str())).exists()) {
		blog(LOG_INFO, "XMagic init path:%s,width:%d,height:%d", exeDirectory.c_str(), m_nWidth, m_nHeight);
		m_pXmagic = createXmagic(exeDirectory.data(), m_nWidth, m_nHeight);
	}
}

void XMagicHandle::UninitializeGL()
{
	m_bProcess = false;

	if (m_pXmagic) {
		destroyXmagic(&m_pXmagic);
		m_pXmagic = nullptr;
	}
	//delete m_QOpenGLContext;
	//delete m_Surface;
}
