#ifndef WCONSOLE_H
#define WCONSOLE_H

// Minimum history size for log entries in bytes
#define LOG_FILE_SIZE 1024*2

// Initialize and mount SPIFFS (format if needed)
// run at boot time
extern void initFs();

// register this with esp_log_set_vprintf()
// will route log output to:
//   * websocket
//   * alternating log files on spiffs
//   * uart
extern int wsDebugPrintf( const char *format, va_list arg );

// print a file on spiffs to uart
extern void printFile( const char *fName );

#endif