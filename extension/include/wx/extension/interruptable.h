////////////////////////////////////////////////////////////////////////////////
// Name:      interruptable.h
// Purpose:   Declaration of class wex::interruptable
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

namespace wex
{
  /// Offers methods to start, stop things.
  class interruptable
  {
  public:
    /// Cancel interruptable process.
    /// Returns false if process was not running.
    static bool Cancel();

    /// Check whether process was cancelled.
    static bool Cancelled() {return m_Cancelled;};

    /// Is process running.
    static bool Running() {return m_Running;};
    
    /// Starts interruptable process.
    /// Returns false if process is already running.
    static bool Start();
    
    /// Stops interruptable process.
    static void Stop();
  private:
    static bool m_Cancelled;
    static bool m_Running;
  };
};
