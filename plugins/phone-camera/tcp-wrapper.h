#pragma once

#include "common.h"
#include <qjsondocument.h>
#include <qjsonobject.h>
#include <qtcpsocket.h>
#include "xxq-tcp.h"

class TcpSocketWrapper : public QObject {
	Q_OBJECT
public:
	TcpSocketWrapper(QTcpSocket *socket) : m_socket(socket)
	{
		setParent(socket);
		connect(socket, &QTcpSocket::readyRead, this, [=]() {
			m_socketDataCache.append(socket->readAll());
			while (m_socketDataCache.size() > 4) {
				int payloadSize = 0;
				memcpy(&payloadSize, m_socketDataCache.data(), 4);
				if (m_socketDataCache.size() < payloadSize + 4)
					break;

				m_socketDataCache.remove(0, 4);
				QJsonDocument doc = QJsonDocument::fromJson(m_socketDataCache.left(payloadSize));
				m_socketDataCache.remove(0, payloadSize);
				if (!doc.isNull()) {
					auto obj = doc.object();
					emit msgReceived(obj);
				}
			}
		});
		connect(socket, &QTcpSocket::disconnected, this, [=]() { socket->deleteLater(); });
	}

	~TcpSocketWrapper() { qDebug() << "TcpSocketWrapper destroyed."; }

	QTcpSocket *socket() { return m_socket; }
	void sendMsg(const QJsonDocument &msg)
	{
		if (m_socket->state() != QTcpSocket::ConnectedState)
			return;

		auto data = msg.toJson(QJsonDocument::Compact);
		int dataSize = data.size();

		QByteArray ret;
		ret.fill(0, 4);
		memcpy(ret.data(), &dataSize, sizeof(int));
		data.prepend(ret);

		m_socket->write(data);
	}

signals:
	void msgReceived(QJsonObject msg);

private:
	QByteArray m_socketDataCache;
	QTcpSocket *m_socket;
};

class MediaDataServer : public TcpServer, public SocketEventListener {
	Q_OBJECT
public:
	MediaDataServer(QObject *parent = nullptr) : TcpServer(parent) { setSocketEventListener(this); }

	~MediaDataServer()
	{
		setSocketEventListener(nullptr);
		stop();
	}

	virtual void onClientData(evutil_socket_t fd, char *data, int size)
	{
		if (fd != m_connectFd)
			return;

		m_mediaData.append(data, size);
		while (m_mediaData.size() > sizeof(av_packet_info)) {
			av_packet_info header = {0};
			memcpy(&header, m_mediaData.data(), sizeof(av_packet_info));
			if (m_mediaData.size() < header.size + sizeof(av_packet_info))
				break;

			m_mediaData.remove(0, sizeof(av_packet_info));
			switch (header.type) {
			case av_packet_type::FFM_PACKET_VIDEO:
				emit mediaData((uint8_t *)m_mediaData.data(), header.size, header.pts, true);
				break;
			case av_packet_type::FFM_PACKET_AUDIO:
				emit mediaData((uint8_t *)m_mediaData.data(), header.size, header.pts, false);
				break;
			case av_packet_type::FFM_MEDIA_VIDEO_INFO: {
				media_video_info vInfo;
				memcpy(vInfo.video_extra, m_mediaData.data(), header.size);
				vInfo.video_extra_len = header.size;
				emit mediaVideoInfo(vInfo);
			} break;
			case av_packet_type::FFM_MEDIA_AUDIO_INFO: {
				media_audio_info aInfo;
				memcpy(&aInfo, m_mediaData.data(), sizeof(media_audio_info));
				emit mediaAudioInfo(aInfo);
			} break;
			case av_packet_type::FFM_MIRROR_STATUS: {
				int status = -1;
				memcpy(&status, m_mediaData.data(), sizeof(int));
				emit mediaStatus((media_status)status);
			} break;
			default:
				break;
			}
			m_mediaData.remove(0, header.size);
		}
	}

	virtual bool onNewConnection(evutil_socket_t fd)
	{
		if (m_connectFd == 0) {
			m_connectFd = fd;
			m_mediaData.clear();
			return true;
		} else
			return false;
	}

	virtual void onConnectionLost(evutil_socket_t fd)
	{
		if (m_connectFd == fd) {
			m_connectFd = 0;
		}
	}

signals:
	void mediaData(uint8_t *data, size_t size, int64_t pts, bool isVideo);
	void mediaVideoInfo(const media_video_info &info);
	void mediaAudioInfo(const media_audio_info &info);
	void mediaStatus(media_status status);

private:
	QByteArray m_mediaData;
	evutil_socket_t m_connectFd = 0;
};
