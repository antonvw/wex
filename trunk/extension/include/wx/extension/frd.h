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

  /// Gets the find replace data.
  static wxExFindReplaceData* Get(bool createOnDemand = true);

  /// Gets the field separator.
  const wxUniChar& GetFieldSeparator() const {return m_FieldSeparator;};

  /// Gets find/replace info text.
  const wxString GetText(bool replace = false) const;

  /// Gets the text for the check boxes.
  const std::set<wxString> & GetInfo() const {return m_Info;};

  /// Gets the regular expression.
  const wxRegEx& GetRegularExpression() const {
    return m_FindRegularExpression;};

  /// Gets text.
  const wxString& GetTextFindWhat() const {return m_TextFindWhat;};

  /// Gets text.
  const wxString& GetTextMatchCase() const {return m_TextMatchCase;};

  /// Gets text.
  const wxString& GetTextMatchWholeWord() const {return m_TextMatchWholeWord;};

  /// Gets text.
  const wxString& GetTextRegEx() const {return m_TextRegEx;};

  /// Gets text.
  const wxString& GetTextReplaceWith() const {return m_TextReplaceWith;};

  /// Gets text.
  const wxString& GetTextSearchDown() const {return m_TextSearchDown;};

  /// Returns true if find text is a regular expression.
  bool IsRegularExpression() const {return m_IsRegularExpression;};

  /// Returns true if the flags have match case set.
  bool MatchCase() const {return (GetFlags() & wxFR_MATCHCASE) > 0;};

  /// Returns true if the flags have whole word set.
  bool MatchWord() const {return (GetFlags() & wxFR_WHOLEWORD) > 0;};

  /// Sets the object as the current one, returns the pointer 
  /// to the previous current object 
  /// (both the parameter and returned value may be NULL). 
  static wxExFindReplaceData* Set(wxExFindReplaceData* frd);

  /// Sets the find string.
  /// If IsRegularExpression also sets the regular expression.
  /// This string is used for tool find in files and replace in files.
  void SetFindString(const wxString& value);

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
  wxRegEx m_FindRegularExpression;
  bool m_IsRegularExpression;
  std::set<wxString> m_Info;

  const wxUniChar m_FieldSeparator;

  const wxString m_TextFindWhat;
  const wxString m_TextMatchCase;
  const wxString m_TextMatchWholeWord;
  const wxString m_TextRegEx;
  const wxString m_TextReplaceWith;
  const wxString m_TextSearchDown;

  static wxExFindReplaceData* m_Self;
};
#endif
