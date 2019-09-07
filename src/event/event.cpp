#include "event/event.hpp"
namespace malice::event{
event::event(int fd, int flag){
    this->fd = fd;
    ev_ptr = std::make_shared<struct epoll_event>();
    ev_ptr->events=flag;
}

}
