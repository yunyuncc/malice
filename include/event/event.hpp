#pragma once
#include <sys/epoll.h>
#include <memory>
namespace malice::event{
class event{
    event(int fd, int flag);
private:
    int fd;
    std::shared_ptr<struct epoll_event> ev_ptr;
};
}
