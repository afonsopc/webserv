#ifdef __linux__

#include <sys/socket.h>
#include <sys/epoll.h>
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
    int epoll_fd = epoll_create1(0);
    struct epoll_event event, events[64];
    
    for (int i = 0; i < num_servers; i++)
    {
        event.events = EPOLLIN;
        event.data.fd = server_fds[i];
        epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fds[i], &event);
    }

    while (1)
    {
        int nfds = epoll_wait(epoll_fd, events, 64, -1);
        for (int i = 0; i < nfds; i++)
        {
            int is_server_socket = 0;
            for (int j = 0; j < num_servers; j++)
            {
                if (events[i].data.fd == server_fds[j])
                {
                    is_server_socket = 1;
                    break;
                }
            }
            
            if (is_server_socket)
            {
                int client_fd = accept(events[i].data.fd, NULL, NULL);
                if (client_fd >= 0)
                {
                    setNonBlocking(client_fd);
                    event.events = EPOLLIN;
                    event.data.fd = client_fd;
                    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event);
                }
            }
            else
            { //TODO add timeout to check for socket hang
                char buffer[4096]; //TODO recv in a loop
                ssize_t bytes = recv(events[i].data.fd, buffer, sizeof(buffer) - 1, MSG_DONTWAIT);
                if (bytes > 0) // TODO handle < 0 and == 0 early returns instead
                {
                    buffer[bytes] = '\0';
                    handler(events[i].data.fd, buffer, bytes);
                }
                close(events[i].data.fd); //TODO check for keep-alive before closing the socket
            }
        }
    }
    close(epoll_fd);
}

#endif
