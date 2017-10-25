#include <string.h>
#include <errno.h>
#include "esp_log.h"
#include "esp_err.h"
#include <lwip/err.h>
#include <lwip/sockets.h>
#include <lwip/sys.h>
#include <lwip/netdb.h>
#include "app_main.h"
#include "wifi_startup.h"
#include "tftp.h"

static const char *T = "TFTP";

int sockHandle=0;		 // UDP socket handle
uint16_t sendNBytes, currentBlockId;

// Send a TFTP error response with user defined error code and message
void sendError( uint16_t errNum, const char* errString ){
	uint8_t errPacket[ERR_PACKET_LEN];
	ESP_LOGE(T, "TFTP error %s", errString );
	memset( errPacket, 0, ERR_PACKET_LEN );
	errPacket[1] = TFTP_OPCODE_ERROR;
	errPacket[2] = errNum>>8;
	errPacket[3] = errNum;
	strncpy((char*)&errPacket[4], errString, 32 );
	hexDump( errPacket, ERR_PACKET_LEN );
	send( sockHandle, errPacket, ERR_PACKET_LEN, 0 );
}

// Sends a data block (<= 512 bytes) returns how many bytes have been sent, decrements len
uint16_t sendData( uint8_t *buff, uint16_t *len, uint16_t blockId ){
	uint16_t packetLen = 4;
	uint8_t dataPacket[DATA_PACKET_LEN];
	uint8_t *wp=dataPacket;
	*wp++ = 0;
	*wp++ = TFTP_OPCODE_DATA;
	*wp++ = blockId>>8;
	*wp++ = blockId;
	for( uint16_t i=0; i<512; i++ ){
		if( *len <= 0 ){
			break;
		}
		*wp++ = *buff++;
		packetLen++;
		*len -= 1;
	}
	hexDump( dataPacket, packetLen );
	packetLen = send( sockHandle, dataPacket, packetLen, 0 );
	return( packetLen-4 );
}

uint16_t getUint( uint8_t *buf ){
    return ( (buf[1]<<8) | buf[0] );
}

// Blocks until valid tftp packet is received, stores data in dataBuffer (including header), returns number of bytes received.
// TODO, implement a timeout, closing the socket and doing a `continue` on the big loop
uint16_t waitForData( uint8_t *dataBuffer, uint16_t bufferSize, uint16_t expBlockId, uint16_t expOpCode ){
    while(1){
        // Loop until a UDP packet is received
        int temp = recv( sockHandle, dataBuffer, bufferSize, 0 );
        if( temp < 4 ) {
            continue;
        }
        // Check its opcode
        if( getUint(dataBuffer) != expOpCode ){
            continue;
        }
        // Check its blockId
        if( getUint(&dataBuffer[2]) == expBlockId ){
            return temp;
        }
    }
}

// Sends a ack packet with `blcokId`
void sendAck( uint16_t blockId ){
    uint8_t ackPacket[ACK_PACKET_LEN];
    ackPacket[0] = 0;
    ackPacket[1] = TFTP_OPCODE_ACK;
    ackPacket[2] = blockId>>8;
    ackPacket[3] = blockId;
    send( sockHandle, ackPacket, ACK_PACKET_LEN, 0 );
}

// Returns a socket handle to an open upd port 69 listener
int generateListenerSocket(){
    int sockHandle;
    struct sockaddr_in addrTemp = {
        .sin_family      = AF_INET,
        .sin_port        = htons( TFTP_PORT ),
        .sin_addr.s_addr = htonl( INADDR_ANY )
    };
    // Open a UDP socket for listening and answering
    sockHandle =  socket(AF_INET, SOCK_DGRAM, 0);
    ESP_ERROR_CHECK( bind( sockHandle, (struct sockaddr *)&addrTemp, sizeof(addrTemp) ) );
    // Set timeout parameter for socket
    struct timeval tv;
    tv.tv_sec = TFTP_TIMEOUT_SEC;   // 3 Secs Timeout 
    tv.tv_usec = 0;                 // Not init'ing this can cause strange errors
    setsockopt( sockHandle, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv,sizeof(struct timeval) );
    ESP_LOGI(T, "... socket %d is ready", sockHandle);
    return sockHandle;
}

void tFtpServerTask(void *pvParameters){ 
	int temp=0, sockHandle=0, plSent=0;
    uint16_t opCode;
	uint8_t dataBuffer[TFTP_BUFFER_SIZE+1];
	char fileName[64], *modeName;
	struct sockaddr_in addrTemp;
	socklen_t addrLenTemp;
	ESP_LOGI( T, "TFTPs erver started");
    while( 1 ){
        // Loop forever statemachine
        // this is all implemented with a stack of blocking while loops.
        // In a sense the program counter holds the current state ;)
        //
        // TODO add timeouts to all recvfrom()
        //
        //-------------------------------------------------
        // IDLE state
        //-------------------------------------------------
        // We wait for the first received packet, which will set the IP to respond to
        // Remote IP will be wrote to `addrTemp`
        if( sockHandle != 0 ){
            close( sockHandle );
        }
        // Open a UDP socket for listening and answering
        sockHandle = generateListenerSocket();
        ESP_LOGI( T, "TFTP_IDLE" );
        // Loop until a valid RRQ or WRQ request is received
        temp = recvfrom( sockHandle, dataBuffer, TFTP_BUFFER_SIZE, 0, (struct sockaddr *)&addrTemp, &addrLenTemp );
        if( temp<=4 ) {
            continue;
        }
        opCode = getUint( dataBuffer );
        // `addrTemp` will be the only address from which datagrams are received and sent to by default
        connect( sockHandle, (struct sockaddr *)&addrTemp, addrLenTemp );
        if( opCode!=TFTP_OPCODE_RRQ && opCode!=TFTP_OPCODE_RRQ ){
            sendError( ERROR_CODE_ILLEGAL_OPERATION, "only RRQ and WRQ" );
            continue;
        }
        hexDump( dataBuffer, temp );
        strcpy( fileName, (char*)&dataBuffer[2] );
        modeName = (char*)&dataBuffer[3+strlen(fileName)];
        if( strcmp("octet",modeName) != 0 ){
            sendError( ERROR_CODE_ILLEGAL_OPERATION, "only octet mode" );
            continue;
        }
        ESP_LOGI( T, "IP: %s,  File: %s,  Mode: %s", inet_ntoa(addrTemp.sin_addr), fileName, modeName );
        if( opCode == TFTP_OPCODE_RRQ ) {
            //-------------------------------------------------
            // SEND state
            //-------------------------------------------------
            sendNBytes = 1024;
            currentBlockId = 1;
            ESP_LOGI( T, "TFTP_SEND %d bytes", sendNBytes );
            // second condition is for notifying end of transmission with an empty packet
            while( sendNBytes>0 || plSent==512 ){ 
                // Loop until all payload data is sent
                // TODO read payload data from flash
                // For now, generate some fake data which includes the filename
                memset( dataBuffer, 'a'-1+currentBlockId, TFTP_BUFFER_SIZE );
                if( currentBlockId == 1 ){
                    temp = sprintf( (char*)dataBuffer, "Filename: %s\nExample File Payload ...\n", fileName );
                    dataBuffer[temp] = 'a';
                }
                plSent = sendData( dataBuffer, &sendNBytes, currentBlockId );
                waitForData( dataBuffer, TFTP_BUFFER_SIZE, currentBlockId, TFTP_OPCODE_ACK );
                currentBlockId++;
            }
        } else if ( opCode == TFTP_OPCODE_WRQ ) {
            //-------------------------------------------------
            // RECEIVE state
            //-------------------------------------------------
            ESP_LOGI( T, "Starting to receive data ... " );
            currentBlockId = 0;
            sendAck( currentBlockId++ );
            temp = 512;
            while( temp >= 512 ){
                temp = waitForData( dataBuffer, TFTP_BUFFER_SIZE, currentBlockId, TFTP_OPCODE_DATA );
                hexDump( dataBuffer, temp );
                sendAck( currentBlockId++ );
            }
            ESP_LOGI( T, "DONE!");
        }
    } // loop forever
	ESP_LOGI( T, "TFTP server stopping");
	vTaskDelete( NULL );
}

