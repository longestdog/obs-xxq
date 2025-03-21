#include "application.h"
#include <QtCore/QCoreApplication>
#include <QFile>
#include <QTextStream>
#include <QDateTime>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>

QString dirPath;
QString GetAppDataDirPath()
{
	QString path = QStandardPaths::writableLocation(QStandardPaths::AppLocalDataLocation) + "/..";
	QDir    dir(path);
	if (!dir.exists("yuerlive")) {
		dir.mkdir("yuerlive");
	}
	return path + "/yuerlive";
}

void cleanUpOldLogFiles(const QString &logDirPath) {
	// 获取当前日期，计算7天前的日期  
	const QDate currentDate = QDate::currentDate();
	const QDate cutoffDate = currentDate.addDays(-4);

	QDir logDir(logDirPath);
	if (!logDir.exists()) {
		qWarning() << "日志目录不存在：" << logDirPath;
		return;
	}

	const QFileInfoList files = logDir.entryInfoList(QStringList("*.log"), QDir::Files);

	for (const QFileInfo &fileInfo : files) {
		const QString fileName = fileInfo.fileName();
		const int underscorePos = fileName.indexOf('_');
		if (underscorePos == -1) {
			continue;
		}
		const QString dateStr = fileName.mid(underscorePos + 1, 8); // 提取yyyymmdd部分  

		// 解析日期  
		QDate fileDate = QDate::fromString(dateStr, "yyyyMMdd");
		if (fileDate.isValid()) {
			if (fileDate < cutoffDate) {
				const QString filePath = fileInfo.filePath();
				if (QFile::remove(filePath)) {
					qDebug() << "成功删除旧日志文件：" << filePath;
				}
				else {
					qWarning() << "失败删除旧日志文件：" << filePath;
				}
			}
		}
		else {
			qWarning() << "无效日期格式的文件名：" << fileName;
		}
	}
}

void customLogHandler(QtMsgType type, const QMessageLogContext &context, const QString &msg) {
	Q_UNUSED(context);
	QDir dir(dirPath);
	if (!dir.exists(dirPath)) {
		dir.mkpath(dirPath);
	}

	QString timestamp = QDateTime::currentDateTime().toString("yyyyMMdd_hh_mm_ss");
	QString logFileName = QString("media-backend_%1.log").arg(timestamp);
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
	QApplication::setAttribute(Qt::AA_EnableHighDpiScaling);
	QApplication::setAttribute(Qt::AA_UseHighDpiPixmaps);
	QApplication::setHighDpiScaleFactorRoundingPolicy(Qt::HighDpiScaleFactorRoundingPolicy::PassThrough);

	Application app(argc, argv);
	dirPath = GetAppDataDirPath() + "/logs/mobile-media-backend/";
	qInstallMessageHandler(customLogHandler);
	cleanUpOldLogFiles(dirPath);

	return app.exec();
}
