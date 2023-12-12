#include "LicenseRequester.h"
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <qnetworkaccessmanager.h>
#include <qnetworkreply.h>
#include <qmessageauthenticationcode.h>
#include <qdatetime.h>
#include <qeventloop.h>
#include <qtimer.h>
#include <qdir.h>

#include <util/base.h>

#include "bef_effect_ai_api.h"
#include "bef_effect_ai_online_license.h"
#include "bef_effect_ai_version.h"

#include "BEResourceProvider.h"

static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZ"
"abcdefghijklmnopqrstuvwxyz"
"0123456789+/";

static inline bool is_base64(unsigned char c)
{
	return (isalnum(c) || (c == '+') || (c == '/'));
}

std::vector<unsigned char> base64_decode(std::string const &encoded_string)
{
	int in_len = encoded_string.size();
	int i = 0;
	int j = 0;
	int in_ = 0;
	unsigned char char_array_4[4], char_array_3[3];
	std::vector<unsigned char> ret;

	while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
		char_array_4[i++] = encoded_string[in_];
		in_++;
		if (i == 4) {
			for (i = 0; i < 4; i++)
				char_array_4[i] = base64_chars.find(char_array_4[i]);

			char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
			char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
			char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

			for (i = 0; (i < 3); i++)
				ret.push_back(char_array_3[i]);
			i = 0;
		}
	}

	if (i) {
		for (j = i; j < 4; j++)
			char_array_4[j] = 0;

		for (j = 0; j < 4; j++)
			char_array_4[j] = base64_chars.find(char_array_4[j]);

		char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
		char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
		char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];

		for (j = 0; (j < i - 1); j++)
			ret.push_back(char_array_3[j]);
	}

	return ret;
}

void check_update_license()
{
	auto licenseDir = BEResourceProvider::getInstance().licenseDirPath();
	auto licenseFile = BEResourceProvider::getInstance().licenseDirPath() + "license.bag";
	auto ret = bef_effect_ai_check_license_base(licenseFile.c_str(), true);
	QFileInfo finfo(licenseFile.c_str());
	if (ret != BEF_RESULT_SUC || !finfo.exists() || finfo.size() == 0) {
		char *authMsg = nullptr;
		int authMsgLen = 0;
		bef_effect_ai_get_online_license_auth_msg(&authMsg, &authMsgLen);

		QString key = "biz_license_tool_test_key1dc40f68d29c421e977b01d61b13f5e0";
		QString screct = "9a671942ec36a5e7d3ff21f96aca2b79";
		/*	QString key = "biz_license_tool_test_key51d1efd97aa2455b8f2e0a954033b568";
			QString screct = "04f350634cdddf77d5d7aaec42769a88";*/
		auto nonce = rand();
		auto timestamp = QDateTime::currentSecsSinceEpoch();
		QJsonObject obj;
		obj["key"] = key;
		obj["authMsg"] = authMsg;
		obj["nonce"] = nonce;
		obj["timestamp"] = timestamp;
		auto func = [](QByteArray data, const char *const digits = "0123456789ABCDEF") {
			QString str;

			for (unsigned char gap = 0; gap < data.size();) {
				unsigned char beg = data[gap];
				++gap;
				str.append(digits[beg >> 4]);
				str.append(digits[beg & 15]);
			}
			return str;
		};

		QString temp = QString("%1%2%3%4").arg(key).arg(nonce).arg(timestamp).arg(authMsg);
		auto data = QMessageAuthenticationCode::hash(temp.toUtf8(), screct.toUtf8(), QCryptographicHash::Sha256);
		obj["digest"] = func(data);

		QNetworkAccessManager *manager = new QNetworkAccessManager;
		QNetworkRequest req;
		req.setUrl(QUrl("https://cv-tob.bytedance.com/v1/api/sdk/tob_license/getlicense"));
		req.setRawHeader("Content-Type", "application/json");
		req.setRawHeader("cache-control", "no-cache");
		QJsonDocument jd(obj);
		auto reply = manager->post(req, jd.toJson());
		QEventLoop loop;
		QTimer t;
		QObject::connect(&t, &QTimer::timeout, &loop, &QEventLoop::quit);
		t.setSingleShot(true);
		t.start(3000);
		QObject::connect(reply, &QNetworkReply::finished, &loop, [=, &loop]() {
			QJsonDocument doc = QJsonDocument::fromJson(reply->readAll());
			qDebug() << "license doc " << doc;
			auto o = doc.object();
			if (!o.isEmpty()) {
				QDir dir(QString::fromStdString(licenseDir));
				if (!dir.exists())
					dir.mkpath(QString::fromStdString(licenseDir));
				QString base64Str = o["data"].toString();
				std::vector<uint8_t> lic_b = base64_decode(base64Str.toStdString());
				QFile f(licenseFile.c_str());
				f.open(QFile::ReadWrite | QFile::Truncate);
				f.write((const char *)lic_b.data(), lic_b.size());
				f.close();
			}
			reply->deleteLater();
			manager->deleteLater();

			loop.quit();
		});
		loop.exec(QEventLoop::ExcludeUserInputEvents);

		bef_effect_ai_free_online_license_auth_msg(authMsg);
	}

	std::string str;
	str.resize(10);
	bef_effect_ai_get_version((char *)str.c_str(), 10);
	blog(LOG_INFO, "EffectDemoVersion %s\n", str.c_str());
}
