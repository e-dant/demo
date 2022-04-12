#include <algorithm>
#include <boost/asio/dispatch.hpp>
#include <boost/asio/strand.hpp>
#include <boost/beast/core.hpp>
#include <boost/beast/http.hpp>
#include <boost/beast/version.hpp>
#include <boost/config.hpp>
#include <cstdlib>
#include <functional>
#include <iostream>
#include <memory>
#include <ostream>
#include <server/config.hpp>
#include <server/fs.hpp>
#include <server/info.hpp>
#include <server/listener.hpp>
#include <server/session.hpp>
#include <string>
#include <thread>
#include <utility>
#include <vector>

/** @project demo server
 *
 * Firstly, this server would not be possible without boost.asio and
 * boost.beast.
 *
 * Moreover, Vinnie Falco's excellent examples and talks provided the listener,
 * response and session sections (where his apache2 licenses are maintained).
 *
 * Another excellent shout-out to @lefticus (github; host of cppcast) for
 * providing the foundation for the C++ Template. While I have removed most of
 * it, for ease of distribution and not needing automatic CI for this demo, It
 * was nevertheless fundamental in its exposition of model boilerplate for a C++
 * build system.
 *
 * This work is also derivative of my own: the containers that host my website
 * scale with load balancers and containers; not concurrent, singular servers.
 * It was therefore unnecessary to implement this kind concurrency for my
 * website -- but the basis for the server, its argument mapping, containment
 * and deployment can be found at @e-dant/shoestring.
 *
 */

auto main(int argc, char* argv[]) -> int try {
  constexpr const auto thread_count{7};
  const auto po{server::program_options(argc, argv)};
  auto ioc{boost::asio::io_context{thread_count}};
  auto thread_pool{std::map<int, std::thread>{}};
  auto listener{std::make_shared<server::listener>(
      ioc, boost::asio::ip::tcp::endpoint{po.address, po.port},
      std::make_shared<std::string>("."))};

  listener->run();

  server::fs::rprint(".");
  info::soft("address:port(", po.address, ":", po.port, ")");

  // we use thread_count-1 because this thread will also be running.
  // (it's a segfault if the program exits while the io context is handling
  // threads)
  for (auto i = 0; i < (thread_count - 1); i++) {
    thread_pool.insert(std::pair<int, std::thread>{i, [&ioc] { ioc.run(); }});
  }

  ioc.run();

  return 0;
} catch (std::exception const& e) {
  info::fatal("fatal standard exception", std::move(e.what()));
}
