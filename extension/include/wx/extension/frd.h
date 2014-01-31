////////////////////////////////////////////////////////////////////////////////
// Name:      frd.h
// Purpose:   Declaration of wxExFindReplaceData class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2014 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXFRD_H
#define _EXFRD_H

#include <list>
#include <wx/fdrepdlg.h> // for wxFindReplaceData
#include <wx/regex.h>
#include <wx/textctrl.h>

class wxExFindTextCtrl;

/// Adds an existing config to wxFindReplaceData, and some members.
class WXDLLIMPEXP_BASE wxExFindReplaceData : public wxFindReplaceData
{
  friend class wxExFindTextCtrl;
public:
  // Destructor (not for Doxy).
 ~wxExFindReplaceData();
 
  /// Gets the find replace data.
  static wxExFindReplaceData* Get(bool createOnDemand = true);

  /// Gets the find strings.
  const std::list < wxString > & GetFindStrings() const {
    return m_FindStrings;};

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
  /// one of the (boolean) text fields.
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
protected:
  void OnEnter(wxCommandEvent& event);
  void OnKey(wxKeyEvent& event);
private:
  std::list < wxString >::const_iterator m_FindsIterator;

  DECLARE_EVENT_TABLE()
};

#endif
