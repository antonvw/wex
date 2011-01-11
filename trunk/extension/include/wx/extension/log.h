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
class WXDLLIMPEXP_BASE wxExLog
{
public:
  /// Constructor.
  wxExLog(const wxFileName& filename);

  /// Logs the text, using a timestamp.
  const wxExLog& operator<< (const wxString& text) {
    Log(text);
    return *this;};

  /// Returns the filename of the logfile.
  const wxFileName& GetFileName() const {return m_FileName;};
  
  /// Logs text (if logging is on) with a timestamp at the end of the file.
  bool Log(const wxString& text, bool add_timestamp = true) const;
private:
  bool m_Logging;
  const wxFileName m_FileName;
};
#endif
