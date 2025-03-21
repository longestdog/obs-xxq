#include <qapplication.h>
#include <qthread.h>
#include "DriverHelper.h"
#include "InformationWidget.h"
#include <libusb-1.0/libusb.h>
#include <QDir>
#include <QStandardPaths>
#include <QDateTime>
#include <QDebug>
QString dirPath;
static void resetUsbDevice(char *t_serial)
{
	libusb_init(NULL);

	libusb_device **list;
	auto devsCount = libusb_get_device_list(NULL, &list);
	for (size_t i = 0; i < devsCount; i++) {
		libusb_device_descriptor desc;
		if (libusb_get_device_descriptor(list[i], &desc) != LIBUSB_SUCCESS)
			continue;

		libusb_device_handle *handle = nullptr;
		if (libusb_open(list[i], &handle) == LIBUSB_SUCCESS) {
			char serial[40] = {0};
			if (libusb_get_string_descriptor_ascii(handle, desc.iSerialNumber, (uint8_t *)serial, 40) <= 0) {
				libusb_close(handle);
				continue;
			}

			if (QString(t_serial).toLower() == QString(serial).toLower()) {
				libusb_claim_interface(handle, 0);
				libusb_reset_device(handle);
				libusb_close(handle);
				break;
			}

			libusb_close(handle);
		}
	}
	libusb_free_device_list(list, 1);

	libusb_exit(NULL);
}

QString GetAppDataDirPath()
{
	QString path = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation);
	QDir    dir(path);
	if (!dir.exists("yuerlive")) {
		dir.mkdir("yuerlive");
	}
	return path + "/yuerlive";
}

void customLogHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
	Q_UNUSED(context);
	QDir dir(dirPath);
	if (!dir.exists(dirPath)) {
		dir.mkpath(dirPath);
	}

	QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hh_mm_ss");
	QString logFileName = QString("DeviceHelper_%1.log").arg(timestamp);
	QFile logFile(dirPath + logFileName);
	if (!logFile.open(QIODevice::WriteOnly | QIODevice::Append | QIODevice::Text)) {
		qWarning() << "Failed to open log file for writing.";
		return;
	}

	QTextStream stream(&logFile);
	QString logLevel;
	switch (type) {
	case QtDebugMsg:   logLevel = "DEBUG";    break;
	case QtInfoMsg:    logLevel = "INFO";     break;
	case QtWarningMsg: logLevel = "WARNING";  break;
	case QtCriticalMsg: logLevel = "CRITICAL"; break;
	case QtFatalMsg:   logLevel = "FATAL";    break;
	}
	stream << QString("[%1] [%2] %3\n").arg(timestamp, logLevel, msg);
	logFile.close();
}
int main(int argc, char *argv[])
{
	dirPath = GetAppDataDirPath() + "/logs/mobile-media-backend/";
	qInstallMessageHandler(customLogHandler);
	if (argc == 5) {
		QGuiApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
		QGuiApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);

		bool isAppleDevice = QString(argv[1]).toInt() == 1;
		int vid = QString(argv[2]).toInt();
		int pid = QString(argv[3]).toInt();
		QString path = QString(argv[4]);

		QApplication app(argc, argv);
		app.setQuitOnLastWindowClosed(false);
		app.setApplicationName("yuerlive");

		QThread thread;
		QObject::connect(&app, &QApplication::aboutToQuit, &thread, &QThread::quit);
		thread.start();

		DriverHelper helper;
		helper.moveToThread(&thread);

		InformationWidget widget;
		QObject::connect(&helper, &DriverHelper::installProgress, &widget, &InformationWidget::onInstallStatus);
		QObject::connect(&helper, &DriverHelper::installError, &widget, &InformationWidget::onInstallError);
		QObject::connect(&helper, &DriverHelper::changeTip, &widget, &InformationWidget::onChangeTip);
		QObject::connect(
			&helper, &DriverHelper::complete, &app,
			[&thread](int status) {
				thread.quit();
				thread.wait();
				qApp->exit(status);
			},
			Qt::QueuedConnection);
		
		QMetaObject::invokeMethod(&helper, "checkInstall", Q_ARG(bool, isAppleDevice), Q_ARG(int, vid), Q_ARG(int, pid), Q_ARG(QString, path));
		
		return app.exec();
	} else if (argc == 2) {
		resetUsbDevice(argv[1]);
	}

	return 0;
}
