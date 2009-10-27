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

/// Offers logging.
class wxExLog
{
public:
  wxExLog();

  /// Returns the filename of the logfile.
  const wxFileName GetFileName() const; // wxExLogfileName

  /// Logs text (only if SetLogging(true) is called, default it is off).
  /// Returns true if logging was on and write was successfull.
  bool Log(const wxString& text);

  /// Logs text with a timestamp at the end of the file.
  /// Returns true if text was written succesfully.
  bool Log(const wxString& text, const wxFileName& filename);

  /// Sets logging as specified.
  /// If the logging is true and the logfile does not exist, it is created.
  bool SetLogging(bool logging = true);
private:
  bool m_Logging;
};
#endif
