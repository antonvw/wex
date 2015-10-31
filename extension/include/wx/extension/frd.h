////////////////////////////////////////////////////////////////////////////////
// Name:      frd.h
// Purpose:   Declaration of wxExFindReplaceData class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <list>
#include <regex>
#include <string>
#include <wx/fdrepdlg.h> // for wxFindReplaceData
#include <wx/textctrl.h>

/// Adds an existing config to wxFindReplaceData, and some members.
class WXDLLIMPEXP_BASE wxExFindReplaceData : public wxFindReplaceData
{
public:
  // Destructor (not for Doxy).
 ~wxExFindReplaceData();
 
  /// Gets the find replace data.
  static wxExFindReplaceData* Get(bool createOnDemand = true);

  /// Gets the find strings.
  const auto & GetFindStrings() const {return m_FindStrings;};

  /// Gets the replace strings.
  const auto & GetReplaceStrings() const {return m_ReplaceStrings;};

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
  
  /// Iterates text ctrl for specified key.
  /// Returns true if key was handled.
  bool Iterate(wxTextCtrl* ctrl, int key);

  /// Returns true if the flags have match case set.
  bool MatchCase() const {return (GetFlags() & wxFR_MATCHCASE) > 0;};

  /// Returns true if the flags have whole word set.
  bool MatchWord() const {return (GetFlags() & wxFR_WHOLEWORD) > 0;};
  
  /// Returns true if GetFindString as regular expression matches text.
  bool RegExMatches(const std::string& text) const;
  
  /// Replaces all occurrences of GetFindString as regular expression
  /// in text by GetReplaceString.
  /// Returns number of replacements done in text.
  int RegExReplaceAll(std::string& text) const;

  /// Returns true if the flags have search down set.
  bool SearchDown() const {return (GetFlags() & wxFR_DOWN) > 0;};

  /// Sets the object as the current one, returns the pointer 
  /// to the previous current object 
  /// (both the parameter and returned value may be NULL). 
  static wxExFindReplaceData* Set(wxExFindReplaceData* frd);

  /// Sets the find string.
  /// If UseRegEx also sets the regular expression.
  /// This string is used for tool find in files and replace in files.
  /// Als moves the find string to the beginning of the find
  /// strings list.
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
  /// Als moves the replace string to the beginning of the replace
  /// strings list.
  void SetReplaceStrings(const std::list < wxString > & value);

  /// Sets using regular expression for find text.
  /// If GetFindString does not contain a valid regular expression
  /// the use member is not set.
  void SetUseRegEx(bool value);

  /// Returns true if find text is used as a regular expression.
  bool UseRegEx() const {return m_UseRegEx;};
private:
  wxExFindReplaceData();

  bool m_UseRegEx;

  const wxString m_TextFindWhat;
  const wxString m_TextMatchCase;
  const wxString m_TextMatchWholeWord;
  const wxString m_TextRegEx;
  const wxString m_TextReplaceWith;
  const wxString m_TextSearchDown;

  std::list < wxString > m_FindStrings;
  std::list < wxString >::const_iterator m_FindsIterator;
  std::list < wxString > m_ReplaceStrings;
  std::regex m_FindRegEx;

  static wxExFindReplaceData* m_Self;
};

/// Offers a find text ctrl.
/// Pressing key up and down browses through values from
/// wxExFindReplaceData, and pressing enter sets value
/// in wxExFindReplaceData.
class wxExFindTextCtrl : public wxTextCtrl
{
public:
  /// Constructor. 
  /// Fills the text ctrl with value from wxExFindReplaceData.
  wxExFindTextCtrl(
    wxWindow* parent,
    wxWindowID id = wxID_ANY,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize);
};
