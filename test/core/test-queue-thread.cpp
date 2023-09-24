////////////////////////////////////////////////////////////////////////////////
// Name:      test-queue-thread.cpp
// Purpose:   Implementation for wex unit testing
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/queue-thread.h>
#include <wex/test/test.h>

#include <chrono>

namespace wex
{
class event_handler : public queue_thread<std::string>::event_handler
{
public:
  const std::vector<std::string>& events() const { return m_events; }

private:
  void process(std::unique_ptr<std::string>& input) override
  {
    std::unique_ptr<std::string> event(input.release());
    m_events.emplace_back(*event.get());
  };

  std::vector<std::string> m_events;
};
}; // namespace wex

TEST_CASE("wex::queue_thread")
{
  wex::event_handler             event_handler;
  wex::queue_thread<std::string> queue_thread(event_handler);
  REQUIRE(!queue_thread.is_running());

  queue_thread.start();
  REQUIRE(queue_thread.is_running());

  const int max_events = 100;
  for (int i = 0; i < max_events; i++)
  {
    std::unique_ptr<std::string> test(
      new std::string(std::to_string(i) + " event to be processed"));
    queue_thread.emplace(test);
  }

  std::this_thread::sleep_for(std::chrono::milliseconds(10));
  REQUIRE(event_handler.events().size() == max_events);
  REQUIRE(queue_thread.empty());

  queue_thread.stop();
  REQUIRE(!queue_thread.is_running());
}
