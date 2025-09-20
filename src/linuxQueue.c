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

void start_event_loop(int server_fd, request_handler_t handler)
{
    int epoll_fd = epoll_create1(0);
    struct epoll_event event, events[64];
    event.events = EPOLLIN;
    event.data.fd = server_fd;
    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, server_fd, &event);

    while (1)
    {
        int nfds = epoll_wait(epoll_fd, events, 64, -1);
        for (int i = 0; i < nfds; i++)
        {
            if (events[i].data.fd == server_fd)
            {
                int client_fd = accept(server_fd, NULL, NULL);
                if (client_fd >= 0)
                {
                    setNonBlocking(client_fd);
                    event.events = EPOLLIN;
                    event.data.fd = client_fd;
                    epoll_ctl(epoll_fd, EPOLL_CTL_ADD, client_fd, &event);
                }
            }
            else
            {
                char buffer[4096];
                ssize_t bytes = recv(events[i].data.fd, buffer, sizeof(buffer) - 1, MSG_DONTWAIT);
                if (bytes > 0)
                {
                    buffer[bytes] = '\0';
                    handler(events[i].data.fd, buffer, bytes);
                }
                close(events[i].data.fd);
            }
        }
    }
    close(epoll_fd);
}

#endif
