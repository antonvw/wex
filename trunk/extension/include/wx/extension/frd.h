/******************************************************************************\
* File:          frd.h
* Purpose:       Declaration of wxExFindReplaceData class
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EXFRD_H
#define _EXFRD_H

#include <set>
#include <wx/checkbox.h>
#include <wx/combobox.h>
#include <wx/config.h> 
#include <wx/regex.h>
#include <wx/fdrepdlg.h> // for wxFindReplaceData

/// Adds an existing config to wxFindReplaceData, and some members.
class wxExFindReplaceData : public wxFindReplaceData
{
public:
  /// Constructor, gets members from config.
  wxExFindReplaceData();

  /// Destructor, saves members to config.
 ~wxExFindReplaceData();

  /// Fills a combobox with the find string (cannot be const as FindString is no const).
  void FromFindString(wxComboBox* cb);

  /// Fills a combobox with the replace string (see above).
  void FromReplaceString(wxComboBox* cb);

  /// Gets the field separator.
  const wxChar GetFieldSeparator() const {return m_FieldSeparator;};

  /// Gets find/replace info text.
  const wxString GetText(bool replace = false) const {
    wxString log = _("Searching for") + ": " + m_TextFindWhat;

    if (replace)
    {
      log += " " + _("Replacing with") + ": " + m_TextReplaceWith;
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

  /// Gets text.
  const wxString GetTextFindWhat() const {return m_TextFindWhat;};

  /// Gets text.
  const wxString GetTextMatchCase() const {return m_TextMatchCase;};

  /// Gets text.
  const wxString GetTextMatchWholeWord() const {return m_TextMatchWholeWord;};

  /// Gets text.
  const wxString GetTextRegEx() const {return m_TextRegEx;};

  /// Gets text.
  const wxString GetTextReplaceWith() const {return m_TextReplaceWith;};

  /// Gets text.
  const wxString GetTextSearchDown() const {return m_TextSearchDown;};

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

  /// Sets flags for all three from the checkboxes.
  void SetFromCheckBoxes(
    const wxCheckBox* matchword, 
    const wxCheckBox* matchcase, 
    const wxCheckBox* regularexpression);

  /// Sets regular expression.
  void SetIsRegularExpression(bool value) {
    m_IsRegularExpression = value;};

  /// Sets flags for match case.
  void SetMatchCase(bool value);

  /// Sets flags for match word.
  void SetMatchWord(bool value);

  /// Sets the replace string.
  void SetReplaceString(const wxString& value);
private:
  void Update(wxComboBox* cb, const wxString& value) const;

  wxRegEx m_FindRegularExpression;
  wxString m_FindStringNoCase; // same as the FindString, but case insensitive
  bool m_IsRegularExpression;
  std::set<wxString> m_Info;

  wxConfigBase* m_Config;

  const wxChar m_FieldSeparator;

  const wxString m_TextFindWhat;
  const wxString m_TextMatchCase;
  const wxString m_TextMatchWholeWord;
  const wxString m_TextRegEx;
  const wxString m_TextReplaceWith;
  const wxString m_TextSearchDown;
};
#endif
