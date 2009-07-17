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
#include <set>
#include <wx/regex.h>
#include <wx/fdrepdlg.h> // for wxFindReplaceData

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
  long Get(const wxString& key, long default_value);

  /// Gets the key as a string. If the key is not present,
  /// it is added to the map.
  /// This also works for comboboxes,
  /// as long as the values are separated by default row delimiter,
  /// as then it returns value before this delimiter.
  const wxString Get(
    const wxString& key,
    const wxString& default_value = wxEmptyString,
    const wxChar field_separator = ',');

  /// Gets the key as a bool. If the key is not present,
  /// it is added to the map.
  bool GetBool(const wxString& key, bool default_value = true); 

  /// Gets the find replace data.
  wxExFindReplaceData* GetFindReplaceData() const {
    return m_FindReplaceData;};

  /// Gets all keys as one string.
  const wxString GetKeys() const;

  /// Sets key to value.
  void Set(const wxString& key, const wxVariant& value) {
    m_Values[key] = value;};

  /// Sets flags in find replace data.
  void SetFindReplaceData(
    bool matchword, 
    bool matchcase, 
    bool regularexpression);

  /// Toggles boolean key value.
  void Toggle(const wxString& key);
private:
  wxExFindReplaceData* m_FindReplaceData;
  std::map<wxString, wxVariant> m_Values;
};

/// Adds an existing config to wxFindReplaceData, and some members.
class wxExFindReplaceData : public wxFindReplaceData
{
public:
  /// Constructor, gets members from config.
  wxExFindReplaceData(wxExConfig* config);

  /// Destructor, saves members to config.
 ~wxExFindReplaceData();

  /// Gets find/replace text.
  const wxString GetText(bool replace = false) const {
    wxString log = _("Searching for") + ": " + m_Config->Get(_("Find what"));
    if (replace)
    {
      log += " " + _("Replacing with") + ": " + m_Config->Get(_("Replace with"));
    }
    return log;};

  /// Gets the text for the check boxes.
  const std::set<wxString> & GetInfo() const {return m_Info;};

  /// Gets the regular expression.
  const wxRegEx& GetRegularExpression() const {
    return m_FindRegularExpression;};

  /// Gets the case insensitive find string.
  const wxString& GetFindStringNoCase() const {
    return m_FindStringNoCase;};

  /// Returns true if find text is a regular expression.
  bool IsRegularExpression() const {return m_IsRegularExpression;};

  /// Returns true if the flags have match case set.
  bool MatchCase() const {return (GetFlags() & wxFR_MATCHCASE) > 0;};

  /// Returns true if the flags have whole word set.
  bool MatchWord() const {return (GetFlags() & wxFR_WHOLEWORD) > 0;};

  /// Sets the find string.
  /// If IsRegularExpression also sets the regular expression.
  /// This string is used for tool find in files and replace in files.
  void SetFindString(const wxString& value);

  /// Sets member.
  void SetIsRegularExpression(bool value) {
    m_IsRegularExpression = value;};

  /// Sets flags for match case.
  void SetMatchCase(bool value);

  /// Sets flags for match word.
  void SetMatchWord(bool value);
private:
  wxExConfig* m_Config;
  wxRegEx m_FindRegularExpression;
  wxString m_FindStringNoCase; // same as the FindString, but case insensitive
  bool m_IsRegularExpression;
  std::set<wxString> m_Info;
};
#endif
