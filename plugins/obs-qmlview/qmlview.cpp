#include <QDebug>
#include <QPaintEvent>
#include <QOpenGLFramebufferObject>
#include <QOpenGLPaintDevice>
#include <QFileInfo>

#include "qmlview.h"
#include "renderer.h"

#ifdef Q_OS_WIN
#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#include <stdint.h> // portable: uint64_t   MSVC: __int64

int gettimeofday(struct timeval *tp, struct timezone *tzp)
{
	// Note: some broken versions only have 8 trailing zero's, the correct epoch has 9 trailing zero's
	// This magic number is the number of 100 nanosecond intervals since January 1, 1601 (UTC)
	// until 00:00:00 January 1, 1970
	static const uint64_t EPOCH = ((uint64_t)116444736000000000ULL);

	SYSTEMTIME system_time;
	FILETIME file_time;
	uint64_t time;

	GetSystemTime(&system_time);
	SystemTimeToFileTime(&system_time, &file_time);
	time = ((uint64_t)file_time.dwLowDateTime);
	time += ((uint64_t)file_time.dwHighDateTime) << 32;

	tp->tv_sec = (long)((time - EPOCH) / 10000000L);
	tp->tv_usec = (long)(system_time.wMilliseconds * 1000);
	return 0;
}
#else
#include <sys/time.h>
#endif

OBSQuickview::OBSQuickview(QObject *parent) : QObject(parent)
{
	m_quickView = NULL;
	m_ready = false;
	m_frameLimited = false;
	m_nextFrame = qQNaN();
	m_enabled = false;
	m_delete = false;

	connect(this, SIGNAL(wantLoad()), this, SLOT(doLoad()),
		Qt::QueuedConnection);
	connect(this, SIGNAL(wantUnload()), this, SLOT(doUnload()),
		Qt::QueuedConnection);
	connect(this, SIGNAL(wantResize(quint32, quint32)), this,
		SLOT(doResize(quint32, quint32)), Qt::QueuedConnection);

	m_quickView = new WindowSingleThreaded();

	m_renderCounter = new FrameCounter("QmlView::render");
	m_qmlFrameCounter = new FrameCounter("QmlView::frame");

	connect(this, &OBSQuickview::frameRendered, this,
		&OBSQuickview::qmlFrame, Qt::QueuedConnection);
}

OBSQuickview::~OBSQuickview()
{
	delete m_quickView;

	m_renderCounter->deleteLater();
	m_qmlFrameCounter->deleteLater();
}

void OBSQuickview::loadUrl(QUrl url)
{
	m_ready = false;
	m_source = url;
	if (m_enabled)
		emit wantLoad();
}

quint32 OBSQuickview::width()
{
	return m_quickView->quickWindowSize().width();
}

quint32 OBSQuickview::height()
{
	return m_quickView->quickWindowSize().height();
}

void OBSQuickview::doLoad()
{
	if (m_delete)
		return;
	qDebug() << "Loading URL: " << m_source;
	m_quickView->startQuick(m_source);
}

void OBSQuickview::doUnload()
{
	qDebug() << "Unloading URL: " << m_source;
	m_quickView->unload();
}

void OBSQuickview::resize(quint32 w, quint32 h)
{
	if (m_quickView->quickWindowSize() == QSize(w, h))
		return;
	m_size = QSize(w, h);
	qDebug() << "Resize: " << m_size;
	emit wantResize(w, h);
}

void OBSQuickview::doResize(quint32 w, quint32 h)
{
	if (m_quickView)
		m_quickView->resize(QSize(w, h));
}

void OBSQuickview::obsshow()
{
	m_enabled = true;
	//emit startForced(1000/m_fps);
	if (!m_persistent || !m_quickView->initialised())
		emit wantLoad();
}

void OBSQuickview::obshide()
{
	m_enabled = false;
	if (!m_persistent)
		emit wantUnload();
}

void OBSQuickview::qmlCheckFrame()
{
	// Renderer has shown the last frame, so now we want a new one.
	m_quickView->wantFrame();
	m_qmlFrameCounter->inc();
}

void OBSQuickview::qmlFrame()
{
	if (m_delete)
		return;
	if (frameDue()) {
		qmlCheckFrame();
		frameSynced();
	}
}

void OBSQuickview::renderFrame(gs_effect_t *effect)
{
	if (m_quickView->getTexture()) {
		gs_effect_set_texture(gs_effect_get_param_by_name(effect,
								  "image"),
				      m_quickView->getTexture());
		obs_source_draw(m_quickView->getTexture(), 0, 0, 0, 0, true);
	}

	m_renderCounter->inc();

	emit frameRendered();
}

bool OBSQuickview::frameDue()
{
	if (!m_frameLimited)
		return true;

	// frame time in seconds: (usually 0.xxxxxxx)
	double fdiff = 1.0 / (double)m_fps;

	struct timeval tv;
	gettimeofday(&tv, NULL);
	double thisFrame = (double)tv.tv_sec + ((double)tv.tv_usec * 0.000001);

	if (qIsNaN(m_nextFrame)) {
		m_nextFrame = thisFrame + fdiff;
		return true;
	}

	//qDebug() << "thisFrame:" << thisFrame << " / m_lastFrame:" << m_lastFrame << " / fdiff:" << fdiff << " / m_fps:" << m_fps;
	if (thisFrame >= m_nextFrame)
		return true;

	return false;
}

//quint32 syncLoop = 0;
void OBSQuickview::frameSynced()
{
	//syncLoop++;
	double fdiff = 1.0 / (double)m_fps;

	if (qFuzzyCompare(m_nextFrame, 0.)) {
		struct timeval tv;
		gettimeofday(&tv, NULL);
		m_nextFrame = (double)tv.tv_sec +
			      ((double)tv.tv_usec * 0.000001) + fdiff;
	} else {
		m_nextFrame += fdiff;
	}
}

void OBSQuickview::tick(quint64 seconds)
{
	return;
}
