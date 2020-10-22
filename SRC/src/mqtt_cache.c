// takes care of taking measurements and sending them off on mqtt if connected
// if not, cache measurements in SPIFFS file and send out once connected

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

// path for the cache-file
#define FILE_BUF "/spiffs/velo_buf.dat"

// max. number of measurements to dump in one mqtt message
#define BULK_CHUNKS 85

// format of one measurement
typedef struct {
	unsigned ts;
	unsigned volts;
	int amps;
	unsigned cnt;
} t_datum;

FILE *f_buf = NULL;

const char* mqtt_topic = NULL;

// first 4 bytes in the cache file gives the number of payload bytes
static int f_size = 0;

// the current mqtt message to wait for being transmitted
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

void cache_init()
{
	mqtt_topic = jGetS(getSettings(), "mqtt_topic", "velogen/raw");
	log_i("Publishing to %s", mqtt_topic);

	f_buf = fopen(FILE_BUF, "r+");
	if (!f_buf) {
		log_w("%s: %s, creating a fresh one ...", FILE_BUF, strerror(errno));
		f_buf = fopen(FILE_BUF, "w");

		// first 4 byte are file-size (because f&^*&ing ESP-IDF can not truncate files on SPIFFS!!!)
		f_size = 0;
		fwrite(&f_size, 4, 1, f_buf);
		fclose(f_buf);
		f_buf = fopen(FILE_BUF, "r+");
	}
	if (!f_buf) {
		log_e("Didn't work, giving up :( %s", strerror(errno));
		return;
	}

	// read f_size
	fseek(f_buf, 0, SEEK_SET);
	fread(&f_size, 4, 1, f_buf);

	// sanity check f_size
	fseek(f_buf, 0, SEEK_END);
	log_i("%s size: %ld, f_size: %d", FILE_BUF, ftell(f_buf), f_size);
	if (f_size < 0 || f_size > ftell(f_buf) - 4) {
		log_w("reseting f_size, starting fresh");
		f_size = 0;
		fseek(f_buf, 0, SEEK_SET);
		fwrite(&f_size, 4, 1, f_buf);
	}

	if (f_size % sizeof(t_datum) != 0) {
		log_w("file corrupt? rounding down pos_end to match %d raster", sizeof(t_datum));
		f_size /= sizeof(t_datum);
		f_size *= sizeof(t_datum);
		fseek(f_buf, 0, SEEK_SET);
		fwrite(&f_size, 4, 1, f_buf);
	}

	// register for MQTT events
	E(esp_mqtt_client_register_event(mqtt_c, MQTT_EVENT_CONNECTED, cb_mqtt_con, mqtt_c));
	E(esp_mqtt_client_register_event(mqtt_c, MQTT_EVENT_DISCONNECTED, cb_mqtt_discon, mqtt_c));
	E(esp_mqtt_client_register_event(mqtt_c, MQTT_EVENT_PUBLISHED, cb_mqtt_pub, mqtt_c));
}

// cannot reduce file-size on SPIFFS (thanks IDF!) so keep number of payload bytes
// as int in the first 4 bytes of the file, which is changed by this function
static void trunc_file(int len)
{
	if (!f_buf || len < 0)
		return;
	log_i("Truncating f_size: %d", len);
	fseek(f_buf, 0, SEEK_SET);
	fwrite(&len, 4, 1, f_buf);
	f_size = len;
	// fflush(f_buf);  // doesn't seem to actually flush the data :(
	fclose(f_buf);  // works but expensive. Too bad.
	f_buf = NULL;
	f_buf = fopen(FILE_BUF, "r+");
	if (!f_buf) {
		log_e("Couldn't re-open file after truncating :( %s", strerror(errno));
		return;
	}
	log_i("Trunc done");
}

// call this to take a measurement and deal with it
void cache_handle()
{
	static enum {
		ST_OFFLINE,  // write to cache, dont publish
		ST_ONLINE,  // cache is empty
		ST_ONLINE_CA,  // cache needs to be transmitted
		ST_WAIT_FOR_PUB // wait for ACK of block
	} tx_state;

	bool isCon = isMqttConnect;
	static char *buf = NULL;
	static int initial_cache_size=0, cache_size=0;  // [blocks]
	static int nTX = 0;  // [blocks]

	// collect a new data point
	time_t now = time(NULL);
	t_datum datum;
	datum.ts = now;
	datum.volts = g_mVolts;
	datum.amps = g_mAmps;
	datum.cnt = g_wheelCnt;

	switch (tx_state) {
		case ST_ONLINE_CA:

			// freshly offline, truncate payload as it may have been partially transmitted
			if (!isCon) {
				trunc_file(cache_size * sizeof(datum));
				tx_state = ST_OFFLINE;
				break;
			}

			// no more data in the cache to transmit
			if (cache_size <= 0) {
				trunc_file(0);
				tx_state = ST_ONLINE;
				break;
			}

			nTX = cache_size;  // number of [blocks] to transmit in this cycle
			if (nTX > BULK_CHUNKS)
				nTX = BULK_CHUNKS;
			log_i("cache_size: %d,  nTX: %d", cache_size, nTX);
			buf = malloc(nTX * sizeof(datum));
			if (!buf) {
				log_e("Could not allocate buffer :(");
				break;
			}
			fseek(f_buf, (cache_size - nTX) * sizeof(datum) + 4, SEEK_SET);
			fread(buf, nTX * sizeof(datum), 1, f_buf);
			msg_needs_ack = esp_mqtt_client_publish(mqtt_c, mqtt_topic, buf, nTX * sizeof(datum), 1, 0);
			free(buf);
			tx_state = ST_WAIT_FOR_PUB;
			// short circuit to ST_WAIT_FOR_PUB, see if there's an ack already

		case ST_WAIT_FOR_PUB:
			// went off-line while waiting for MQTT ACK
			if (!isCon) {
				// clear pending transmissions?
				tx_state = ST_OFFLINE;
				break;
			}

			// broker sent ACK for the currently active block
			if (msg_needs_ack == 0 || (last_ack > 0 && last_ack == msg_needs_ack)) {
				last_ack = -1;
				msg_needs_ack = -1;
				cache_size -= nTX;
				setStatus("ACK %d / %d", initial_cache_size - cache_size, initial_cache_size);
				tx_state = ST_ONLINE_CA;
			}
			break;

		case ST_ONLINE:
			if (!isCon) {
				// freshly offline, cache is empty
				tx_state = ST_OFFLINE;
			}
			break;

		case ST_OFFLINE:
			// freshly online, start emptying the cache
			if (isCon) {
				// How many blocks are in the cache?
				cache_size = f_size / sizeof(datum);
				initial_cache_size = cache_size;
				tx_state = ST_ONLINE_CA;
			}
			break;
	}

	// take care of the most recent data point (publish or cache)
	if (isCon) {
		// if mqtt is connected, publish immediately with QOS1
		esp_mqtt_client_publish(mqtt_c, mqtt_topic, (const char *)&datum, sizeof(datum), 1, 0);
	} else {
		// append to cache file. 1.4 MB spiffs is enough for 26 h at 1 Hz
		if (!f_buf)
			return;
		fseek(f_buf, f_size + 4, SEEK_SET);
		if (fwrite(&datum, sizeof(datum), 1, f_buf) != 1) {
			log_e("%s write error! Stopping caching.", FILE_BUF);
			fclose(f_buf);
			f_buf = NULL;
			return;
		}
		f_size += sizeof(datum);
		fseek(f_buf, 0, SEEK_SET);
		fwrite(&f_size, 4, 1, f_buf);
		fflush(f_buf);
		log_d("f_size: %d", f_size);
	}
}
