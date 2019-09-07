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
    SUBCASE("two handler") {
      int counter = 0;
      ev->set_handler(EPOLLIN, [&counter](event *e) {
        counter++;
        int flag = e->get_flag();
        CHECK((flag & EPOLLIN) == true);
      });
      ev->set_handler(EPOLLOUT, [&counter](event *) {
        counter++;
        CHECK(false);
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
