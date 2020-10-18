// static file server + a websocket
// stolen from
// https://github.com/espressif/esp-idf/blob/master/examples/protocols/http_server/file_serving/main/file_server.c

#include <stdio.h>
#include "esp_http_server.h"
#include "esp_log.h"
#include "esp_vfs.h"

static const char *T = "STATIC_WS";

static httpd_handle_t server = NULL;

#define FILE_PATH_MAX (ESP_VFS_PATH_MAX + CONFIG_SPIFFS_OBJ_NAME_LEN)
#define MIN(a, b) ((a) < (b) ? (a) : (b))
#define MAX(a, b) ((a) > (b) ? (a) : (b))

#define BASE_PATH "/spiffs"
#define SCRATCH_BUFSIZE  8192

char scratch[SCRATCH_BUFSIZE];

#define IS_FILE_EXT(filename, ext) \
	(strcasecmp(&filename[strlen(filename) - sizeof(ext) + 1], ext) == 0)

// Set HTTP response content type according to file extension
static esp_err_t set_content_type_from_file(httpd_req_t *req, const char *filename)
{
	if (IS_FILE_EXT(filename, ".htm")) return httpd_resp_set_type(req, "text/html");
	if (IS_FILE_EXT(filename, ".html"))return httpd_resp_set_type(req, "text/html");
	if (IS_FILE_EXT(filename, ".css")) return httpd_resp_set_type(req, "text/css");
	if (IS_FILE_EXT(filename, ".js"))  return httpd_resp_set_type(req, "application/javascript");
	if (IS_FILE_EXT(filename, ".png")) return httpd_resp_set_type(req, "image/png");
	if (IS_FILE_EXT(filename, ".gif")) return httpd_resp_set_type(req, "image/gif");
	if (IS_FILE_EXT(filename, ".jpg")) return httpd_resp_set_type(req, "image/jpeg");
	if (IS_FILE_EXT(filename, ".ico")) return httpd_resp_set_type(req, "image/x-icon");
	if (IS_FILE_EXT(filename, ".xml")) return httpd_resp_set_type(req, "text/xml");
	if (IS_FILE_EXT(filename, ".pdf")) return httpd_resp_set_type(req, "application/x-pdf");
	if (IS_FILE_EXT(filename, ".zip")) return httpd_resp_set_type(req, "application/x-zip");
	if (IS_FILE_EXT(filename, ".gz"))  return httpd_resp_set_type(req, "application/x-gzip");
	return httpd_resp_set_type(req, "text/plain");
}

/* Copies the full path into destination buffer and returns
 * pointer to path (skipping the preceding base path) */
static const char* get_path_from_uri(char *dest, const char *uri, size_t destsize)
{
	const size_t base_pathlen = strlen(BASE_PATH);
	size_t pathlen = strlen(uri);

	const char *quest = strchr(uri, '?');
	if (quest) {
		pathlen = MIN(pathlen, quest - uri);
	}
	const char *hash = strchr(uri, '#');
	if (hash) {
		pathlen = MIN(pathlen, hash - uri);
	}

	if (base_pathlen + pathlen + 1 > destsize) {
		/* Full path string won't fit into destination buffer */
		return NULL;
	}

	/* Construct full path (base + path) */
	strcpy(dest, BASE_PATH);
	strlcpy(dest + base_pathlen, uri, pathlen + 1);

	/* Return pointer to path, skipping the base */
	return dest + base_pathlen;
}

// redirect / to /index.htm
static esp_err_t index_html_get_handler(httpd_req_t *req)
{
	httpd_resp_set_status(req, "307 Temporary Redirect");
	httpd_resp_set_hdr(req, "Location", "/index.htm");
	httpd_resp_send(req, NULL, 0);  // Response body can be empty
	return ESP_OK;
}

// Handler to download a file kept on SPIFFS
static esp_err_t download_get_handler(httpd_req_t *req)
{
	char filepath[FILE_PATH_MAX];
	FILE *fd = NULL;
	struct stat file_stat;

	const char *filename = get_path_from_uri(filepath, req->uri, sizeof(filepath));
	if (!filename) {
		ESP_LOGE(T, "Filename is too long");
		/* Respond with 500 Internal Server Error */
		httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Filename too long");
		return ESP_FAIL;
	}

	if (strcmp(filename, "/") == 0)
		return index_html_get_handler(req);

	if (stat(filepath, &file_stat) == -1) {
		ESP_LOGE(T, "Failed to stat file : %s", filepath);
		/* Respond with 404 Not Found */
		httpd_resp_send_err(req, HTTPD_404_NOT_FOUND, "File does not exist");
		return ESP_FAIL;
	}

	fd = fopen(filepath, "r");
	if (!fd) {
		ESP_LOGE(T, "Failed to read existing file : %s", filepath);
		/* Respond with 500 Internal Server Error */
		httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to read existing file");
		return ESP_FAIL;
	}

	ESP_LOGI(T, "Sending file : %s (%ld bytes)...", filename, file_stat.st_size);
	set_content_type_from_file(req, filename);

	/* Retrieve the pointer to scratch buffer for temporary storage */
	char *chunk = scratch;
	size_t chunksize;
	do {
		/* Read file in chunks into the scratch buffer */
		chunksize = fread(chunk, 1, SCRATCH_BUFSIZE, fd);

		if (chunksize > 0) {
			/* Send the buffer contents as HTTP response chunk */
			if (httpd_resp_send_chunk(req, chunk, chunksize) != ESP_OK) {
				fclose(fd);
				ESP_LOGE(T, "File sending failed!");
				/* Abort sending file */
				httpd_resp_sendstr_chunk(req, NULL);
				/* Respond with 500 Internal Server Error */
				httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to send file");
			   return ESP_FAIL;
		   }
		}

		/* Keep looping till the whole file is sent */
	} while (chunksize != 0);

	/* Close file after sending complete */
	fclose(fd);
	ESP_LOGI(T, "File sending complete");

	/* Respond with an empty chunk to signal HTTP response completion */
	httpd_resp_send_chunk(req, NULL, 0);
	return ESP_OK;
}


// This handles websocket traffic
// unfotunately needs ESP-IDF 4.2.x which is not yet av. on platformio
// static esp_err_t ws_handler(httpd_req_t *req)
// {
// 	uint8_t buf[128] = { 0 };
// 	httpd_ws_frame_t ws_pkt;
// 	memset(&ws_pkt, 0, sizeof(httpd_ws_frame_t));
// 	ws_pkt.payload = buf;
// 	ws_pkt.type = HTTPD_WS_TYPE_TEXT;
// 	esp_err_t ret = httpd_ws_recv_frame(req, &ws_pkt, 128);
// 	if (ret != ESP_OK) {
// 		ESP_LOGE(T, "httpd_ws_recv_frame failed with %d", ret);
// 		return ret;
// 	}
// 	ESP_LOGI(T, "Got packet with message: %s", ws_pkt.payload);
// 	ESP_LOGI(T, "Packet type: %d", ws_pkt.type);
// 	// if (ws_pkt.type == HTTPD_WS_TYPE_TEXT &&
// 	//     strcmp((char*)ws_pkt.payload,"Trigger async") == 0) {
// 	//     return trigger_async_send(req->handle, req);
// 	// }

// 	ret = httpd_ws_send_frame(req, &ws_pkt);
// 	if (ret != ESP_OK) {
// 		ESP_LOGE(T, "httpd_ws_send_frame failed with %d", ret);
// 	}
// 	return ret;
// }

void startWebServer()
{
	httpd_config_t config = HTTPD_DEFAULT_CONFIG();
	config.uri_match_fn = httpd_uri_match_wildcard;

	// Start the httpd server
	ESP_LOGI(T, "Starting server on port: '%d'", config.server_port);
	if (httpd_start(&server, &config) != ESP_OK) {
		ESP_LOGE(T, "Error starting server!");
		return;
	}

	// Set URI handlers
	ESP_LOGI(T, "Registering URI handlers");
	httpd_uri_t file_download = {
		.uri       = "/*",  // Match all URIs of type /path/to/file
		.method    = HTTP_GET,
		.handler   = download_get_handler
	};
	httpd_register_uri_handler(server, &file_download);

	// httpd_uri_t ws = {
	// 	.uri        = "/ws",
	// 	.method     = HTTP_GET,
	// 	.handler    = ws_handler,
	// 	.user_ctx   = NULL,
	// 	.is_websocket = true
	// };
	// httpd_register_uri_handler(server, &ws);
}
