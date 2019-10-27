#define DOCTEST_CONFIG_IMPLEMENT_WITH_MAIN
#include "base/log.hpp"
#include "event/channel.hpp"
#include <atomic>
#include <doctest/doctest.h>
#include <future>
#include <sys/socket.h>
#include <sys/types.h>
#include <utility>
using namespace malice::event;
using namespace spdlog;
std::pair<int, int> create_sock_pair() {
  int fds[2] = {0};
  int ret = socketpair(AF_UNIX, SOCK_STREAM, 0, fds);
  CHECK(ret != -1);
  int local_fd = fds[0];
  int peer_fd = fds[1];
  CHECK(local_fd != 0);
  CHECK(peer_fd != 0);
  return {local_fd, peer_fd};
}
TEST_CASE("test shutdown and close") {
  auto [local, peer] = create_sock_pair();
  CHECK(fd_is_valid(local));
  CHECK(fd_is_valid(peer));
  CHECK(shutdown(local, SHUT_RDWR) == 0);
  CHECK(fd_is_valid(local));
  CHECK(close(local) == 0);
  CHECK(!fd_is_valid(local));
  CHECK(close(local) == -1);
  CHECK(shutdown(peer, SHUT_RDWR) == 0);
  // fd will leak if shutdown but not close
  CHECK(close(peer) == 0);
}
TEST_CASE("test fd_is_valid") {
  CHECK(!fd_is_valid(-1));
  auto [local, peer] = create_sock_pair();
  CHECK(fd_is_valid(local));
  CHECK(fd_is_valid(peer));
  close(local);
  close(peer);
  CHECK(!fd_is_valid(local));
  CHECK(!fd_is_valid(peer));
}
TEST_CASE("create  a channel") {
  event_loop loop(-1);
  channel c(2, &loop);
  c.enable_read(true);
  c.enable_read(false);
}
TEST_CASE("test fd_stat") {
  auto [local_fd, peer_fd] = create_sock_pair();
  auto [local_fd2, peer_fd2] = create_sock_pair();
  event_loop loop(-1);

  // shutdown read -> shutdown write -> close
  auto local = std::make_shared<channel>(local_fd, &loop);
  CHECK(local->get_fd_stat() == channel::fd_stat_t::open);
  local->shutdown_read();
  local->shutdown_read();
  CHECK(local->get_fd_stat() == channel::fd_stat_t::shutdown_read);
  local->shutdown_write();
  local->shutdown_write();
  CHECK(local->get_fd_stat() == channel::fd_stat_t::shutdown_rw);
  local->force_close();
  local->shutdown_rw();
  CHECK(local->get_fd_stat() == channel::fd_stat_t::closed);

  // shutdown rw -> close
  auto peer = std::make_shared<channel>(peer_fd, &loop);
  peer->shutdown_rw();
  CHECK(peer->get_fd_stat() == channel::fd_stat_t::shutdown_rw);
  peer->force_close();
  CHECK(peer->get_fd_stat() == channel::fd_stat_t::closed);

  // shutdown write -> shutdown read -> close
  auto peer2 = std::make_shared<channel>(peer_fd2, &loop);
  peer2->shutdown_write();
  CHECK(peer2->get_fd_stat() == channel::fd_stat_t::shutdown_write);
  peer2->shutdown_read();
  CHECK(peer2->get_fd_stat() == channel::fd_stat_t::shutdown_rw);
  peer2->force_close();
  CHECK(peer2->get_fd_stat() == channel::fd_stat_t::closed);

  auto local2 = std::make_shared<channel>(local_fd2, &loop);
  local2->force_close();
}

TEST_CASE("test default on_error by write closed fd") {
  spdlog::set_level(spdlog::level::info);
  event_loop loop(-1);
  auto [local_fd, peer_fd] = create_sock_pair();

  channel local(local_fd, &loop);
  CHECK(fd_is_valid(local_fd)); //此时fd是open的
  std::shared_ptr<channel> peer = std::make_shared<channel>(peer_fd, &loop);
  peer.reset(); //将peer reset掉后peer内的fd就被close了

  local.write("hello"); //因为peer已经close了，所以写会失败
  // default on_error
  loop.wait();
  CHECK(!fd_is_valid(local_fd)); //因为local的default_on_error会强制close掉fd
}
// peer:
//  write
//  close
// local:
//  read
//  close
TEST_CASE("test write read close") {
  std::atomic<bool> stop_wait{false};

  auto [local_fd, peer_fd] = create_sock_pair();
  const char *send_msg = "hello";
  event_loop loop(-1);

  // set local channel
  channel local(local_fd, &loop);
  local.set_read_handler([send_msg](::malice::base::buffer &read_buf) {
    std::string msg = read_buf.take_all_as_string();
    CHECK(msg == send_msg);
  });
  local.set_close_handler([&stop_wait](channel *chan, channel::close_type ct) {
    CHECK(ct == channel::close_type::eof);
    CHECK(chan->buffer_empty());
    chan->force_close();
    stop_wait = true;
  });
  local.enable_read(true);

  // set peer channel
  channel peer_chan(peer_fd, &loop);
  peer_chan.set_write_finish_handler(
      [](channel *chan) { chan->shutdown_write(); });

  peer_chan.write(send_msg);
  while (!stop_wait) {
    loop.wait();
  }
  CHECK(true);
}

TEST_CASE("test default_on_read") {
  event_loop loop(-1);
  auto [local_fd, peer_fd] = create_sock_pair();
  // setup local channel, default_on_read default_on_close
  channel local(local_fd, &loop);
  local.enable_read(true);

  // setup peer channel
  channel peer(peer_fd, &loop);
  peer.set_write_finish_handler([](channel *chan) { chan->shutdown_write(); });
  peer.write("hello");
  // wait write
  CHECK(!peer.buffer_empty());
  loop.wait();
  CHECK(peer.buffer_empty());

  // wait read,read and drop data
  CHECK(local.buffer_empty());
  loop.wait();
  CHECK(local.buffer_empty());
  // wait close
  loop.wait();
}

TEST_CASE("call default_on_close and read_buf full") {
  std::string msg(1024, 'c');
  int read_count = 0;

  event_loop loop(-1);
  auto [local_fd, peer_fd] = create_sock_pair();
  // setup local channel
  channel local(local_fd, &loop);
  local.enable_read(true);
  local.set_read_handler([&msg, &read_count](::malice::base::buffer &buf) {
    read_count++;
    if (read_count == 1) {
      CHECK(buf.readable_size() == msg.size());
    } else if (read_count == 2) {
      CHECK(buf.readable_size() == msg.size() * 2);
      buf.take_all();
    }
  });
  // setup peer channel
  channel peer(peer_fd, &loop);
  peer.set_write_finish_handler([](channel *chan) { chan->shutdown_write(); });

  // peer 将数据写入write_buf
  peer.write(msg);
  peer.write(msg);
  CHECK(!peer.write_buffer_empty());

  //等待peer可写
  loop.wait();

  // peer写完成
  CHECK(peer.write_buffer_empty());

  // local 还没开始读
  CHECK(local.read_buffer_empty());
  CHECK(read_count == 0);

  //因为默认的read_buf只有1024大小,所以要两次读事件才能把数据全部读出来
  // 1.读1024字节
  loop.wait();
  CHECK(read_count == 1);
  CHECK(!local.read_buffer_empty());
  // 2.这次等到读事件发现read_buf已经满了，所以会扩容，然后将数据继续读进来
  loop.wait();
  CHECK(read_count == 2);

  //读完2048字节了，会会收到eof,因为local没有挂载on_close,所有会调用default_on_close,关闭掉local_fd
  loop.wait();
}
