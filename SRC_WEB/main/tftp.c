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
tftpBuf_t tftpBuf;

uint16_t swapBytes( uint16_t num ){
    return ((num>>8) | (num<<8));
}

void setHeader( uint16_t opCode, uint16_t blockId ){
    tftpBuf.dat.opCode =  swapBytes(opCode);
    tftpBuf.dat.blockId = swapBytes(blockId);
}

// Send a TFTP error response with user defined error code and message
void sendError( uint16_t errNum, const char* errString ){
	ESP_LOGE(T, "TFTP error %s", errString );
	setHeader( TFTP_OPCODE_ERROR, errNum );
	strncpy((char*)tftpBuf.dat.payload, errString, 32 );
	send( sockHandle, tftpBuf.b, ERR_PACKET_LEN, 0 );
}

// Sends a data block (<= 512 bytes) returns how many bytes have been sent
int sendData( uint8_t *buff, uint16_t len, uint16_t blockId ){
    int retVal;
    setHeader( TFTP_OPCODE_DATA, blockId );
    if( buff > (uint8_t*)0 ){
        memcpy( tftpBuf.dat.payload, buff, len );
    }
	retVal = send( sockHandle, tftpBuf.b, len+4, 0 );
    if( retVal < 0 ){
        ESP_LOGE(T, "%d = send(): %s", retVal, strerror(errno) );
        return( retVal );
    }
	return( retVal-4 );
}

// Check if buffer contains a valid tftp packet
// Checks opcode and blockid
// Returns 1 if valid
uint8_t isValidPacket( int32_t rxLen, uint16_t expBlockId, uint16_t expOpCode ){
    // Loop until a UDP packet is received
    if( rxLen < 4 ) {
        return 0;
    }
    // Check its opcode
    if( tftpBuf.dat.opCode != swapBytes(expOpCode) ){
        return 0;
    }
    // Check its blockId
    if( tftpBuf.dat.blockId != swapBytes(expBlockId) ){
        return 0;
    }
    return 1;
}

// Sends a ack packet with `blcokId`
void sendAck( uint16_t blockId ){
    setHeader( TFTP_OPCODE_ACK, blockId );
    send( sockHandle, tftpBuf.b, ACK_PACKET_LEN, 0 );
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
    tv.tv_sec = 0;   // 3 Secs Timeout 
    tv.tv_usec = TFTP_TIMEOUT_USEC; // Not init'ing this can cause strange errors
    ESP_ERROR_CHECK( setsockopt( sockHandle, SOL_SOCKET, SO_RCVTIMEO, (const char*)&tv,sizeof(struct timeval) ) );
    ESP_LOGI(T, "... socket %d is ready", sockHandle);
    return sockHandle;
}

#define MIN(a,b) ((a>b)?b:a)

int getHeaderInfo( char *fileNameBuffer, int rxLen ){
    char *rBuff=(char*)&tftpBuf.b[2];
    int temp, blksize=512;
    if( rxLen < 5 ){
        sendError( ERROR_CODE_FILE_NOT_FOUND, "fileName?" );
        return -1;
    }
    temp = strlcpy( fileNameBuffer, rBuff, FILE_NAME_LEN );
    rxLen -= temp;
    rBuff += temp+1;
    if( rxLen<=6 || strcmp("octet",rBuff)!=0 ){
        sendError( ERROR_CODE_ILLEGAL_OPERATION, "only octet mode!" );
        return -1;
    }
    rBuff += 6;
    rxLen -= 6;
    if( rxLen>0 && strcmp("blksize",rBuff)==0 ){
        rBuff += 8;
        rxLen -= 8;
        blksize = atoi( rBuff );
        if( blksize < 8 || blksize > MAX_PAYLOAD_LEN ){
            blksize = 512;
        }
    }
    return blksize;
}

void tFtpServerTask(void *pvParameters){ 
	int rxLen, temp=0, plSent=0;
    uint32_t sendNBytes;
    uint16_t opCode, currentBlockId, resendCounter, blockSize;
	char fileName[FILE_NAME_LEN];
	struct sockaddr_in addrTemp;
    enum TFTP_STATE tftpState = TFTP_OFF;
	socklen_t addrLenTemp = sizeof( addrTemp );
	ESP_LOGI( T, "TFTPs erver started");
    while( 1 ){
        // Loop forever statemachine
        if ( tftpState==TFTP_IDLE || tftpState==TFTP_SEND_WAITACK || tftpState==TFTP_RECEIVE ) {
            rxLen = recvfrom( sockHandle, tftpBuf.b, DATA_PACKET_LEN, 0, (struct sockaddr *)&addrTemp, &addrLenTemp );
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
                // `addrTemp` will be the only address from which datagrams are received and sent to by default
                temp = connect(sockHandle, (struct sockaddr *)&addrTemp, addrLenTemp);
                ESP_LOGI(T,"%d = connect(%s:%d)", temp, inet_ntoa(addrTemp.sin_addr), addrTemp.sin_port );
                if ( temp < 0 ){
                    ESP_LOGE(T, "%s", strerror(errno) );
                    tftpState = TFTP_OFF;
                    continue;
                }
                opCode = swapBytes( tftpBuf.dat.opCode );
                if( opCode==TFTP_OPCODE_RRQ ){
                    tftpState = TFTP_SEND;
                    sendNBytes = 1024*1024*4;
                    currentBlockId = 1;
                    ESP_LOGI( T, "TFTP_SEND %d bytes", sendNBytes );
                } else if ( opCode==TFTP_OPCODE_WRQ ){
                    tftpState = TFTP_RECEIVE;
                    currentBlockId = 1;
                } else {
                    sendError( ERROR_CODE_ILLEGAL_OPERATION, "only RRQ and WRQ" );
                    tftpState = TFTP_OFF;       // Get a new socket
                    continue;
                }
                if( (blockSize = getHeaderInfo( fileName, rxLen )) < 1 ){
                    tftpState = TFTP_OFF;       // Get a new socket
                    continue;   
                }
                if( blockSize != 512 ){          // Need to send an optack
                    // Send the optack
                    setHeader( TFTP_OPCODE_OPACK, 0 );
                    temp = sprintf( (char*)&tftpBuf.b[2], "blksize %d", blockSize );
                    ESP_LOGI(T,"sending opack: %s", (char*)&tftpBuf.b[2] );
                    tftpBuf.b[9] = '\0';
                    send( sockHandle, tftpBuf.b, temp+2, 0 );
                    currentBlockId = 0;
                    plSent = 0;
                    tftpState = TFTP_SEND_WAITACK;
                }
                ESP_LOGI( T, "File: %s,  State: %d", fileName, tftpState );
                break;
        //-------------------------------------------------
        case TFTP_SEND:
        //-------------------------------------------------
            // second condition is for notifying end of transmission with an empty packet
            // Loop until all payload data is sent
            // TODO read payload data from flash
            // For now, generate some fake data which includes the filename
            memset( tftpBuf.dat.payload, currentBlockId, blockSize );
            if( currentBlockId == 1 ){
                temp = sprintf( (char*)tftpBuf.dat.payload, "Filename: %s\nExample File Payload ...\n", fileName );
            }
            plSent = sendData( 0, MIN(sendNBytes,blockSize), currentBlockId );
            resendCounter = 0;
            tftpState = TFTP_SEND_WAITACK;
            break;
        //-------------------------------------------------
        case TFTP_SEND_WAITACK:
        //-------------------------------------------------
            // Check if we received a valid ACK after sending a data block
            if( isValidPacket( rxLen, currentBlockId, TFTP_OPCODE_ACK ) ){
                currentBlockId++;
                sendNBytes -= plSent;
                if( sendNBytes<=0 && plSent!=blockSize ){
                    ESP_LOGI(T,"DONE transf");
                    tftpState = TFTP_OFF;
                } else {
                    tftpState = TFTP_SEND;
                }
            } else {
                // If not (or timeout) resend current data block
                if ( resendCounter <= MAX_RESENDS ){
                    ESP_LOGW(T,"SEND_WAITACK: %s, Resend %d ...", strerror(errno), resendCounter );
                    plSent = sendData( 0, sendNBytes, currentBlockId );
                    resendCounter++;
                } else {
                    ESP_LOGE(T,"SEND_WAITACK: %s, back to Start", strerror(errno) );
                    tftpState = TFTP_OFF;
                }
            }
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
            if( !isValidPacket( rxLen, currentBlockId, TFTP_OPCODE_DATA ) ){
                // Might be caused by a previously lost ack
                continue;
            }
            hexDump( &tftpBuf.b[4], rxLen-4 );
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

