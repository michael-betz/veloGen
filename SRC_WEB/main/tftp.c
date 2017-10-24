#include <string.h>
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
uint16_t sendData( uint8_t *buff, uint16_t *len ){
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
	packetLen = send( sockHandle, dataPacket, packetLen, 0 );
	currentBlockId++;
	return( packetLen-4 );
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

// Blocks until a ACK packet with corresponding `blockId` is received
void waitForAck( uint16_t blockId ){
    uint8_t ackPacket[ACK_PACKET_LEN];
    uint16_t receivedBlockId;
    while(1){
        // Loop until a valid ACK packet is received
        int temp = recv( sockHandle, ackPacket, ACK_PACKET_LEN, 0 );
        if(temp != 4) {
            continue;
        }
        if( ackPacket[0]!=0 || ackPacket[1]!=TFTP_OPCODE_ACK ){
            continue;
        }
        receivedBlockId = ( ackPacket[2]<<8 | ackPacket[3] );
        if( receivedBlockId == blockId ){
            return;
        }
    }
}

int generateListenerSocket(){
    int temp;
    struct sockaddr_in addrTemp = {
        .sin_family      = AF_INET,
        .sin_port        = htons( TFTP_PORT ),
        .sin_addr.s_addr = htonl( INADDR_ANY )
    };
    // Open a UDP socket for listening and answering
    sockHandle = ESP_ERROR_CHECK( socket(AF_INET, SOCK_DGRAM, 0) );
    temp       = ESP_ERROR_CHECK( bind( sockHandle, (struct sockaddr *)&addrTemp, sizeof(addrTemp) ) );
    ESP_LOGI(T, "... socket %d is ready", temp);
    return temp;
}

void tFtpServerTask(void *pvParameters){ 
	int temp=0, sockHandle=0;
	uint8_t dataBuffer[TFTP_BUFFER_SIZE+1];
	char fileName[64], *modeName;
	struct sockaddr_in addrTemp;
	socklen_t addrLenTemp;
	ESP_LOGI( T, "TFTPs erver started");
    while( 1 ){
        if( sockHandle != 0 ){
            close( sockHandle );
        }
        // Open a UDP socket for listening and answering
        sockHandle = generateListenerSocket();
        // Loop forever statemachine
        // this is all implemented with stacked blocking while loops.
        // In a sense the program counter holds the current state ;)
        //
        // TODO add timeouts to all recvfrom()
        //
    	//-------------------------------------------------
        // IDLE state
    	//-------------------------------------------------
        // We wait for the first received packet, which will set the IP to respond to
    	// Remote IP will be wrote to `addrTemp`
        ESP_LOGI( T, "TFTP_IDLE" );
        while( 1 ){
            // Loop until a valid RRQ or WRQ request is received
            temp = recvfrom( sockHandle, dataBuffer, TFTP_BUFFER_SIZE, 0, (struct sockaddr *)&addrTemp, &addrLen )
            // addrTemp contains the IP of the remote client
            if( temp<=4 || dataBuffer[0]!=0 ) {
                continue;
            }
            if( !( dataBuffer[1]==TFTP_OPCODE_RRQ || dataBuffer[1]==TFTP_OPCODE_RRQ ) ){
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
            break;
        }            
        // Now `addrTemp` is the only address from which datagrams are received and sent to by default
        connect( sockHandle, (struct sockaddr *)&addrTemp, &addrLenTemp );
        ESP_LOGI( T, "IP: %s,  File: %s,  Mode: %s", inet_ntoa(addrTemp.sin_addr), fileName, modeName );

        if( dataBuffer[1] == TFTP_OPCODE_RRQ ) {
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
                int16_t plSent = sendData( dataBuffer, &sendNBytes );
                waitForAck( currentBlockId );
            }
        } else if ( dataBuffer[1] == TFTP_OPCODE_RRQ ) {
            //-------------------------------------------------
            // RECEIVE state
            //-------------------------------------------------
            currentBlockId = 1;
            sendAck( currentBlockId );
            //...
        }
    } // loop forever
	ESP_LOGI( T, "TFTP server stopping");
	vTaskDelete( NULL );
}

