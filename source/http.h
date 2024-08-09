#ifndef HTTP_H
#define HTTP_H

#include <winyl/request.h>

winyl_response get_http(char *url, int port, char *path);

#endif
