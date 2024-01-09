#include "stream.h"

#include <assert.h>
#include "adb-config.h"
#include "util/buffer_util.h"
#include "util/log.h"

#define BUFSIZE 0x10000

#define HEADER_SIZE 12
#define NO_PTS UINT64_C(-1)

static bool stream_recv_packet(struct stream *stream)
{
	// The video stream contains raw packets, without time information. When we
	// record, we retrieve the timestamps separately, from a "meta" header
	// added by the server before each raw packet.
	//
	// The "meta" header length is 12 bytes:
	// [. . . . . . . .|. . . .]. . . . . . . . . . . . . . . ...
	//  <-------------> <-----> <-----------------------------...
	//        PTS        packet        raw packet
	//                    size
	//
	// It is followed by <packet_size> bytes containing the packet/frame.

	uint8_t header[HEADER_SIZE];
	long long r = net_recv_all(stream->socket, header, HEADER_SIZE);
	if (r < HEADER_SIZE) {
		return false;
	}

	uint64_t pts = buffer_read64be(header);
	uint32_t len = buffer_read32be(&header[8]);
	assert(pts == NO_PTS || (pts & 0x8000000000000000) == 0);
	assert(len);

	uint8_t *buffer = (uint8_t *)calloc(1, len);
	r = net_recv_all(stream->socket, buffer, len);
	if (r < 0 || ((uint32_t)r) < len) {
		free(buffer);
		return false;
	}

	uint64_t packet_pts = pts != NO_PTS ? (int64_t)pts : ((int64_t)UINT64_C(0x8000000000000000));
	bool is_pps = packet_pts == ((int64_t)UINT64_C(0x8000000000000000));

	stream->data_cb(buffer, len, packet_pts, true);
	free(buffer);
	return true;
}

static void *run_stream(void *data)
{
	struct stream *stream = (struct stream *)data;
	stream->state_cb(media_status::Media_START);
	while (!stream->stop) {
		bool ok = stream_recv_packet(stream);
		if (!ok) {
			// end of stream
			break;
		}
	}
	stream->state_cb(media_status::Media_STOP);

	LOGD("End of frames");
	return NULL;
}

void stream_init(struct stream *stream, socket_t socket)
{
	stream->socket = socket;
}

bool stream_start(struct stream *stream)
{
	LOGD("Starting stream thread");

	int perr = pthread_create(&stream->thread, NULL, run_stream, stream);
	if (perr != 0) {
		LOGC("Could not start stream thread");
		return false;
	}
	return true;
}

void stream_stop(struct stream *stream)
{
	stream->stop = true;
	stream_join(stream);
	memset(stream, 0, sizeof(struct stream));
}

void stream_join(struct stream *stream)
{
	pthread_join(stream->thread, NULL);
}
