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

/// Returns the filename of the logfile.
const wxFileName wxExLogfileName();

/// Logs text with a timestamp at the end of the file.
/// Returns true if text was written succesfully.
bool wxExLog(const wxString& text, const wxFileName& filename = wxExLogfileName());

/// Offers logging.
class wxExLog
{
public:
  /// Logs text (only if SetLogging(true) is called, default it is off).
  /// Returns true if logging was on and write was successfull.
  static bool Log(const wxString& text) {
    if (m_Logging) return wxExLog(text);
    else           return false;};

  /// Sets logging as specified.
  /// If the logging is true and the logfile does not exist, it is created.
  static bool SetLogging(bool logging = true);
private:
  static bool m_Logging;
};
#endif
