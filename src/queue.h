#ifndef QUEUE_H
#define QUEUE_H

#include <stddef.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef void (*request_handler_t)(int client_fd, char *buffer, size_t bytes_read);
int setNonBlocking(int fd);
void start_event_loop(int server_fd, request_handler_t handler);

#ifdef __cplusplus
}
#endif

#endif
