#include "tool.h"
#include <qstandardpaths.h>
#include <qmap.h>
#include <qfile.h>
#include <qimage.h>
#include <qpainter.h>
#include <qimagewriter.h>
#include <qdatetime.h>
#include "common.h"
#include "qrcode/qr_to_png.h"
#include <netfw.h>
#include <qprocess.h>
#include <qregularexpression.h>

static QMap<PhoneType, QString> g_resource_map = {{PhoneType::iOSAirplay, "iOSwuxian"},
						  {PhoneType::iOS, "iOSyouxian"},
						  {PhoneType::AndroidAOAScreen, "AndroidAOA"},
						  {PhoneType::AndroidAOACamera, "AndroidAOA"},
						  {PhoneType::AndroidRtmp, "Androidwuxian"},
						  {PhoneType::AndroidADB, "AndroidADB"},
						  {PhoneType::iOSRtmp, "iOSsaoma"},
						  {PhoneType::iOSCamera, "iOSyouxian"}};
static QMap<PhoneType, QMap<media_status, QString>> g_status_img;

#include <QJsonArray>
#include <QJsonDocument>
#include <QHostAddress>
#include <QNetworkInterface>
#include <QNetworkAddressEntry>
static QString streamUrlImage()
{
	QJsonArray arr;
	for (const QNetworkInterface &address : QNetworkInterface::allInterfaces()) {
		if (address.humanReadableName().contains("virtual", Qt::CaseInsensitive) || address.humanReadableName().contains("vmware", Qt::CaseInsensitive))
			continue;

		if (address.type() != QNetworkInterface::Virtual && address.type() != QNetworkInterface::Loopback &&
		    address.type() != QNetworkInterface::Unknown) {
			QList<QNetworkAddressEntry> cc = address.addressEntries();
			for (auto iter = cc.begin(); iter != cc.end(); iter++) {
				QNetworkAddressEntry &entry = *iter;
				QHostAddress ad = entry.ip();
				if (ad.protocol() == QAbstractSocket::IPv4Protocol) {
					auto ipstr = ad.toString();
					if (ipstr.startsWith("192") || ipstr.startsWith("172") || ipstr.startsWith("10"))
						arr.append(ad.toString());
				}
			}
		}
	}

	QJsonDocument jd(arr);
	QString s = jd.toJson(QJsonDocument::Compact);

	QString pngPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation) + "/xxq-stream-qrcode.png";
	auto exampleQrPng1 = QrToPng(pngPath, 200, 3, s.toStdString(), qrcodegen::QrCode::Ecc::MEDIUM);
	exampleQrPng1.writeToPNG();
	return pngPath;
}

void dumpResourceImgs()
{
	auto checkWrite = [](QString path) {
		QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
		auto p = QString("%1/%2").arg(tempPath).arg(path);
		if (QFile::exists(p))
			return;

		auto src = QString(":/%1").arg(path);
		QFile::copy(src, p);
	};

	for (auto iter = g_resource_map.begin(); iter != g_resource_map.end(); iter++) {
		auto str = iter.value();
		checkWrite(str + ".png");
		checkWrite(str + "zhong.png");
		checkWrite(str + "shibai.png");
	}
}

gs_texture_t *prepareStateTexture(PhoneType type, media_status status)
{
	auto prefix = g_resource_map.value(type);
	auto &imgs = g_status_img[type];
	if (!imgs.contains(status)) {
		QString suffix;
		if (status == media_status::Media_STOP || status == media_status::Media_INIT)
			suffix = "";
		else if (status == media_status::Media_START)
			suffix = "zhong";
		else if (status == media_status::Media_DEVICE_LOST)
			suffix = "shibai";

		QString tempPath = QStandardPaths::writableLocation(QStandardPaths::TempLocation);
		auto imgPath = QString("%1/%2%3.png").arg(tempPath).arg(prefix).arg(suffix);
		if (type == PhoneType::AndroidRtmp || type == PhoneType::iOSRtmp) {
			auto qrcode = streamUrlImage();
			if (!qrcode.isEmpty()) {
				auto writeQRCodeImage = [=](QString srcFile, QString dstFile) {
					QImage image(srcFile);
					image = image.convertToFormat(QImage::Format_RGBA8888);
					QPainter p(&image);
					p.drawImage(QRect(174, 223, 260, 260), QImage(qrcode));
					QImageWriter writer(dstFile);
					writer.write(image);
					return dstFile;
				};

				imgPath = writeQRCodeImage(imgPath, QString("%1/%2%3_bak.png").arg(tempPath).arg(prefix).arg(suffix));
			}
		}

		imgs[status] = imgPath;
	}

	return gs_texture_create_from_file(imgs[status].toUtf8().data());
}

// Instantiate INetFwPolicy2
HRESULT WFCOMInitialize(INetFwPolicy2 **ppNetFwPolicy2)
{
	HRESULT hr = S_OK;

	hr = CoCreateInstance(__uuidof(NetFwPolicy2), NULL, CLSCTX_INPROC_SERVER, __uuidof(INetFwPolicy2), (void **)ppNetFwPolicy2);

	if (FAILED(hr)) {
		printf("CoCreateInstance for INetFwPolicy2 failed: 0x%08lx\n", hr);
		goto Cleanup;
	}

Cleanup:
	return hr;
}

void closeWindowsFireWall()
{
	HRESULT hrComInit = S_OK;
	HRESULT hr = S_OK;

	INetFwPolicy2 *pNetFwPolicy2 = NULL;

	// Initialize COM.
	hrComInit = CoInitializeEx(0, COINIT_APARTMENTTHREADED);

	// Ignore RPC_E_CHANGED_MODE; this just means that COM has already been
	// initialized with a different mode. Since we don't care what the mode is,
	// we'll just use the existing mode.
	if (hrComInit != RPC_E_CHANGED_MODE) {
		if (FAILED(hrComInit)) {
			printf("CoInitializeEx failed: 0x%08lx\n", hrComInit);
			goto Cleanup;
		}
	}

	// Retrieve INetFwPolicy2
	hr = WFCOMInitialize(&pNetFwPolicy2);
	if (FAILED(hr)) {
		goto Cleanup;
	}

	// Disable Windows Firewall for the Domain profile
	hr = pNetFwPolicy2->put_FirewallEnabled(NET_FW_PROFILE2_DOMAIN, FALSE);
	if (FAILED(hr)) {
		printf("put_FirewallEnabled failed for Domain: 0x%08lx\n", hr);
	}

	// Disable Windows Firewall for the Private profile
	hr = pNetFwPolicy2->put_FirewallEnabled(NET_FW_PROFILE2_PRIVATE, FALSE);
	if (FAILED(hr)) {
		printf("put_FirewallEnabled failed for Private: 0x%08lx\n", hr);
	}

	// Disable Windows Firewall for the Public profile
	hr = pNetFwPolicy2->put_FirewallEnabled(NET_FW_PROFILE2_PUBLIC, FALSE);
	if (FAILED(hr)) {
		printf("put_FirewallEnabled failed for Public: 0x%08lx\n", hr);
	}

Cleanup:

	// Release INetFwPolicy2
	if (pNetFwPolicy2 != NULL) {
		pNetFwPolicy2->Release();
	}

	// Uninitialize COM.
	if (SUCCEEDED(hrComInit)) {
		CoUninitialize();
	}
}

int findProcessListeningToPort(int port)
{
	QString netstatOutput;
	{
		QProcess process;
		process.start("netstat -ano -p tcp");
		process.waitForFinished();
		netstatOutput = process.readAllStandardOutput();
	}
	QRegularExpression processFinder;
	{
		const auto pattern = QStringLiteral(R"(TCP[^:]+:%1.+LISTENING\s+(\d+))").arg(port);
		processFinder.setPattern(pattern);
	}
	const auto processInfo = processFinder.match(netstatOutput);
	if (processInfo.hasMatch()) {
		const auto processId = processInfo.captured(1).toInt();
		return processId;
	}
	return 0;
}
