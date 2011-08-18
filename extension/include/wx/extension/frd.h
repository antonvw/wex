////////////////////////////////////////////////////////////////////////////////
// Name:      frd.h
// Purpose:   Declaration of wxExFindReplaceData class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXFRD_H
#define _EXFRD_H

#include <list>
#include <set>
#include <wx/regex.h>
#include <wx/fdrepdlg.h> // for wxFindReplaceData

class wxCheckListBox;

/// Adds an existing config to wxFindReplaceData, and some members.
class WXDLLIMPEXP_BASE wxExFindReplaceData : public wxFindReplaceData
{
public:
  // Destructor (not for Doxy).
 ~wxExFindReplaceData();
 
  /// Gets the find replace data.
  static wxExFindReplaceData* Get(bool createOnDemand = true);

  /// Gets field member into a check list box.
  bool Get(const wxString& field, wxCheckListBox* clb, int item) const;

  /// Gets find/replace info text.
  const wxString GetFindReplaceInfoText(bool replace = false) const;

  /// Gets the find strings.
  const std::list < wxString > & GetFindStrings() const {
    return m_FindStrings;};

  /// Gets the text for the check boxes.
  const std::set<wxString> & GetInfo() const {return m_Info;};

  /// Gets the regular expression.
  const wxRegEx& GetRegularExpression() const {
    return m_FindRegularExpression;};

  /// Gets the replace strings.
  const std::list < wxString > & GetReplaceStrings() const {
    return m_ReplaceStrings;};

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

  /// Returns true if the flags have match case set.
  bool MatchCase() const {return (GetFlags() & wxFR_MATCHCASE) > 0;};

  /// Returns true if the flags have whole word set.
  bool MatchWord() const {return (GetFlags() & wxFR_WHOLEWORD) > 0;};

  /// Returns true if the flags have search down set.
  bool SearchDown() const {return (GetFlags() & wxFR_DOWN) > 0;};

  /// Sets the object as the current one, returns the pointer 
  /// to the previous current object 
  /// (both the parameter and returned value may be NULL). 
  static wxExFindReplaceData* Set(wxExFindReplaceData* frd);

  /// Sets field member if the specified text matches 
  /// one of the text fields.
  bool Set(const wxString& field, bool value);

  /// Sets the find string.
  /// If UseRegularExpression also sets the regular expression.
  /// This string is used for tool find in files and replace in files.
  void SetFindString(const wxString& value);

  /// Sets the find strings.
  void SetFindStrings(const std::list < wxString > & value);

  /// Sets flags for match case.
  void SetMatchCase(bool value);

  /// Sets flags for match word.
  void SetMatchWord(bool value);

  /// Sets the replace string.
  void SetReplaceString(const wxString& value);

  /// Sets the replace strings.
  void SetReplaceStrings(const std::list < wxString > & value);

  /// Sets using regular expression for find text.
  void SetUseRegularExpression(bool value) {
    m_UseRegularExpression = value;};

  /// Convert search flags into STC search flags.
  int STCFlags() const;

  /// Returns true if find text is used as a regular expression.
  bool UseRegularExpression() const {return m_UseRegularExpression;};
private:
  wxExFindReplaceData();
  void SetFindRegularExpression();

  wxRegEx m_FindRegularExpression;
  bool m_UseRegularExpression;

  const wxString m_TextFindWhat;
  const wxString m_TextMatchCase;
  const wxString m_TextMatchWholeWord;
  const wxString m_TextRegEx;
  const wxString m_TextReplaceWith;
  const wxString m_TextSearchDown;

  std::list < wxString > m_FindStrings;
  std::list < wxString > m_ReplaceStrings;
  std::set < wxString > m_Info;

  static wxExFindReplaceData* m_Self;
};
#endif
