#pragma once

#include <qobject.h>
#include <qbytearray.h>
#include <qtimer.h>
#include <qset.h>
#include "../common.h"

class BaseMediaTask : public QObject {
	Q_OBJECT
public:
	BaseMediaTask(PhoneType type, QObject *parent = nullptr) : QObject(parent) { Q_UNUSED(type) }
	PhoneType type() { return m_type; }
	virtual void setExpectedDevice(QString device) {}
	virtual QString currentDevice() { return QString(); }

signals:
	void mediaData(char *data, int size, int64_t timestamp, bool isVideo);
	void mediaState(media_status status);

private:
	PhoneType m_type = PhoneType::None;
};
