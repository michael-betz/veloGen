//--------------------------------------------------------
// esp32Httpd cgi callback for serving files from SPIFFS
//--------------------------------------------------------

#include "libesphttpd/esp.h"
#include "app_main.h"
#include "cgi.h"

//This is a catch-all cgi function. It takes the url passed to it, looks up the corresponding
//path in the SPIFFS filesystem and if it exists, passes the file through. This simulates what a normal
//webserver would do with static files.
CgiStatus ICACHE_FLASH_ATTR cgiEspSPIFFSHook(HttpdConnData *connData) {
	FILE *file=connData->cgiData;
	int len;
	char buff[1024];

	if (connData->conn==NULL) {
		//Connection aborted. Clean up.
		fclose(file);
		return HTTPD_CGI_DONE;
	}

	//First call to this cgi.
	if (file==NULL) {
		if (connData->cgiArg != NULL) {
			//Open a different file than provided in http request.
			//Common usage: {"/", cgiEspFsHook, "/index.html"} will show content of index.html without actual redirect to that file if host root was requested
			file = fopen((char*)connData->cgiArg, "r");
		} else {
			//Open the file so we can read it.
			file = fopen(connData->url, "r");
		}

		if (file==NULL) {
			return HTTPD_CGI_NOTFOUND;
		}

		connData->cgiData=file;
		httpdStartResponse(connData, 200);
		httpdHeader(connData, "Content-Type", httpdGetMimetype(connData->url));
		httpdHeader(connData, "Cache-Control", "max-age=3600, must-revalidate");
		httpdEndHeaders(connData);
		return HTTPD_CGI_MORE;
	}

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
}