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
#include <wx/regex.h>
#include <wx/fdrepdlg.h> // for wxFindReplaceData

class exFindReplaceData;

/// Offers a general configuration.
/// Keys are read the first time accessed from the config.
/// Next time they are retrieved from the maps, so access is fast.
#ifdef EX_PORTABLE
#include <wx/fileconf.h>
class exConfig : public wxFileConfig
#else
#include <wx/config.h>
class exConfig : public wxConfig
#endif
{
public:
  /// Default constructor.
  exConfig(
    const wxString& filename = wxEmptyString,
    long style = 0);

  /// Destructor, writes all keys.
 ~exConfig();

  /// Gets the key as a long. If the key is not present,
  /// it is added to the map of long values.
  long Get(
    const wxString& key, long default_value) {
    std::map<wxString, long>::const_iterator it = m_LongValues.find(key);

    if (it != m_LongValues.end())
    {
      return it->second;
    }
    else
    {
      const long config_value = Read(key, default_value);
      m_LongValues.insert(std::make_pair(key, config_value));
      return config_value;
    }
  }

  /// Gets all longs keys as one string.
  const wxString GetLongKeys() const;
  
  /// Gets the key as a string. If the key is not present,
  /// it is added to the map of string values.
  /// This also works for comboboxes,
  /// as long as the values are separated by default row delimiter,
  /// as then it returns value before this delimiter.
  const wxString Get(
    const wxString& key,
    const wxString& default_value = wxEmptyString,
    const wxChar field_separator = ',') {
    std::map<wxString, wxString>::const_iterator it = m_StringValues.find(key);

    if (it != m_StringValues.end())
    {
      const wxString value = it->second;
      return value.BeforeFirst(field_separator);
    }  
    else
    {
      const wxString config_value = Read(key, default_value);
      m_StringValues.insert(std::make_pair(key, config_value));
      return config_value;
    }
  }

  /// Gets all string keys as one string.
  const wxString GetStringKeys() const;
  
  /// Gets the key as a bool. If the key is not present,
  /// it is added to the map of bool values.
  bool GetBool(
    const wxString& key, bool default_value = true) {
    std::map<wxString, bool>::const_iterator it = m_BoolValues.find(key);

    if (it != m_BoolValues.end())
    {
      return it->second;
    }
    else
    {
      const bool config_value = ReadBool(key, default_value);
      m_BoolValues.insert(std::make_pair(key, config_value));
      return config_value;
    }
  }

  /// Gets all bool keys as one string.
  const wxString GetBoolKeys() const;
  
  /// Gets the find replace data.
  exFindReplaceData* GetFindReplaceData() const {
    return m_FindReplaceData;};

  /// Sets key as a long.
  void Set(const wxString& key, long value) {
    m_LongValues[key] = value;};

  /// Sets key as a string.
  void Set(const wxString& key, const wxString& value) {
    m_StringValues[key] = value;};

  /// Sets key as a bool.
  void SetBool(const wxString& key, bool value) {
    m_BoolValues[key] = value;};
  
  /// Sets flags in find replace data.
  void SetFindReplaceData(
    bool matchword, bool matchcase, bool regularexpression);

  /// Toggles boolean key value.
  void Toggle(const wxString& key) {
    m_BoolValues[key] = !m_BoolValues[key];}
private:
  exFindReplaceData* m_FindReplaceData;
  std::map<wxString, bool> m_BoolValues;
  std::map<wxString, long> m_LongValues;
  std::map<wxString, wxString> m_StringValues;
};

/// Adds an existing config to wxFindReplaceData, and some members.
class exFindReplaceData : public wxFindReplaceData
{
public:
  /// Constructor, gets members from config.
  exFindReplaceData(exConfig* config);

  /// Destructor, saves members to config.
 ~exFindReplaceData();

  /// Gets find/replace text.
  const wxString GetText(bool replace = false) const {
    wxString log = _("Searching for") + ": " + m_Config->Get(_("Find what"));
    if (replace)
    {
      log += " " + _("Replacing with") + ": " + m_Config->Get(_("Replace with"));
    }
    return log;};

  /// Gets the regular expression.
  const wxRegEx& GetFindRegularExpression() const {
    return m_FindRegularExpression;};

  /// Gets the case insensitive find string.
  const wxString& GetFindStringNoCase() const {
    return m_FindStringNoCase;};

  /// Returns true if find text is a regular expression.
  bool IsRegExp() const;

  /// Returns true if the flags have match case set.
  bool MatchCase() const {return (GetFlags() & wxFR_MATCHCASE) > 0;};

  /// Returns true if the flags have whole word set.
  bool MatchWord() const {return (GetFlags() & wxFR_WHOLEWORD) > 0;};

  /// Sets the find string.
  /// If IsRegExp also sets the  and regular expression, if it returns false,
  /// no valid regular expression has been entered.
  /// This string is used for tool find in files and replace in files.
  bool SetFindString(const wxString& value);

  /// Sets member.
  void SetIsRegularExpression(bool value);

  /// Sets flags for match case.
  void SetMatchCase(bool value);

  /// Sets flags for match word.
  void SetMatchWord(bool value);
private:
  exConfig* m_Config;
  wxRegEx m_FindRegularExpression;
  wxString m_FindStringNoCase; // same as the FindString, but case insensitive
  bool m_IsRegularExpression;
};
#endif
