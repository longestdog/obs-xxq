#include "c-util.h"
#include <qprocess.h>
#include <qmutex.h>
#include <qmap.h>
#include <qset.h>
#include <atomic>
#include <qtimer.h>
#include <qdebug.h>
#include <qdatetime.h>
#include <qcoreapplication.h>

extern "C" {
extern void lock_install();
extern void unlock_install();
}

static QMap<QString, QProcess *> g_process;

void add_process(const char *id, void *process)
{
	g_process.insert(QString(id).toLower(), (QProcess *)process);
}

void remove_process(const char *id)
{
	g_process.remove(QString(id).toLower());
}

bool in_processing(const char *id)
{
	QString device = QString(id).toLower();
	for (auto iter = g_process.begin(); iter != g_process.end(); iter++) {
		if (iter.key().contains(device, Qt::CaseInsensitive)) {
			return true;
		}
	}

	return false;
}

void kill_driver_process(const char *id)
{
	QString device = QString(id).toLower();
	for (auto iter = g_process.begin(); iter != g_process.end(); iter++) {
		if (iter.key().contains(device, Qt::CaseInsensitive)) {
			auto p = g_process.value(device);
			p->kill();
			break;
		}
	}
}

static uint32_t g_count = 0;
void usb_device_reset(const char *serial)
{
	QString s = QString(serial).toLower();
	QMetaObject::invokeMethod(
		qApp,
		[s]() {
			qDebug() << "request device reset: " << s;
			QProcess *process = new QProcess;
			add_process(s.toStdString().c_str(), process);

			QTimer *timer = new QTimer;
			timer->setSingleShot(true);

			QObject::connect(process, QOverload<int, QProcess::ExitStatus>::of(&QProcess::finished),
					 [=](int exitCode, QProcess::ExitStatus exitStatus) {
						 Q_UNUSED(exitCode)
						 Q_UNUSED(exitStatus)

						 qDebug() << "device reset finished: " << s;

						 if (timer->isActive())
							 timer->stop();

						 update_driver_process(false);
						 unlock_install();

						 process->deleteLater();
						 timer->deleteLater();

						 remove_process(s.toStdString().c_str());
					 });

			QObject::connect(timer, &QTimer::timeout, process, [=]() { process->kill(); });

			lock_install();
			update_driver_process(true);

			QStringList p;
			p << s;
			process->start("device-helper.exe", p);

			timer->start(2000);
		},
		Qt::QueuedConnection);
}

void update_driver_process(bool in)
{
	if (in)
		g_count++;
	else
		g_count--;

	if (g_count < 0)
		g_count = 0;
}

bool in_driver_process()
{
	return g_count > 0;
}

static QMutex g_mediaCBMutex;
struct MediaCbInfo {
	media_data_cb cb1;
	device_lost cb2;
	void *cb_data;
};
QMap<QString, MediaCbInfo> g_mediaCBs;
QSet<QString> g_exceptions;

void add_exceptions(const char *serial)
{
	QMutexLocker locker(&g_mediaCBMutex);
	g_exceptions.insert(QString(serial).toLower());
}

void add_media_callback(const char *serial, media_data_cb cb, device_lost lost_cb, void *cb_data)
{
	QMutexLocker locker(&g_mediaCBMutex);
	g_mediaCBs.insert(QString(serial).toLower(), {cb, lost_cb, cb_data});
}

void remove_media_callback(const char *serial)
{
	QMutexLocker locker(&g_mediaCBMutex);
	g_mediaCBs.remove(QString(serial).toLower());
	g_exceptions.remove(QString(serial).toLower());
}

void process_media_data(const char *serial, char *buf, int size)
{
	QMutexLocker locker(&g_mediaCBMutex);
	for (auto iter = g_mediaCBs.begin(); iter != g_mediaCBs.end(); iter++) {
		if (iter.key() == QString(serial).toLower()) {
			auto &info = iter.value();
			info.cb1(buf, size, info.cb_data);
			break;
		}
	}
}

void process_device_lost(const char *serial)
{
	QMutexLocker locker(&g_mediaCBMutex);
	for (auto iter = g_mediaCBs.begin(); iter != g_mediaCBs.end(); iter++) {
		if (iter.key() == QString(serial).toLower()) {
			auto &info = iter.value();
			info.cb2(info.cb_data);
			break;
		}
	}
}

bool media_callback_installed(const char *serial)
{
	QMutexLocker locker(&g_mediaCBMutex);
	return g_mediaCBs.contains(QString(serial).toLower()) && !g_exceptions.contains(QString(serial).toLower());
}

int64_t current_time()
{
	return QDateTime::currentMSecsSinceEpoch();
}
