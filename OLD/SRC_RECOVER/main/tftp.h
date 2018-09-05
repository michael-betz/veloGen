#ifndef TFTP_H
#define TFTP_H

#define TFTP_PORT 			69
#define TFTP_TIMEOUT_USEC   500*1000
#define MAX_PAYLOAD_LEN     2048
#define MAX_RESENDS         3
#define ERR_PACKET_LEN 		32+5
#define DATA_PACKET_LEN 	MAX_PAYLOAD_LEN+4
#define ACK_PACKET_LEN 		4
#define FILE_NAME_LEN		64

#define MIN(a,b) ((a>b)?b:a)

enum OPCODE {
	TFTP_OPCODE_RRQ   = 1, // Read request
	TFTP_OPCODE_WRQ   = 2, // Write request
	TFTP_OPCODE_DATA  = 3, // Data
	TFTP_OPCODE_ACK   = 4, // Acknowledgement
	TFTP_OPCODE_ERROR = 5, // Error
	TFTP_OPCODE_OPACK = 6  // Option ack
};

enum ERRORCODE {
	ERROR_CODE_NOTDEFINED        = 0,
	ERROR_CODE_FILE_NOT_FOUND    = 1,
	ERROR_CODE_ACCESS_VIOLATION  = 2,
	ERROR_CODE_NO_SPACE          = 3,
	ERROR_CODE_ILLEGAL_OPERATION = 4,
	ERROR_CODE_UNKNOWN_ID        = 5,
	ERROR_CODE_FILE_EXISTS       = 6,
	ERROR_CODE_UNKNOWN_USER      = 7
};

typedef struct {
    uint16_t opCode;
    uint16_t blockId;
    uint8_t  payload[MAX_PAYLOAD_LEN];
} tftpHeader_t;

typedef union {
    uint8_t      b[DATA_PACKET_LEN];
    tftpHeader_t dat;
} tftpBuf_t;

enum TFTP_STATE {
	TFTP_OFF, TFTP_IDLE, TFTP_RECEIVE, TFTP_SEND, TFTP_SEND_WAITACK
};

extern void tFtpServerTask(void *pvParameters);

// will give you requested filename and operation (read/write). 
// Return <0 to reject it.
// Otherwise return number of bytes to transfer (only needed for outgoing transfer)
extern int conSetupCb( char* fName, uint16_t opCode );

// will give you the received payload. Return <0 to cancel the transfer.
int rxPayloadCb( uint8_t *buff, int buffLen );

// int txPayloadCb( uint8_t *buff, int buffLen );

#endif
