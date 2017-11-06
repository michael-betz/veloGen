//--------------------------------------------------------
// esp32Httpd cgi callback for serving files from SPIFFS
//--------------------------------------------------------

#include <errno.h>
#include <dirent.h> 
#include <sys/stat.h>
#include "libesphttpd/esp.h"
#include "esp_log.h"
#include "cJSON.h"

#include "app_main.h"
#include "cgi.h"

static const char *T = "CGI_C";

//-----------------------------------------
// List spiffs files as json
//-----------------------------------------
CgiStatus ICACHE_FLASH_ATTR cgiEspSPIFFSListHook(HttpdConnData *connData) {
	struct stat st;
	struct dirent *dir;
	DIR *d;
	cJSON *jRoot, *jFile;
	int maxFiles = 32;
	char *jsonString, lastFname[35], tempBuff[37];

	if (connData->conn==NULL) {
		//Connection aborted. Clean up.
		return HTTPD_CGI_DONE;
	}
	httpdStartResponse(connData, 200);
	httpdHeader(connData, "Content-Type", "application/json");
	// httpdHeader(connData, "Cache-Control", "max-age=10, must-revalidate");
	httpdEndHeaders(connData);

	if (NULL == (d=opendir("/S"))) {
		ESP_LOGE(T,"Could not open root folder: %s", strerror(errno) );
		return HTTPD_CGI_DONE;
	}
	jRoot = cJSON_CreateObject();
		while ( (dir = readdir(d)) ) {
		// Crude workaround for infinite loop with repeating filenames
		if( strcmp( lastFname, dir->d_name) == 0 ) break;
		if( !maxFiles-- ) break;
		strcpy( lastFname, dir->d_name );
		cJSON_AddItemToObject(   jRoot,  dir->d_name, jFile=cJSON_CreateObject());
		cJSON_AddNumberToObject( jFile, "d_type", dir->d_type );
		snprintf( tempBuff, 37, "/S/%s", dir->d_name );
		if( stat(tempBuff, &st) >= 0 ){
			cJSON_AddNumberToObject( jFile, "st_size",  st.st_size );
			cJSON_AddNumberToObject( jFile, "st_atime", st.st_atime );
			cJSON_AddNumberToObject( jFile, "st_mtime", st.st_mtime );
			cJSON_AddNumberToObject( jFile, "st_ctime", st.st_ctime );
		} else {
			ESP_LOGE(T,"stat( %s ) failed: %s", tempBuff, strerror(errno) );
		}
		ESP_LOGW(T, "%s (%d)\n", dir->d_name, dir->d_type );
	}
	closedir(d);
	jsonString = cJSON_Print( jRoot );
	httpdSend(connData, jsonString, strlen(jsonString) );
	free( jsonString );
	cJSON_Delete( jRoot );
	return HTTPD_CGI_DONE;
}

//-----------------------------------------
// Transfer file to / from SPIFFS
//-----------------------------------------
//This is a catch-all cgi function. It takes the url passed to it, looks up the corresponding
//path in the SPIFFS filesystem and if it exists, passes the file through. This simulates what a normal
//webserver would do with static files.
CgiStatus ICACHE_FLASH_ATTR cgiEspSPIFFSHook(HttpdConnData *connData) {
	FILE *file=connData->cgiData;
	HttpdPostData *p;
	int len;
	char buff[1024];
	const char *rwMode, *fName;

	if (connData->conn==NULL) {
		//Connection aborted. Clean up.
		fclose(file);
		return HTTPD_CGI_DONE;
	}

	//-----------------------------------------
	// First call
	//-----------------------------------------
	// Initialize state variables
	if (file==NULL) {
		rwMode = (connData->requestType==HTTPD_METHOD_POST) ? "w" : "r";
		//Open a different file than provided in http request.
		//Common usage: {"/", cgiEspFsHook, "/index.html"} will show content of index.html without actual redirect to that file if host root was requested
		fName  = (connData->cgiArg) ? (char*)connData->cgiArg : connData->url;
		file = fopen( fName, rwMode );
		ESP_LOGI(T, "%d = fopen(%s, %s)", (int)file, fName, rwMode );
		if (file==NULL) {
			return HTTPD_CGI_NOTFOUND;
		}
		connData->cgiData=file;
		if( connData->requestType==HTTPD_METHOD_GET ){
			httpdStartResponse(connData, 200);
			httpdHeader(connData, "Content-Type", httpdGetMimetype(connData->url));
			httpdHeader(connData, "Cache-Control", "max-age=3600, must-revalidate");
			httpdEndHeaders(connData);
			return HTTPD_CGI_MORE;
		} else {
			ESP_LOGI(T, "Writing %d bytes", connData->post->len );
		}
	}

	if( connData->requestType==HTTPD_METHOD_GET ){
		//-----------------------------------------
		// GET /S/*
		//-----------------------------------------
		// Read file from SPIFFS
		len=fread(buff, 1, 1024, file);
		if (len>0) httpdSend(connData, buff, len);
		if (len!=1024) {
			//We're done.
			fclose(file);
			return HTTPD_CGI_DONE;
		} else {
			//Ok, till next time.
			return HTTPD_CGI_MORE;
		}
	} else {
		//----------------------------------------------------
		// POST /S/*
		//----------------------------------------------------
		// (Over)write file on spiffs
		p = connData->post;
		len = fwrite( p->buff, 1, p->buffLen, file );
		if ( len != p->buffLen ){
			ESP_LOGE(T, "%d = fwrite(), should be %d, err: %s", len, p->buffLen, strerror(errno) );
		}
		if (connData->post->len==connData->post->received) {
			//We're done! Format a response.
			fclose(file);
			ESP_LOGI(T, "Upload done.\n");
			httpdStartResponse(connData, 200);
			httpdHeader(connData, "Content-Type", "text/plain");
			httpdEndHeaders(connData);
			return HTTPD_CGI_DONE;
		} else {
			//Ok, till next time.
			return HTTPD_CGI_MORE;
		}	
	}
}