#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "doctest.h"
#include "event/event.hpp"
using namespace malice::event;
TEST_CASE("ev_str") {
  CHECK(ev_str(EPOLLIN) == "EPOLLIN");
  CHECK(ev_str(EPOLLIN | EPOLLOUT) == "EPOLLIN|EPOLLOUT");
  CHECK(ev_str(EPOLLIN | EPOLLOUT | EPOLLERR) == "EPOLLIN|EPOLLOUT|EPOLLERR");
  CHECK(ev_str(0) == "");
}

TEST_CASE("event handler") {
  SUBCASE("one event") {
    std::shared_ptr<event> ev(new event(0, EPOLLIN));

    SUBCASE("one handler") {
      int counter = 0;
      ev->set_handler(EPOLLIN, [&counter](event *e) {
        counter++;
        int flag = e->get_flag();
        CHECK((flag & EPOLLIN) == true);
      });
      CHECK(counter == 0);
      ev->fire();
      CHECK(counter == 1);
    }
  }
  SUBCASE("two event") {
    std::shared_ptr<event> ev(new event(0, EPOLLIN | EPOLLOUT));
    SUBCASE("one handler") {
      int c = 0;
      ev->set_handler(EPOLLIN, [&c](event *e) {
        int flag = e->get_flag();
        CHECK((flag & EPOLLIN));
        CHECK((flag & EPOLLOUT));
        CHECK(flag == (EPOLLIN | EPOLLOUT));
        c++;
      });
      CHECK(c == 0);
      ev->fire();
      CHECK(c == 1);
    }
    SUBCASE("two handler") {
      int c = 0;
      ev->set_handler(EPOLLIN, [&c](event *e) {
        int flag = e->get_flag();
        CHECK((flag & EPOLLIN));
        CHECK((flag & EPOLLOUT));
        CHECK(flag == (EPOLLIN | EPOLLOUT));
        c++;
      });
      ev->set_handler(EPOLLOUT, [&c](event *e) {
        int flag = e->get_flag();
        CHECK((flag & EPOLLIN));
        CHECK((flag & EPOLLOUT));
        CHECK(flag == (EPOLLIN | EPOLLOUT));
        c++;
      });

      CHECK(c == 0);
      ev->fire();
      CHECK(c == 2);
    }
  }
}
TEST_CASE("native_handle") {
  event ev(0, EPOLLIN);
  struct epoll_event *eh = ev.native_handle();
  CHECK(eh != nullptr);
  event *evp = to_event(eh);
  CHECK(evp == &ev);
}
TEST_CASE("set_flag") {
  event ev(0, EPOLLIN);
  auto flag = ev.get_flag();
  flag |= EPOLLOUT;
  ev.set_flag(flag);
  CHECK(ev_str(ev.get_flag()) == "EPOLLIN|EPOLLOUT");
}
TEST_CASE("set event_handler fail") {
  auto ev = std::make_shared<event>(0, EPOLLIN);
  ev->set_handler(EPOLLIN, nullptr);
  try {
    ev->set_handler(EPOLLIN | EPOLLOUT, nullptr);
  } catch (const event_mult_handler &e) {
    std::string msg = e.what();
    CHECK(msg == "EPOLLIN");
  }
  try {
    ev->set_handler(EPOLLPRI, nullptr);
    CHECK(true);
    ev->set_handler(EPOLLIN | EPOLLOUT | EPOLLPRI, nullptr);
  } catch (const event_mult_handler &e) {
    std::string msg = e.what();
    CHECK(msg == "EPOLLIN|EPOLLPRI");
  }
}
