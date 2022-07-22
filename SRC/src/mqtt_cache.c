// takes care of taking measurements and sending them off on mqtt if connected
// if not, cache measurements in SPIFFS file and send out once connected
// implement a circular buffer in the file.
// There is a read pointer, pointing to the first valid block and the number of blocks
// both are stored in `velo_ptr.dat`

#include <errno.h>
#include <stdio.h>
#include "esp_event.h"
#include "esp_log.h"
#include "mqtt_client.h"
#include "json_settings.h"
#include "velo_wifi.h"
#include "velo_gui.h"
#include "velo.h"

static const char *T = "MQTT_CACHE";

// format of one measurement
typedef struct {
	unsigned ts;
	unsigned volts;
	int amps;
	unsigned cnt;
} t_datum;
#define BLOCK_SIZE sizeof(t_datum)

// paths for the cache and pointer file
#define FILE_BUF "/spiffs/velo_buf.dat"
#define FILE_PTR1 "/spiffs/velo_ptr1.dat"
#define FILE_PTR2 "/spiffs/velo_ptr2.dat"

// cache is a ring buffer, 0.5 MB is enough for 9 h at 1 Hz
#define MAX_CACHE_SIZE (512 * 1024 / BLOCK_SIZE)  // [blocks]

// max. number of blocks to dump in one mqtt message
#define BULK_CHUNKS 64

// cache file (stays open for r+)
FILE *f_buf = NULL;

// initialized from .json
const char* mqtt_topic = NULL;

// first entry (read pointer) and number of entries in buffer file [blocks]
static unsigned block_first=0, block_N=0;

// msg_id of the currently transmitted / last acknowledged mqtt message
static int msg_needs_ack=-1, last_ack=-1;

static void cb_mqtt_pub(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data)
{
	esp_mqtt_event_handle_t event = (esp_mqtt_event_handle_t) event_data;
	last_ack = event->msg_id;
	if (msg_needs_ack == event->msg_id) {
		log_d("msg_needs_ack=%d, msg_id=%d", msg_needs_ack, event->msg_id);
		msg_needs_ack = 0;
	}
}
static void cb_mqtt_con(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) { isMqttConnect = true; }
static void cb_mqtt_discon(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {	isMqttConnect = false; }

// write block_first and block_N to FILE_PTR
static int commit_ptrs()
{
	static unsigned seq=0;
	const char *fName = (seq & 1) ? FILE_PTR2 : FILE_PTR1;
	seq++;
	FILE *f_ptr = fopen(fName, "w");
	if (!f_ptr) {
		log_e("%s: %s, cannot write, stopping caching", fName, strerror(errno));
		fclose(f_buf);
		f_buf = NULL;
		return -1;
	}
	int ret = fwrite(&block_first, sizeof(block_first), 1, f_ptr);
	ret += fwrite(&block_N, sizeof(block_N), 1, f_ptr);
	ret += fwrite(&seq, sizeof(seq), 1, f_ptr);
	fclose(f_ptr);
	log_d("commit_ptrs(): block_first: %d, block_N: %d, seq: %d, ret: %d", block_first, block_N, seq, ret);
	return 0;
}

static void load_ptrs()
{
	unsigned a_first=0, a_N=0, a_seq=0, b_first=0, b_N=0, b_seq=0;
	int ret = -1;

	// read pointer file A
	FILE *f_ptr = fopen(FILE_PTR1, "r");
	if (f_ptr) {
		ret = fread(&a_first, sizeof(a_first), 1, f_ptr);
		ret += fread(&a_N, sizeof(a_N), 1, f_ptr);
		ret += fread(&a_seq, sizeof(a_seq), 1, f_ptr);
		log_w("cacheA: %d, %d, %d", a_first, a_N, a_seq);
		fclose(f_ptr);
		f_ptr = NULL;
	}
	if (ret != 3) {
		log_w("Reading %s failed. Ret: %d", FILE_PTR1, ret);
		a_first = 0;
		a_N = 0;
		a_seq = 0;
	}

	// read pointer file B
	f_ptr = fopen(FILE_PTR2, "r");
	if (f_ptr) {
		ret = fread(&b_first, sizeof(b_first), 1, f_ptr);
		ret += fread(&b_N, sizeof(b_N), 1, f_ptr);
		ret += fread(&b_seq, sizeof(b_seq), 1, f_ptr);
		log_w("cacheB: %d, %d, %d", b_first, b_N, b_seq);
		fclose(f_ptr);
		f_ptr = NULL;
	}
	if (ret != 3) {
		log_w("Reading %s failed. Ret: %d", FILE_PTR2, ret);
		b_first = 0;
		b_N = 0;
		b_seq = 0;
	}

	if (b_seq > a_seq) {
		block_first = b_first;
		block_N = b_N;
	} else {
		block_first = a_first;
		block_N = a_N;
	}

	if (block_first > (MAX_CACHE_SIZE - 1) || block_N > MAX_CACHE_SIZE) {
		log_w("illegal cache: block_first = %d, block_N = %d, starting from scratch", block_first, block_N);
		block_first = 0;
		block_N = 0;
	} else {
		log_w("cache initialized: block_first = %d, block_N = %d", block_first, block_N);
	}
}

int meas_ticks = 1;

void cache_init()
{
	cJSON *s = getSettings();
	meas_ticks = jGetI(s, "meas_ticks", 20);  // 0 = off, otherwise [.05 s]
	mqtt_topic = jGetS(s, "mqtt_topic", "velogen/raw");
	log_i("Publishing to %s", mqtt_topic);

	// register for MQTT events
	E(esp_mqtt_client_register_event(mqtt_c, MQTT_EVENT_CONNECTED, cb_mqtt_con, NULL));
	E(esp_mqtt_client_register_event(mqtt_c, MQTT_EVENT_DISCONNECTED, cb_mqtt_discon, NULL));
	E(esp_mqtt_client_register_event(mqtt_c, MQTT_EVENT_PUBLISHED, cb_mqtt_pub, mqtt_c));

	load_ptrs();

	// open buffer file
	f_buf = fopen(FILE_BUF, "r+");
	if (!f_buf) {
		log_w("%s: %s, creating a fresh one ...", FILE_BUF, strerror(errno));
		f_buf = fopen(FILE_BUF, "w");
		fclose(f_buf);
		f_buf = fopen(FILE_BUF, "r+");
		block_first = 0;
		block_N = 0;
	}
	if (!f_buf) {
		log_e("Didn't work, giving up :( %s", strerror(errno));
		return;
	}

	// avoid seeking beyond end of file, might need to write some dummy
	// bytes to get the file-size to match up with write pointer
	// https://github.com/pellepl/spiffs/wiki/Using-spiffs#seeking-in-a-file
	fseek(f_buf, 0, SEEK_END);
	unsigned fSize = ftell(f_buf);
	log_i("fSize = %d", fSize);
	unsigned wPtr = (block_first + block_N) % MAX_CACHE_SIZE; // [blocks]
	int lostBytes = wPtr * BLOCK_SIZE - fSize;
	if (lostBytes > 0) {
		log_w("appending %d dummy bytes", lostBytes);
		while (lostBytes-- > 0)
			fputc('\0', f_buf);
	}
}

// call this to take a measurement and deal with them
void cache_handle()
{
	static unsigned seq=0, m_seq=0;

	static enum {
		ST_OFFLINE,  // write to cache, dont publish
		ST_ONLINE,  // cache is empty
		ST_ONLINE_CA,  // cache needs to be transmitted
		ST_WAIT_FOR_PUB // wait for ACK of block
	} tx_state;

	static char *buf = NULL;
	static int initial_block_N=0;  // [blocks]
	static int nTX = 0;  // [blocks]

	// collect a new data point
	time_t now = time(NULL);
	t_datum datum;
	datum.ts = now;
	datum.volts = g_mVolts;
	datum.amps = g_mAmps;
	datum.cnt = g_wheelCnt;

	// datum.ts = seq;
	// datum.volts = 1;
	// datum.amps = 2;
	// datum.cnt = 3;

	if (f_buf){
		switch (tx_state) {
			case ST_ONLINE_CA:
				// freshly offline, but there's still data in the cache
				if (!isMqttConnect) {
					commit_ptrs();
					tx_state = ST_OFFLINE;
					break;
				}

				// no more data in the cache to transmit
				if (block_N <= 0) {
					block_N = 0;
					block_first = 0;
					commit_ptrs();
					tx_state = ST_ONLINE;
					break;
				}

				// nTX = number of [blocks] to transmit in this cycle
				nTX = block_N;
				// avoid buffer roll-over during transmission
				if ((block_first + nTX) > MAX_CACHE_SIZE)
					nTX = MAX_CACHE_SIZE - block_first;
				if (nTX > BULK_CHUNKS)
					nTX = BULK_CHUNKS;

				log_i("ST_ONLINE_CA: block_first: %d, block_N: %d,  nTX: %d", block_first, block_N, nTX);
				buf = malloc(nTX * BLOCK_SIZE);
				if (!buf) {
					log_e("Could not allocate buffer :(");
					break;
				}
				if (fseek(f_buf, block_first * BLOCK_SIZE, SEEK_SET) < 0) {
					log_e("READ seek failed, to %d, %s", block_first * BLOCK_SIZE, strerror(errno));
					free(buf);
					break;
				}
				int ret = fread(buf, nTX * BLOCK_SIZE, 1, f_buf);
				if (ret != 1) {
					log_e("READ fread failed :(, read %d, ret %d", nTX * BLOCK_SIZE, ret);
					esp_log_level_set("SPIFFS", ESP_LOG_DEBUG);
					fclose(f_buf);
					f_buf = fopen(FILE_BUF, "r+");
					free(buf);
					block_first = 0;
					block_N = 0;
					commit_ptrs();
					break;
				}

				msg_needs_ack = esp_mqtt_client_publish(mqtt_c, mqtt_topic, buf, nTX * BLOCK_SIZE, 1, 0);
				free(buf);
				buf = NULL;

				tx_state = ST_WAIT_FOR_PUB;
				// short circuit to ST_WAIT_FOR_PUB, see if there's an ack already
				__attribute__ ((fallthrough));

			case ST_WAIT_FOR_PUB:
				// went off-line while waiting for MQTT ACK
				if (!isMqttConnect) {
					commit_ptrs();
					tx_state = ST_OFFLINE;
					break;
				}

				// broker sent ACK for the currently active block
				if (msg_needs_ack == 0 || (last_ack > 0 && last_ack == msg_needs_ack)) {
					last_ack = -1;
					msg_needs_ack = -1;

					// move cache pointers
					block_first = (block_first + nTX) % MAX_CACHE_SIZE;
					block_N -= nTX;
					commit_ptrs();

					setStatus("ACK %d / %d", initial_block_N - block_N, initial_block_N);
					tx_state = ST_ONLINE_CA;
				}
				break;

			case ST_ONLINE:
				// freshly offline, cache is empty
				if (!isMqttConnect) {
					tx_state = ST_OFFLINE;
				}
				break;

			case ST_OFFLINE:
				// freshly online, start emptying the cache
				if (isMqttConnect) {
					// How many blocks are in the cache?
					initial_block_N = block_N;
					tx_state = ST_ONLINE_CA;
				}
				break;
		}
	}

	// shall we take a new data point?
	if (!(meas_ticks > 0 && (seq % meas_ticks) == 0)) {
		seq++;
		return;
	}

	if (isMqttConnect) {
		// if mqtt is connected, publish immediately with QOS1
		esp_mqtt_client_publish(mqtt_c, mqtt_topic, (const char *)&datum, sizeof(datum), 1, 0);
	} else {
		// otherwise append to ring buffer file on SPIFFS
		if (!f_buf)
			return;

		unsigned wPtr = (block_first + block_N) % MAX_CACHE_SIZE; // [blocks]

		// bool sk = false;
		if (ftell(f_buf) != wPtr * BLOCK_SIZE) {
			// sk = true;
			if (fseek(f_buf, wPtr * BLOCK_SIZE, SEEK_SET) < 0) {
				log_e("seek failed, to %d, %s. Stopping caching.", wPtr * BLOCK_SIZE, strerror(errno));
				fclose(f_buf);
				f_buf = NULL;
				return;
			}
		}

		if (fwrite(&datum, BLOCK_SIZE, 1, f_buf) != 1) {
			log_e("%s write error! write %d. Stopping caching.", FILE_BUF, BLOCK_SIZE);
			fclose(f_buf);
			f_buf = NULL;
			return;
		}
		fflush(f_buf);

		if (block_N < MAX_CACHE_SIZE) {
			block_N++;
		} else {
			block_first = (block_first + 1) % MAX_CACHE_SIZE;
		}
		if ((m_seq % 10) == 0)
			commit_ptrs();
	}

	m_seq++;
	seq++;
}
