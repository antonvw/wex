////////////////////////////////////////////////////////////////////////////////
// Name:      frd.h
// Purpose:   Declaration of wex::find_replace_data class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <list>
#include <regex>
#include <string>
#include <wx/fdrepdlg.h> // for wxFindReplaceData
#include <wex/textctrl.h>

namespace wex
{
  /// Adds an existing config to wxFindReplaceData, and some members.
  class find_replace_data
  {
    friend class textctrl;
    friend class find_textctrl;
  public:
    /// Destructor, writes data to config.
   ~find_replace_data();
   
    /// Returns the find replace data.
    static find_replace_data* Get(bool createOnDemand = true);

    /// Returns the find string.
    const auto GetFindString() const {return m_FRD.GetFindString().ToStdString();};

    /// Returns the find strings.
    const auto & GetFindStrings() const {return m_FindStrings.GetValues();};
    
    /// Returns flags.
    int GetFlags() const {return m_FRD.GetFlags();};
    
    /// Access to data.
    auto & GetFRD() {return m_FRD;};

    /// Returns the replace string.
    const auto GetReplaceString() const {return m_FRD.GetReplaceString().ToStdString();};

    /// Returns the replace strings.
    const auto & GetReplaceStrings() const {return m_ReplaceStrings.GetValues();};

    /// Returns text.
    static const auto & GetTextFindWhat() {return m_TextFindWhat;};

    /// Returns text.
    static const auto & GetTextMatchCase() {return m_TextMatchCase;};

    /// Returns text.
    static const auto & GetTextMatchWholeWord() {return m_TextMatchWholeWord;};

    /// Returns text.
    static const auto & GetTextRegEx() {return m_TextRegEx;};

    /// Returns text.
    static const auto & GetTextReplaceWith() {return m_TextReplaceWith;};

    /// Returns text.
    static const auto & GetTextSearchDown() {return m_TextSearchDown;};
    
    /// Returns true if the flags have match case set.
    bool MatchCase() const {return (m_FRD.GetFlags() & wxFR_MATCHCASE) > 0;};

    /// Returns true if the flags have whole word set.
    bool MatchWord() const {return (m_FRD.GetFlags() & wxFR_WHOLEWORD) > 0;};
    
    /// Returns -1 if GetFindString as regular expression does not match text,
    /// otherwise start pos of match.
    int RegExMatches(const std::string& text) const;
    
    /// Replaces all occurrences of GetFindString as regular expression
    /// in text by GetReplaceString.
    /// Returns number of replacements done in text.
    int RegExReplaceAll(std::string& text) const;

    /// Returns true if the flags have search down set.
    bool SearchDown() const {return (m_FRD.GetFlags() & wxFR_DOWN) > 0;};

    /// Sets the object as the current one, returns the pointer 
    /// to the previous current object 
    /// (both the parameter and returned value may be nullptr). 
    static find_replace_data* Set(find_replace_data* frd);

    /// Sets the find string.
    /// If UseRegEx also sets the regular expression.
    /// This string is used for tool find in files and replace in files.
    /// Als moves the find string to the beginning of the find
    /// strings list.
    void SetFindString(const std::string& value);

    /// Sets the find strings.
    void SetFindStrings(const std::list < std::string > & value);
    
    /// Sets flags.
    void SetFlags(wxUint32 flags) {m_FRD.SetFlags(flags);};
    
    /// Sets flags for match case.
    void SetMatchCase(bool value);

    /// Sets flags for match word.
    void SetMatchWord(bool value);

    /// Sets the replace string.
    void SetReplaceString(const std::string& value);

    /// Sets the replace strings.
    /// Als moves the replace string to the beginning of the replace
    /// strings list.
    void SetReplaceStrings(const std::list < std::string > & value);

    /// Sets using regular expression for find text.
    /// If GetFindString does not contain a valid regular expression
    /// the use member is not set.
    void SetUseRegEx(bool value);

    /// Returns true if find text is used as a regular expression.
    bool UseRegEx() const {return m_UseRegEx;};
  private:
    find_replace_data();

    static std::string m_TextFindWhat;
    static std::string m_TextMatchCase;
    static std::string m_TextMatchWholeWord;
    static std::string m_TextRegEx;
    static std::string m_TextReplaceWith;
    static std::string m_TextSearchDown;
    
    wxFindReplaceData m_FRD;

    bool m_UseRegEx {false};

    textctrl_input m_FindStrings;
    textctrl_input m_ReplaceStrings;

    std::regex m_FindRegEx;

    static find_replace_data* m_Self;
  };
};
