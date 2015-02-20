////////////////////////////////////////////////////////////////////////////////
// Name:      interruptabl.h
// Purpose:   Declaration of class wxExDir and wxExDirOpenFile
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

/// Adds FindFiles to a wxDir.
/// By overriding OnDir and OnFile you can take care
/// of what to do with the result.
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
