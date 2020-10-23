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
#define FILE_PTR "/spiffs/velo_ptr.dat"

// cache is a ring buffer, 1 MB is enough for 18 h at 1 Hz
#define MAX_CACHE_SIZE (1024 * 1024 / BLOCK_SIZE)  // [blocks]

// max. number of blocks to dump in one mqtt message
#define BULK_CHUNKS 85

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
		log_i("msg_needs_ack=%d, msg_id=%d", msg_needs_ack, event->msg_id);
		msg_needs_ack = 0;
	}
}
static void cb_mqtt_con(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) { isMqttConnect = true; }
static void cb_mqtt_discon(void *handler_args, esp_event_base_t base, int32_t event_id, void *event_data) {	isMqttConnect = false; }

// write block_first and block_N to FILE_PTR
static int commit_ptrs()
{
	FILE *f_ptr = fopen(FILE_PTR, "w");
	if (!f_ptr) {
		log_e("%s: %s, cannot write, stopping caching", FILE_PTR, strerror(errno));
		fclose(f_buf);
		f_buf = NULL;
		return -1;
	}
	fwrite(&block_first, sizeof(block_first), 1, f_ptr);
	fwrite(&block_N, sizeof(block_N), 1, f_ptr);
	fclose(f_ptr);
	log_d("commit_ptrs(): block_first: %d, block_N: %d", block_first, block_N);
	return 0;
}

void cache_init()
{
	mqtt_topic = jGetS(getSettings(), "mqtt_topic", "velogen/raw");
	log_i("Publishing to %s", mqtt_topic);

	// register for MQTT events
	E(esp_mqtt_client_register_event(mqtt_c, MQTT_EVENT_CONNECTED, cb_mqtt_con, mqtt_c));
	E(esp_mqtt_client_register_event(mqtt_c, MQTT_EVENT_DISCONNECTED, cb_mqtt_discon, mqtt_c));
	E(esp_mqtt_client_register_event(mqtt_c, MQTT_EVENT_PUBLISHED, cb_mqtt_pub, mqtt_c));

	// read pointer file
	FILE *f_ptr = fopen(FILE_PTR, "r");
	if (f_ptr) {
		fread(&block_first, sizeof(block_first), 1, f_ptr);
		fread(&block_N, sizeof(block_N), 1, f_ptr);
		fclose(f_ptr);
	} else {
		log_w("%s: %s, creating a fresh one ...", FILE_PTR, strerror(errno));
		block_first = 0;
		block_N = 0;
		if (commit_ptrs() < 0) {
			log_e("Failed, giving up :( %s", strerror(errno));
			return;
		}
	}

	if (block_first > (MAX_CACHE_SIZE - 1) || block_N > MAX_CACHE_SIZE) {
		log_w("illegal cache: block_first = %d, block_N = %d, starting from scratch", block_first, block_N);
		block_first = 0;
		block_N = 0;
	} else {
		log_i("cache initialized: block_first = %d, block_N = %d", block_first, block_N);
	}

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
}

// call this to take a measurement and deal with them
void cache_handle()
{
	static unsigned seq = 0;

	static enum {
		ST_OFFLINE,  // write to cache, dont publish
		ST_ONLINE,  // cache is empty
		ST_ONLINE_CA,  // cache needs to be transmitted
		ST_WAIT_FOR_PUB // wait for ACK of block
	} tx_state;

	bool isCon = isMqttConnect;
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
	// datum.volts = 0;
	// datum.amps = 0;
	// datum.cnt = 0;

	switch (tx_state) {
		case ST_ONLINE_CA:
			// freshly offline, but there's still data in the cache
			if (!isCon) {
				commit_ptrs();
				tx_state = ST_OFFLINE;
				break;
			}

			// no more data in the cache to transmit
			if (block_N <= 0) {
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

			fseek(f_buf, block_first * BLOCK_SIZE, SEEK_SET);
			fread(buf, nTX * BLOCK_SIZE, 1, f_buf);

			msg_needs_ack = esp_mqtt_client_publish(mqtt_c, mqtt_topic, buf, nTX * BLOCK_SIZE, 1, 0);
			free(buf);
			buf = NULL;

			tx_state = ST_WAIT_FOR_PUB;
			// short circuit to ST_WAIT_FOR_PUB, see if there's an ack already

		case ST_WAIT_FOR_PUB:
			// went off-line while waiting for MQTT ACK
			if (!isCon) {
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

				setStatus("ACK %d / %d", initial_block_N - block_N, initial_block_N);
				log_i("ACK: block_first: %d, block_N: %d", block_first, block_N);
				tx_state = ST_ONLINE_CA;
			}
			break;

		case ST_ONLINE:
			// freshly offline, cache is empty
			if (!isCon) {
				tx_state = ST_OFFLINE;
			}
			break;

		case ST_OFFLINE:
			// freshly online, start emptying the cache
			if (isCon) {
				// How many blocks are in the cache?
				initial_block_N = block_N;
				tx_state = ST_ONLINE_CA;
			}
			break;
	}

	// take care of the most recent data point (publish or cache)
	if (isCon) {
		// if mqtt is connected, publish immediately with QOS1
		esp_mqtt_client_publish(mqtt_c, mqtt_topic, (const char *)&datum, sizeof(datum), 1, 0);
	} else {
		// append to cache ring buffer. 1 MB is enough for 18 h at 1 Hz
		if (!f_buf)
			return;

		unsigned wPtr = (block_first + block_N) % MAX_CACHE_SIZE; // [blocks]
		fseek(f_buf, wPtr * BLOCK_SIZE, SEEK_SET);

		if (fwrite(&datum, BLOCK_SIZE, 1, f_buf) != 1) {
			log_e("%s write error! Stopping caching.", FILE_BUF);
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
		commit_ptrs();
	}

	seq++;
}
