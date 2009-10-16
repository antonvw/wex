/******************************************************************************\
* File:          config.h
* Purpose:       Declaration of wxExtension config class
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EXCONFIG_H
#define _EXCONFIG_H

class wxExFindReplaceData;

/// Offers a general configuration.
/// Keys are read the first time accessed from the config.
/// Next time they are retrieved from the maps, so access is fast.
#ifdef wxExUSE_PORTABLE
#include <wx/fileconf.h>
class wxExConfig : public wxFileConfig
#else
#include <wx/config.h>
class wxExConfig : public wxConfig
#endif
{
public:
  /// Default constructor.
  wxExConfig(
    const wxString& filename = wxEmptyString,
    long style = 0);

  /// Destructor, writes all keys.
 ~wxExConfig();

  /// Reads the key as a string.
  /// This also works for comboboxes,
  /// as long as the values are separated by default row delimiter,
  /// as then it returns value before this delimiter.
  const wxString Read(
    const wxString& key,
    const wxString& default_value = wxEmptyString,
    const wxChar field_separator = ',')	{
      const wxString value = wxConfig::Read(key, default_value);
      return value.BeforeFirst(field_separator);
    };

  /// Gets the find replace data.
  wxExFindReplaceData* GetFindReplaceData() const {
    return m_FindReplaceData;};
private:
  wxExFindReplaceData* m_FindReplaceData;
};
#endif
