#ifndef APP_MAIN_H
#define APP_MAIN_H
#include "libesphttpd/cgiwebsocket.h"

#define WS_ID_WEBCONSOLE 'a'
#define WS_ID_LOGLEVEL   'l'
#define WS_ID_FONT       'f'
#define WS_ID_MAXFONT    'g'


#define MIN(a,b) (a<b?a:b)
#define MAX(a,b) (a>b?a:b)
#define pi M_PI

// Random number within the range [a,b]
#define RAND_AB(a,b) (rand()%(b+1-a)+a)

// connection to app websocket happened
void wsAppConnect(Websock *ws);

#endif
