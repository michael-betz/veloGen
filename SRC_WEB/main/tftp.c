#include <string.h>
#include "esp_log.h"
#include <lwip/err.h>
#include <lwip/sockets.h>
#include <lwip/sys.h>
#include <lwip/netdb.h>
#include "app_main.h"
#include "wifi_startup.h"
#include "tftp.h"

static const char *T = "TFTP";

int sTX=0;				// UDP socket handles
struct sockaddr_in addrTx;		// Remote IP addr. Written once remote sent us a udp packet, so we can answer
uint16_t sendNBytes, currentBlockId;

// Send a TFTP error response with user defined error code and message
void sendError( uint16_t errNum, const char* errString ){
	#define ERR_PACKET_LEN 32+5
	uint8_t errPacket[ERR_PACKET_LEN];
	ESP_LOGE(T, "TFTP error %s", errString );
	memset( errPacket, 0, ERR_PACKET_LEN );
	errPacket[1] = TFTP_OPCODE_ERROR;
	errPacket[2] = errNum>>8;
	errPacket[3] = errNum;
	strncpy((char*)&errPacket[4], errString, 32 );
	hexDump( errPacket, ERR_PACKET_LEN );
	sendto( sTX, errPacket, ERR_PACKET_LEN, 0, (struct sockaddr*)&addrTx, sizeof(addrTx) );
}

// Sends a data block (<= 512 bytes) returns how many bytes have been sent, decrements len
uint16_t sendData( uint8_t *buff, uint16_t *len ){
	#define DATA_PACKET_LEN 512+4
	uint16_t packetLen = 4;
	uint8_t dataPacket[DATA_PACKET_LEN];
	uint8_t *wp=dataPacket;
	*wp++ = 0;
	*wp++ = TFTP_OPCODE_DATA;
	*wp++ = currentBlockId>>8;
	*wp++ = currentBlockId;
	for( uint16_t i=0; i<512; i++ ){
		if( *len <= 0 ){
			break;
		}
		*wp++ = *buff++;
		packetLen++;
		*len -= 1;
	}
	hexDump( dataPacket, packetLen );
	sendto( sTX, dataPacket, packetLen, 0, (struct sockaddr*)&addrTx, sizeof(addrTx) );
	currentBlockId++;
	return( packetLen );
}

void tFtpServerTask(void *pvParameters){ 
	int sRX=0, temp, tftpState=TFTP_OFF;
	uint8_t dataBuffer[TFTP_BUFFER_SIZE+1];
	char fileName[64], *modeName;
	struct sockaddr_in addrTemp = {
    	.sin_family      = AF_INET,
    	.sin_port        = htons(69),
    	.sin_addr.s_addr = htonl(INADDR_ANY)
    };
	socklen_t addrLen = sizeof(addrTemp);
	ESP_LOGI( T, "TFTPs erver started");
	// Open a UDP socket 69 for answering
	sTX = socket(AF_INET, SOCK_DGRAM, 0);
	if(sTX < 0) {
		ESP_LOGE(T, "... Failed to allocate sTX socket.");
		goto tftp_error;
	}
	// Open a UDP socket 69 for listening
	sRX = socket(AF_INET, SOCK_DGRAM, 0);
    if(sRX < 0) {
        ESP_LOGE(T, "... Failed to allocate sRX socket.");
        goto tftp_error;
    }
    temp = bind( sRX, (struct sockaddr *)&addrTemp, addrLen );
    if( temp < 0 ){
		ESP_LOGE(T, "bind failed: %d", temp);
		goto tftp_error;
    }
    ESP_LOGI(T, "... socket sRX is ready");
    tftpState = TFTP_IDLE;
    while( 1 ){
    	// We wait for the first received packet, which will set the IP to respond to
    	// Remote IP will be wrote to `addrTemp`
		switch( tftpState ){
			case TFTP_IDLE:
				ESP_LOGI( T, "TFTP_IDLE" );
				temp = recvfrom( sRX, dataBuffer, TFTP_BUFFER_SIZE, 0, (struct sockaddr *)&addrTemp, &addrLen );
				if(temp <= 0) {
					continue;
				}
				hexDump( dataBuffer, temp );
				memcpy( &addrTx, &addrTemp, addrLen );
				if( dataBuffer[1] == TFTP_OPCODE_RRQ ){
					sendNBytes = 1024;
					currentBlockId = 1;
					tftpState = TFTP_SEND;
				} else if ( dataBuffer[1]==TFTP_OPCODE_WRQ ) {
					tftpState = TFTP_RECEIVE;
				} else {
					sendError( ERROR_CODE_ILLEGAL_OPERATION, "only RRQ and WRQ" );
					tftpState = TFTP_IDLE;
					continue;
				}
				strcpy( fileName, (char*)&dataBuffer[2] );
				modeName = (char*)&dataBuffer[3+strlen(fileName)];
				if( strcmp("octet",modeName) != 0 ){
					sendError( ERROR_CODE_ILLEGAL_OPERATION, "only octet mode" );
					tftpState = TFTP_IDLE;
					continue;
				}
				ESP_LOGI( T, "IP: %s,  File: %s,  Mode: %s, State: %d", inet_ntoa(addrTx.sin_addr), fileName, modeName, tftpState );
			break;

			case TFTP_SEND:
				ESP_LOGI( T, "TFTP_SEND %d bytes", sendNBytes );
				memset( dataBuffer, 'a'-1+currentBlockId, TFTP_BUFFER_SIZE );
				if( currentBlockId == 1 ){
					temp = sprintf( (char*)dataBuffer, "Filename: %s\nExample File Payload ...\n", fileName );
					dataBuffer[temp] = 'a';
				}
				sendData( dataBuffer, &sendNBytes );
				tftpState = TFTP_SEND_WAITACK;
			break;

			case TFTP_SEND_WAITACK:
				ESP_LOGI( T, "TFTP_SEND_WAITACK" );
				temp = recvfrom( sRX, dataBuffer, TFTP_BUFFER_SIZE, 0, (struct sockaddr *)&addrTemp, &addrLen );
				if(temp <= 0) {
					continue;
				}
				hexDump( dataBuffer, temp );
			break;
		} 
		vTaskDelay(100/portTICK_PERIOD_MS);
    }
tftp_error:
	ESP_LOGI( T, "TFTP server stopping");
	vTaskDelete( NULL );
}

