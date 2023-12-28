#pragma once

#include <qobject.h>
#include <qpointer.h>

#include "../common.h"
#include "../ipc.h"

class QTcpSocket;
class BaseMediaTask;
class MediaSource : public QObject {
	Q_OBJECT
public:
	MediaSource(QObject *parent = nullptr);
	~MediaSource();
	void connectToMediaTarget(QString name);
	void setCurrentDevice(PhoneType type, QString deviceId);
	PhoneType sourceType();

	Q_INVOKABLE void mediaTargetLost();

private:
	bool m_emitError = false;
	struct IPCClient *m_ipcClient = nullptr;
	QPointer<BaseMediaTask> m_mediaTask = nullptr;
	PhoneType m_phoneType = PhoneType::None;
};
