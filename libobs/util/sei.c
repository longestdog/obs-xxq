#include "sei.h"
#include <string.h>
#include "util/bmem.h"
#include "util/base.h"

#define UUID_SIZE 16
static unsigned char start_code[] = {0x00, 0x00, 0x00, 0x01};
static unsigned char uuid[] = {0x54, 0x80, 0x83, 0x97, 0xf0, 0x23, 0x47, 0x4b, 0xb7, 0xf7, 0x4f, 0x32, 0xb5, 0x4e, 0x06, 0xac};

size_t avc_sei(unsigned char *out, const unsigned char *in, int in_len)
{
	size_t sei_payload_size = in_len + UUID_SIZE;
	size_t sei_size = 1 + 1 + (sei_payload_size / 0xFF + (sei_payload_size % 0xFF != 0 ? 1 : 0)) + sei_payload_size;
	size_t tail_size = 2;
	if (sei_size % 2 == 1) {
		tail_size -= 1;
	}
	sei_size += tail_size;
	unsigned char *data = out;
	memcpy(data, start_code, sizeof(unsigned int));

	data += sizeof(start_code);

	unsigned char *sei = data;
	*data++ = 6; //SEI
	*data++ = 5; //unregister
	while (1) {
		*data++ = (sei_payload_size >= 0xFF ? 0xFF : (char)sei_payload_size);
		if (sei_payload_size < 0xFF)
			break;
		sei_payload_size -= 0xFF;
	}

	memcpy(data, uuid, UUID_SIZE);
	data += UUID_SIZE;
	memcpy(data, in, in_len);
	data += in_len;

	if (tail_size == 1) {
		*data = 0x80;
	} else if (tail_size == 2) {
		*data++ = 0x00;
		*data++ = 0x80;
	}

	return sei_size + 4;
}

struct bit_stream {
	uint8_t *fifo;
	uint32_t byte_alloc;
	uint32_t byte_occupancy;
	uint32_t partial_byte_bits;
	uint8_t partial_byte;
};

static void bitstream_reset_bits(struct bit_stream *bs)
{
	if (!bs)
		return;

	bs->partial_byte_bits = bs->byte_occupancy = 0;
	bs->partial_byte = 0;
}

#define MIN_FIFO_SIZE 1000
static struct bit_stream *bitstream_create()
{
	struct bit_stream *bs = bzalloc(sizeof(struct bit_stream));
	bs->fifo = bzalloc(MIN_FIFO_SIZE);
	bs->byte_alloc = MIN_FIFO_SIZE;
	bitstream_reset_bits(bs);
	return bs;
}

static void bitstream_destroy(struct bit_stream **bs)
{
	if (!*bs)
		return;

	bfree((*bs)->fifo);
	bfree(*bs);
	*bs = NULL;
}

static void bitstream_pushback(struct bit_stream *bs, uint8_t val)
{
	if (!bs || !bs->fifo)
		return;

	if (bs->byte_occupancy >= bs->byte_alloc) {
		/** reallocate buffer with doubled size */
		uint8_t *temp = bzalloc(bs->byte_alloc * 2);
		if (temp) {
			memcpy(temp, bs->fifo, bs->byte_occupancy);
			bfree(bs->fifo);
			bs->fifo = temp;
			bs->byte_alloc *= 2;
		} else {
			blog(LOG_ERROR, "Unable to realloc bitstream buffer");
			return;
		}
	}
	bs->fifo[bs->byte_occupancy++] = val;
}

static void bitstream_write(struct bit_stream *bs, uint32_t val, uint32_t numBits)
{
	if (!bs)
		return;

	uint32_t totalPartialBits = bs->partial_byte_bits + numBits;
	uint32_t nextPartialBits = totalPartialBits & 7;
	uint8_t nextHeldByte = val << (8 - nextPartialBits);
	uint32_t writeBytes = totalPartialBits >> 3;

	if (writeBytes) {
		/* topword aligns bs->partial_byte with the msb of val */
		uint32_t topword = (numBits - nextPartialBits) & ~7;
#if USING_FTRAPV
		uint32_t write_bits = (topword < 32 ? bs->partial_byte << topword : 0) | (val >> nextPartialBits);
#else
		uint32_t write_bits = (bs->partial_byte << topword) | (val >> nextPartialBits);
#endif

		switch (writeBytes) {
		case 4:
			bitstream_pushback(bs, write_bits >> 24); // fall-through
		case 3:
			bitstream_pushback(bs, write_bits >> 16); // fall-through
		case 2:
			bitstream_pushback(bs, write_bits >> 8); // fall-through
		case 1:
			bitstream_pushback(bs, write_bits);
		}

		bs->partial_byte = nextHeldByte;
		bs->partial_byte_bits = nextPartialBits;
	} else {
		bs->partial_byte |= nextHeldByte;
		bs->partial_byte_bits = nextPartialBits;
	}
}

static void bitstream_write_byte(struct bit_stream *bs, uint32_t val)
{
	if (!bs)
		return;

	bitstream_pushback(bs, val);
}

static void bitstream_write_align_one(struct bit_stream *bs)
{
	if (!bs)
		return;

	uint32_t numBits = (8 - bs->partial_byte_bits) & 0x7;

	bitstream_write(bs, (1 << numBits) - 1, numBits);
}

static void bitstream_write_align_zero(struct bit_stream *bs)
{
	if (!bs)
		return;

	if (bs->partial_byte_bits) {
		bitstream_pushback(bs, bs->partial_byte);
		bs->partial_byte = 0;
		bs->partial_byte_bits = 0;
	}
}

static void bitstream_write_byte_alignment(struct bit_stream *bs)
{
	if (!bs)
		return;

	bitstream_write(bs, 1, 1);
	bitstream_write_align_zero(bs);
}

static uint32_t bitstream_get_number_of_written_bytes(struct bit_stream *bs)
{
	if (!bs)
		return 0;

	return bs->byte_occupancy;
}

static uint32_t bitstream_get_number_of_written_bits(struct bit_stream *bs)
{
	if (!bs)
		return 0;

	return bs->byte_occupancy * 8 + bs->partial_byte_bits;
}

static const uint8_t *bitstream_get_FIFO(struct bit_stream *bs)
{
	if (!bs)
		return NULL;

	return bs->fifo;
}

typedef enum
{
    NAL_UNIT_CODED_SLICE_TRAIL_N = 0,
    NAL_UNIT_CODED_SLICE_TRAIL_R,
    NAL_UNIT_CODED_SLICE_TSA_N,
    NAL_UNIT_CODED_SLICE_TLA_R,
    NAL_UNIT_CODED_SLICE_STSA_N,
    NAL_UNIT_CODED_SLICE_STSA_R,
    NAL_UNIT_CODED_SLICE_RADL_N,
    NAL_UNIT_CODED_SLICE_RADL_R,
    NAL_UNIT_CODED_SLICE_RASL_N,
    NAL_UNIT_CODED_SLICE_RASL_R,
    NAL_UNIT_CODED_SLICE_BLA_W_LP = 16,
    NAL_UNIT_CODED_SLICE_BLA_W_RADL,
    NAL_UNIT_CODED_SLICE_BLA_N_LP,
    NAL_UNIT_CODED_SLICE_IDR_W_RADL,
    NAL_UNIT_CODED_SLICE_IDR_N_LP,
    NAL_UNIT_CODED_SLICE_CRA,
    NAL_UNIT_VPS = 32,
    NAL_UNIT_SPS,
    NAL_UNIT_PPS,
    NAL_UNIT_ACCESS_UNIT_DELIMITER,
    NAL_UNIT_EOS,
    NAL_UNIT_EOB,
    NAL_UNIT_FILLER_DATA,
    NAL_UNIT_PREFIX_SEI,
    NAL_UNIT_SUFFIX_SEI,
    NAL_UNIT_UNSPECIFIED = 62,
    NAL_UNIT_INVALID = 64,
} NalUnitType;

static bool bitstream_serialize(struct bit_stream *bs, uint8_t *out_buf, uint32_t *out_len)
{
	static const char start_code_prefix[] = {0, 0, 0, 1};

	uint32_t payload_size = bitstream_get_number_of_written_bytes(bs);
	const uint8_t *bpayload = bitstream_get_FIFO(bs);
	if (!bpayload)
		return false;

	uint32_t size = sizeof(start_code_prefix) + 2 + payload_size + (payload_size >> 1);
	uint8_t *out = out_buf;
	uint32_t bytes = 0;

	bool annexB = true;
	int nal_unit_type = NAL_UNIT_PREFIX_SEI;
	if (!annexB) {
		/* Will write size later */
		bytes += 4;
	} else if (!nal_unit_type == NAL_UNIT_VPS || nal_unit_type == NAL_UNIT_SPS || nal_unit_type == NAL_UNIT_PPS ||
		   nal_unit_type == NAL_UNIT_UNSPECIFIED) {
		memcpy(out, start_code_prefix, 4);
		bytes += 4;
	} else {
		memcpy(out, start_code_prefix + 1, 3);
		bytes += 3;
	}

	/* 16 bit NAL header:
     * forbidden_zero_bit       1-bit
     * nal_unit_type            6-bits
     * nuh_reserved_zero_6bits  6-bits
     * nuh_temporal_id_plus1    3-bits */
	out[bytes++] = (uint8_t)nal_unit_type << 1;
	out[bytes++] = 1 + (nal_unit_type == NAL_UNIT_CODED_SLICE_TSA_N);

	/* 7.4.1 ...
     * Within the NAL unit, the following three-byte sequences shall not occur at
     * any byte-aligned position:
     *  - 0x000000
     *  - 0x000001
     *  - 0x000002 */
	for (uint32_t i = 0; i < payload_size; i++) {
		if (i > 2 && !out[bytes - 2] && !out[bytes - 3] && out[bytes - 1] <= 0x03 && nal_unit_type != NAL_UNIT_UNSPECIFIED) {
			/* inject 0x03 to prevent emulating a start code */
			out[bytes] = out[bytes - 1];
			out[bytes - 1] = 0x03;
			bytes++;
		}

		out[bytes++] = bpayload[i];
	}

	if (bytes > 4 + 2 + payload_size + (payload_size >> 1)) {
		blog(LOG_ERROR, "NAL buffer overflow");
		return false;
	}

	/* 7.4.1.1
     * ... when the last byte of the RBSP data is equal to 0x00 (which can
     * only occur when the RBSP ends in a cabac_zero_word), a final byte equal
     * to 0x03 is appended to the end of the data.  */
	if (!out[bytes - 1])
		out[bytes++] = 0x03;

	if (!annexB) {
		uint32_t dataSize = bytes - 4;
		out[0] = (uint8_t)(dataSize >> 24);
		out[1] = (uint8_t)(dataSize >> 16);
		out[2] = (uint8_t)(dataSize >> 8);
		out[3] = (uint8_t)dataSize;
	}

	*out_len = bytes;
	return true;
}

struct bit_counter {
	uint32_t counter;
};

static struct bit_counter *bitcounter_create()
{
	struct bit_counter *bc = bzalloc(sizeof(struct bit_counter));
	return bc;
}

static void bitcounter_destroy(struct bit_counter **bc)
{
	if (!*bc)
		return;

	bfree(*bc);
	*bc = NULL;
}

static void bitcounter_write(struct bit_counter *bc, uint32_t val, uint32_t num)
{
	if (!bc)
		return;

	bc->counter += num;
}

static void bitcounter_write_byte(struct bit_counter *bc, uint32_t val)
{
	if (!bc)
		return;

	bc->counter += 8;
}

static void bitcounter_reset_bits(struct bit_counter *bc)
{
	if (!bc)
		return;

	bc->counter = 0;
}

static uint32_t bitcounter_get_number_of_written_bits(struct bit_counter *bc)
{
	if (!bc)
		return 0;

	return bc->counter;
}

static const int ISO_IEC_11578_LEN = 16;
static const uint8_t uuid_iso_iec_11578[16] = {0x2C, 0xA2, 0xDE, 0x09, 0xB5, 0x17, 0x47, 0xDB, 0xBB, 0x55, 0xA4, 0xFE, 0x7F, 0xC2, 0xFC, 0x4E};
#define USER_DATA_UNREGISTERED 5

size_t hevc_sei(unsigned char *out, const unsigned char *in, int in_len)
{
	struct bit_counter *bc = bitcounter_create();
	for (int i = 0; i < ISO_IEC_11578_LEN; i++)
		bitcounter_write(bc, uuid_iso_iec_11578[i], 8);
	for (int i = 0; i < in_len; i++)
		bitcounter_write(bc, in[i], 8);

	uint32_t payload_data = bitcounter_get_number_of_written_bits(bc) >> 3;

	bitcounter_destroy(&bc);

	struct bit_stream *bs = bitstream_create();

	uint32_t payload_type = USER_DATA_UNREGISTERED;
	for (; payload_type >= 0xff; payload_type -= 0xff)
		bitstream_write(bs, 0xff, 8);
	bitstream_write(bs, payload_type, 8);

	uint32_t payload_size = payload_data;
	for (; payload_size >= 0xff; payload_size -= 0xff)
		bitstream_write(bs, 0xff, 8);
	bitstream_write(bs, payload_size, 8);

	for (int i = 0; i < ISO_IEC_11578_LEN; i++)
		bitstream_write(bs, uuid_iso_iec_11578[i], 8);
	for (int i = 0; i < in_len; i++)
		bitstream_write(bs, in[i], 8);

	bitstream_write_byte_alignment(bs);

	uint32_t len = 0;
	bool ret = bitstream_serialize(bs, out, &len);
	bitstream_destroy(&bs);

	return ret ? len : 0;
}
