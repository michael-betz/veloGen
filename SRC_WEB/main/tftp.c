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
uint8_t g_dataBuffer[DATA_PACKET_LEN+1];

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

// Sends a data block (<= 512 bytes) returns how many bytes have been sent
int sendDataBuffer( uint32_t len, uint16_t blockId ){
	int packetLen = 4;
	uint8_t *wp=g_dataBuffer;
	*wp++ = 0;
	*wp++ = TFTP_OPCODE_DATA;
	*wp++ = blockId>>8;
	*wp++ = blockId;
	//for( uint16_t i=0; i<512; i++ ){
	//	if( len <= 0 ){
	//		break;
	//	}
	//	*wp++ = *buff++;
	//	packetLen++;
	//	len--;
    //}
    if( len > MAX_PAYLOAD_LEN ){
        len = MAX_PAYLOAD_LEN;
    }
    packetLen = len + 4;
	packetLen = send( sockHandle, g_dataBuffer, packetLen, 0 );
    if( packetLen < 0 ){
        ESP_LOGE(T, "%d = send(): %s", packetLen, strerror(errno) );
        return( packetLen );
    }
	return( packetLen-4 );
}

uint16_t getUint( uint8_t *buf ){
    return ( (buf[0]<<8) | buf[1] );
}

// Check if buffer contains a valid tftp packet
// Checks opcode and blockid
// Returns 1 if valid
uint8_t isValidPacket( uint8_t *dataBuffer, uint16_t bufferSize, uint16_t expBlockId, uint16_t expOpCode ){
    // Loop until a UDP packet is received
    if( bufferSize < 4 ) {
        return 0;
    }
    // Check its opcode
    if( getUint(dataBuffer) != expOpCode ){
        return 0;
    }
    // Check its blockId
    if( getUint(&dataBuffer[2]) != expBlockId ){
        return 0;
    }
    return 1;
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
    ESP_ERROR_CHECK( setsockopt( sockHandle, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv,sizeof(struct timeval) ) );
    ESP_LOGI(T, "... socket %d is ready", sockHandle);
    return sockHandle;
}

void tFtpServerTask(void *pvParameters){ 
	int rxLen, temp=0, plSent=0;
    uint32_t sendNBytes;
    uint16_t opCode, currentBlockId;
	char fileName[64], *modeName;
	struct sockaddr_in addrTemp;
    enum TFTP_STATE tftpState = TFTP_OFF;
	socklen_t addrLenTemp = sizeof( addrTemp );
	ESP_LOGI( T, "TFTPs erver started");
    while( 1 ){
        // Loop forever statemachine
        if ( tftpState==TFTP_IDLE || tftpState==TFTP_SEND_WAITACK || tftpState==TFTP_RECEIVE ) {
            rxLen = recvfrom( sockHandle, g_dataBuffer, DATA_PACKET_LEN, 0, (struct sockaddr *)&addrTemp, &addrLenTemp );
        }
        switch( tftpState ){
            //-------------------------------------------------
            case TFTP_OFF:
            //-------------------------------------------------
                // We need to setup the listening socket 
                if( sockHandle != 0 ){
                    ESP_ERROR_CHECK( close( sockHandle ) );
                }
                sockHandle = generateListenerSocket();
                rxLen = 0;
                tftpState = TFTP_IDLE;
                break;
            //-------------------------------------------------
            case TFTP_IDLE:
            //-------------------------------------------------
                // wait for the first received packet, which will set the IP to respond to
                if ( rxLen < 4 ) {
                    // Timeout / random packet, try again
                    continue;
                }
                opCode = getUint( g_dataBuffer );
                // `addrTemp` will be the only address from which datagrams are received and sent to by default
                temp = connect(sockHandle, (struct sockaddr *)&addrTemp, addrLenTemp);
                ESP_LOGI(T,"%d = connect(%s:%d)", temp, inet_ntoa(addrTemp.sin_addr), addrTemp.sin_port );
                if ( temp < 0 ){
                    ESP_LOGE(T, "%s", strerror(errno) );
                    continue;
                }
                if( opCode==TFTP_OPCODE_RRQ ){
                    tftpState = TFTP_SEND;
                    currentBlockId = 1;
                    sendNBytes = 1024*1024*4;
                    ESP_LOGI( T, "TFTP_SEND %d bytes", sendNBytes );
                } else if ( opCode==TFTP_OPCODE_WRQ ){
                    tftpState = TFTP_RECEIVE;
                    currentBlockId = 1;
                } else {
                    sendError( ERROR_CODE_ILLEGAL_OPERATION, "only RRQ and WRQ" );
                    tftpState = TFTP_OFF;       // Get a new socket
                    continue;
                }
                // TODO parse blocksize
                strcpy( fileName, (char*)&g_dataBuffer[2] );
                modeName = (char*)&g_dataBuffer[3+strlen(fileName)];
                if( strcmp("octet",modeName) != 0 ){
                    sendError( ERROR_CODE_ILLEGAL_OPERATION, "only octet mode" );
                    tftpState = TFTP_OFF;       // Get a new socket
                    continue;
                }
                ESP_LOGI( T, "File: %s,  Mode: %s,  State: %d", fileName, modeName, tftpState );
                break;
        //-------------------------------------------------
        case TFTP_SEND:
        //-------------------------------------------------
            // second condition is for notifying end of transmission with an empty packet
            // Loop until all payload data is sent
            // TODO read payload data from flash
            // For now, generate some fake data which includes the filename
            // Generate some test data
            memset( &g_dataBuffer[4], currentBlockId, MAX_PAYLOAD_LEN );
            if( currentBlockId == 1 ){
                temp = sprintf( (char*)g_dataBuffer, "Filename: %s\nExample File Payload ...\n", fileName );
                g_dataBuffer[temp] = 'a';
            }
            plSent = sendDataBuffer( sendNBytes, currentBlockId );
            tftpState = TFTP_SEND_WAITACK;
            break;
        //-------------------------------------------------
        case TFTP_SEND_WAITACK:
        //-------------------------------------------------
            if( rxLen < 0 ){
                ESP_LOGE(T,"SEND_WAITACK: %s, back to IDLE ...", strerror(errno) );
                tftpState = TFTP_OFF;
                continue;
            }
            // Check if we received a valid ACK after sending a data block
            if( isValidPacket( g_dataBuffer, rxLen, currentBlockId, TFTP_OPCODE_ACK ) ){
                currentBlockId++;
                sendNBytes -= plSent;
                if( sendNBytes<=0 && plSent!=512 ){
                    ESP_LOGI(T,"DONE transf");
                    tftpState = TFTP_OFF;
                    continue;
                }
            }
            // If not (or timeout) resend current data block
            tftpState = TFTP_SEND;
            break;
        //-------------------------------------------------
        case TFTP_RECEIVE:
        //-------------------------------------------------
            if( rxLen < 4 ){
                //maybe timeout
                ESP_LOGE( T, "TFTP_RECEIVE: rxLen = %d, %s", rxLen, strerror(errno) );
                tftpState = TFTP_OFF;
                continue;
            }
            sendAck( currentBlockId );
            if( !isValidPacket( g_dataBuffer, rxLen, currentBlockId, TFTP_OPCODE_DATA ) ){
                // Might be caused by a previously lost ack
                continue;
            }
            hexDump( &g_dataBuffer[4], rxLen-4 );
            if( rxLen-4 >= 512 ){
                currentBlockId++;
            } else {
                // THis was the last block, we are done
                tftpState = TFTP_OFF;
                ESP_LOGI( T, "DONE!");
            }
            break;
        //-------------------------------------------------
        default:
        //-------------------------------------------------
            tftpState = TFTP_OFF;
        } // switch
    } // while
	ESP_LOGI( T, "TFTP server stopping");
	vTaskDelete( NULL );
}

