/******************************************************************************\
* File:          log.h
* Purpose:       Include file for wxExLog class
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EXLOG_H
#define _EXLOG_H

#include <wx/filename.h>

/// Offers logging.
class wxExLog
{
public:
  /// Constructor.
  wxExLog(const wxFileName& filename);

  /// Returns the log object.
  static wxExLog* Get(bool createOnDemand = true);

  /// Returns the filename of the logfile.
  const wxFileName GetFileName() const {return m_FileName;};

  /// Logs text with a timestamp at the end of the file.
  /// Logs text (only if SetLogging(true) is called, default it is off).
  /// Returns true if logging was on and write was successfull.
  bool Log(const wxString& text);

  /// Sets the object as the current one, returns the pointer 
  /// to the previous current object (both the parameter and returned value may be NULL). 
  static wxExLog* Set(wxExLog* log);

  /// Sets logging as specified.
  /// If the logging is true and the logfile does not exist, it is created.
  bool SetLogging(bool logging = true);
private:
  bool m_Logging;
  const wxFileName m_FileName;

  static wxExLog* m_Self;
};
#endif
