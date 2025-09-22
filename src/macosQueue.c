#ifdef __APPLE__

#include <sys/socket.h>
#include <sys/event.h>
#include <unistd.h>
#include <fcntl.h>
#include "queue.h"

int setNonBlocking(int fd)
{
    int flags = fcntl(fd, F_GETFL, 0);
    return (fcntl(fd, F_SETFL, flags | O_NONBLOCK));
}

void start_multi_server_event_loop(int *server_fds, int num_servers, request_handler_t handler)
{
    int kq = kqueue();
    struct kevent change, events[64];
    
    for (int i = 0; i < num_servers; i++)
    {
        EV_SET(&change, server_fds[i], EVFILT_READ, EV_ADD, 0, 0, NULL);
        kevent(kq, &change, 1, NULL, 0, NULL);
    }

    while (1)
    {
        int nev = kevent(kq, NULL, 0, events, 64, NULL);
        for (int i = 0; i < nev; i++)
        {
            int is_server_socket = 0;
            for (int j = 0; j < num_servers; j++)
            {
                if ((int)events[i].ident == server_fds[j])
                {
                    is_server_socket = 1;
                    break;
                }
            }
            
            if (is_server_socket)
            {
                int client_fd = accept(events[i].ident, NULL, NULL);
                if (client_fd >= 0)
                {
                    setNonBlocking(client_fd);
                    EV_SET(&change, client_fd, EVFILT_READ, EV_ADD, 0, 0, NULL);
                    kevent(kq, &change, 1, NULL, 0, NULL);
                }
            }
            else
            { //TODO add timeout to check for socket hang
                char buffer[4096]; //TODO recv in a loop
                ssize_t bytes = recv(events[i].ident, buffer, sizeof(buffer) - 1, MSG_DONTWAIT);
                if (bytes > 0) // TODO handle < 0 and == 0 early returns instead
                {
                    buffer[bytes] = '\0';
                    handler(events[i].ident, buffer, bytes);
                }
                close(events[i].ident); //TODO check for keep-alive before closing the socket
            }
        }
    }
    close(kq);
}

#endif
