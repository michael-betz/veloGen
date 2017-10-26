#ifndef TFTP_H
#define TFTP_H

enum OPCODE {
	TFTP_OPCODE_RRQ   = 1, // Read request
	TFTP_OPCODE_WRQ   = 2, // Write request
	TFTP_OPCODE_DATA  = 3, // Data
	TFTP_OPCODE_ACK   = 4, // Acknowledgement
	TFTP_OPCODE_ERROR = 5  // Error
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

enum TFTP_STATE {
	TFTP_OFF, TFTP_IDLE, TFTP_RECEIVE, TFTP_SEND, TFTP_SEND_WAITACK
};

#define TFTP_PORT 			69
#define TFTP_TIMEOUT_SEC    3
#define MAX_PAYLOAD_LEN     1024
#define ERR_PACKET_LEN 		32+5
#define DATA_PACKET_LEN 	MAX_PAYLOAD_LEN+4
#define ACK_PACKET_LEN 		4

extern void tFtpServerTask(void *pvParameters);

#endif
