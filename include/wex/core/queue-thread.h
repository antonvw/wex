////////////////////////////////////////////////////////////////////////////////
// Name:      queue-thread.h
// Purpose:   Declaration of class wex::queue_thread
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/core/log.h>

import<atomic>;
import<condition_variable>;
import<mutex>;
import<queue>;
import<thread>;
import<vector>;

namespace wex
{
/// This class offers a templatized queue you can put events in,
/// and a thread pool that retrieves the events and
/// publishes them through the event handler.
/// Type T is the type of event to be processed
template <typename T> class queue_thread
{
public:
  /// This event handler class must be subclassed and passed in to the
  /// queue thread constructor (in order to process events of type T).
  class event_handler
  {
  public:
    /// Default constructor.
    event_handler() {}

    /// Destructor.
    virtual ~event_handler() = default;

    /// This method must be overridden to take care that
    /// the event is processed.
    virtual void process(std::unique_ptr<T>& event) = 0;
  };

  /// Constructor.
  queue_thread(
    /// event handler
    event_handler& event_handler_instance,
    /// timeout im milliseconds used
    unsigned int timeout_ms = 500);

  /// Destructor.
  ~queue_thread() {}

  /// Emplaces event on the queue.
  /// The thread pool takes care of reading and clearing
  /// the queue, and sending it to the event handler
  /// (be sure you have invoked start).
  void emplace(std::unique_ptr<T>& event);

  /// Returns true if queue is empty.
  bool empty();

  /// Returns true if event loop is started.
  bool is_running() const { return m_running.load(); }

  // Starts the threads.
  void start(unsigned int pool_size = 1);

  /// Stops the threads (and processing of events).
  void stop();

private:
  void thread_main();

  event_handler&            m_event_handler;
  std::atomic_bool          m_running{false};
  std::chrono::milliseconds m_timeout;
  std::condition_variable   m_queue_condition;

  std::mutex m_queue_mutex, m_thread_mutex;

  std::queue<std::unique_ptr<T>> m_event_queue;
  std::vector<std::thread>       m_threads;
};
}; // namespace wex

// implementation

template <typename T>
wex::queue_thread<T>::queue_thread(
  event_handler& event_handler_instance,
  unsigned int   timeout_ms)
  : m_event_handler(event_handler_instance)
  , m_timeout(timeout_ms)
{
}

template <typename T>
void wex::queue_thread<T>::emplace(std::unique_ptr<T>& event)
{
  std::unique_lock<std::mutex> lock(m_queue_mutex);
  m_event_queue.emplace(event.release());

  // Wake up one thread to process the event
  m_queue_condition.notify_one();
}

template <typename T> bool wex::queue_thread<T>::empty()
{
  std::unique_lock<std::mutex> lock(m_queue_mutex);
  return m_event_queue.empty();
}

template <typename T> void wex::queue_thread<T>::start(unsigned int pool_size)
{
  std::unique_lock<std::mutex> lock(m_thread_mutex);

  if (!m_running.load())
  {
    m_threads.reserve(pool_size);
    m_running.store(true);

    for (unsigned int i = 0; i < pool_size; ++i)
    {
      m_threads.emplace_back(&queue_thread::thread_main, this);
    }
  }
}

template <typename T> void wex::queue_thread<T>::stop()
{
  std::unique_lock<std::mutex> lock(m_thread_mutex);

  if (m_running.load())
  {
    m_running.store(false);

    // Get the queue lock and wake up all the threads to try to expedite
    // the exiting of the threads.
    {
      std::unique_lock<std::mutex> lock(m_queue_mutex);
      m_queue_condition.notify_all();
    }
    for (auto& thread : m_threads)
    {
      if (thread.joinable())
      {
        try
        {
          thread.join();
        }
        catch (std::exception& e)
        {
          log(e) << "thread.join";
        }
      }
    }

    m_threads.clear();
  }
}

template <typename T> void wex::queue_thread<T>::thread_main()
{
  std::unique_ptr<T> event;

  // Run until another thread sets the stop flag
  while (m_running.load())
  {
    {
      // Get the queue lock so this thread can wait on the condition
      // and then get exclusive access to the queue when it
      // wakes up.
      std::unique_lock<std::mutex> lock(m_queue_mutex);

      // Check to see if there is work to do.
      bool work_to_do = !m_event_queue.empty();

      if (!work_to_do)
      {
        // Queue is empty, wait for a new event, timeout or
        // spurious wake-up
        m_queue_condition.wait_for(lock, std::chrono::milliseconds(m_timeout));

        // Regardless of why the thread was woken up, check again
        // to see if there is work to do.
        work_to_do = m_running.load() && !m_event_queue.empty();
      }

      if (work_to_do)
      {
        // Transfer ownership of new event to this function,
        // delete old event (if there is one)
        event.reset(m_event_queue.front().release());
        m_event_queue.pop();
      }
    }

    if (event.get() != nullptr)
    {
      m_event_handler.process(event);
    }
  }
}
