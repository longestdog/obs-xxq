#pragma once

#include <QObject>
#include <QVector>
#include "qmlsourcebase.h"

class GiftTV : public QmlSourceBase {
	Q_OBJECT
public:
	DEFINE_PROPERTY(QString, giftArray);

public:
	GiftTV(QObject *parent = nullptr);
	virtual ~GiftTV();
	static void default(obs_data_t *settings);
};
