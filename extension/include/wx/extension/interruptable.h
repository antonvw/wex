////////////////////////////////////////////////////////////////////////////////
// Name:      interruptable.h
// Purpose:   Declaration of class wxExInterruptable
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

/// Offers methods to start, stop things.
class WXDLLIMPEXP_BASE wxExInterruptable
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
  static void Start();
  
  /// Stops interruptable process.
  static void Stop();
private:
  static bool m_Cancelled;
  static bool m_Running;
};
