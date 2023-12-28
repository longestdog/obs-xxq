#include "rtmp-main.h"
#include <map>
#include <signal.h>
#include <iostream>

#include "Util/MD5.h"
#include "Util/File.h"
#include "Util/logger.h"
#include "Util/SSLBox.h"
#include "Util/onceToken.h"
#include "Network/TcpServer.h"
#include "Poller/EventPoller.h"

#include "Common/config.h"
#include "Rtsp/UDPServer.h"
#include "Rtsp/RtspSession.h"
#include "Rtmp/RtmpSession.h"
#include "Shell/ShellSession.h"
#include "Rtmp/FlvMuxer.h"
#include "Player/PlayerProxy.h"
#include "Http/WebSocketSession.h"

#include "../../../common.h"

using namespace std;
using namespace toolkit;
using namespace mediakit;

static std::function<void(media_status)> g_state_cb;
static std::function<void(uint8_t *, size_t, int64_t, bool)> g_data_cb;

namespace rtmp {
void send_status(media_status status)
{
	if (!g_state_cb)
		return;

	g_state_cb(status);
}

void send_data(uint8_t *data, size_t len, int64_t ts, bool is_video)
{
	if (!g_data_cb)
		return;

	g_data_cb(data, len, ts, is_video);
}
}

void initEventListener()
{
	static onceToken s_token(
		[]() {
			//监听rtsp/rtmp推流事件，返回结果告知是否有推流权限
			NoticeCenter::Instance().addListener(nullptr, Broadcast::kBroadcastMediaPublish, [](BroadcastMediaPublishArgs) {
				DebugL << "推流鉴权：" << args._schema << " " << args._vhost << " " << args._app << " " << args._streamid << " "
				       << args._param_strs;
				invoker("", false, false); //鉴权成功

				rtmp::send_status(media_status::Media_START);
			});

			//监听播放或推流结束时消耗流量事件
			NoticeCenter::Instance().addListener(nullptr, Broadcast::kBroadcastFlowReport, [](BroadcastFlowReportArgs) {
				DebugL << "播放器(推流器)断开连接事件:" << args._schema << " " << args._vhost << " " << args._app << " " << args._streamid
				       << " " << args._param_strs << "\r\n使用流量:" << totalBytes << " bytes,连接时长:" << totalDuration << "秒";

				rtmp::send_status(media_status::Media_STOP);
			});
		},
		nullptr);
}

static std::shared_ptr<TcpServer> rtmpSrv = nullptr;
void start_rtmp_server(std::function<void(media_status)> state_cb, std::function<void(uint8_t *, size_t, int64_t, bool)> data_cb)
{
	if (rtmpSrv)
		return;

	Logger::Instance().add(std::make_shared<ConsoleChannel>());
	Logger::Instance().add(std::make_shared<FileChannel>());
	Logger::Instance().setWriter(std::make_shared<AsyncLogWriter>());

	initEventListener();

	g_state_cb = state_cb;
	g_data_cb = data_cb;
	rtmpSrv = std::make_shared<TcpServer>();
	rtmpSrv->start<RtmpSession>(1935); //默认1935
}

void stop_rtmp_server()
{
	if (rtmpSrv) {
		rtmpSrv.reset();
	}

	g_state_cb = nullptr;
	g_data_cb = nullptr;
}
