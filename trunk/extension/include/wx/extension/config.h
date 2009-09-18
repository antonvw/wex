/******************************************************************************\
* File:          config.h
* Purpose:       Declaration of wxWidgets config extension classes
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EXCONFIG_H
#define _EXCONFIG_H

#include <map>
#include <wx/variant.h> 

class wxExFindReplaceData;

/// Offers a general configuration.
/// Keys are read the first time accessed from the config.
/// Next time they are retrieved from the maps, so access is fast.
#ifdef EX_PORTABLE
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

  /// Gets the key as a long. If the key is not present,
  /// it is added to the map.
  long Get(const wxString& key, long default_value) {
    std::map<wxString, wxVariant>::const_iterator it = m_Values.find(key);

    if (it != m_Values.end())
    {
      return it->second.GetLong();
    }
    else
    {
      const long config_value = Read(key, default_value);
      m_Values.insert(std::make_pair(key, config_value));
      return config_value;
    }
  };

  /// Gets the key as a string. If the key is not present,
  /// it is added to the map.
  /// This also works for comboboxes,
  /// as long as the values are separated by default row delimiter,
  /// as then it returns value before this delimiter.
  const wxString Get(
    const wxString& key,
    const wxString& default_value = wxEmptyString,
    const wxChar field_separator = ',')	{
    std::map<wxString, wxVariant>::const_iterator it = m_Values.find(key);

    if (it != m_Values.end())
    {
      const wxString value = it->second;
      return value.BeforeFirst(field_separator);
     }
    else
    {
      const wxString value = Read(key, default_value);
      m_Values.insert(std::make_pair(key, value));
      return value.BeforeFirst(field_separator);
    }
  };

  /// Gets the key as a bool. If the key is not present,
  /// it is added to the map.
  bool GetBool(const wxString& key, bool default_value = true) {
    std::map<wxString, wxVariant>::const_iterator it = m_Values.find(key);

    if (it != m_Values.end())
    {
      return it->second.GetBool();
    }
    else
    {
      const bool config_value = ReadBool(key, default_value);
      m_Values.insert(std::make_pair(key, config_value));
      return config_value;
    }
  };

  /// Gets the find replace data.
  wxExFindReplaceData* GetFindReplaceData() const {
    return m_FindReplaceData;};

  /// Gets all keys as one string.
  const wxString GetKeys() const;

  /// Sets key to value.
  void Set(const wxString& key, const wxVariant& value) {
    m_Values[key] = value;};

  /// Toggles boolean key value.
  void Toggle(const wxString& key);
private:
  wxExFindReplaceData* m_FindReplaceData;
  std::map<wxString, wxVariant> m_Values;
};
#endif
