////////////////////////////////////////////////////////////////////////////////
// Name:      lexer.h
// Purpose:   Declaration of wex::lexer class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>
#include <set>
#include <vector>
#include <wex/property.h>
#include <wex/style.h>

namespace pugi
{
  class xml_node;
};

namespace wex
{
  /// The edge mode as is specified in the lexers file.
  /// When ABSENT the edge mode is shown as specified
  /// by the editor option settings.
  enum class edge_mode
  {
    ABSENT,     ///< no edge mode present
    NONE,       ///< no edge is shown
    LINE,       ///< edge shown as line
    BACKGROUND, ///< adge shown as background colour
  };

  class stc;

  /// This class defines a lexer using file extensions,
  /// syntax colouring and comment definitions.
  /// This lexer is one of the Scintilla lexers.
  /// The lexers are read by and kept in the wex::lexers class.
  class lexer
  {
  public:
    /// Default constructor.
    lexer(const std::string& lexer = std::string())
      : wex::lexer(nullptr, lexer) {;};
      
    /// Constructor using stc, and optional lexer.
    lexer(stc* stc, const std::string& lexer = std::string())
      : m_STC(stc) {
      if (!lexer.empty())
      {
        Set(lexer);
      }};

    /// Constructor using xml node.
    lexer(const pugi::xml_node* node) {Set(node);};

    /// Assignment operator.
    lexer& operator=(const lexer& l);

    /// Adds keywords (public for testing only).
    bool AddKeywords(const std::string& text, int setno = 0);
    
    /// Applies this lexer to stc component (and colours the component).
    bool Apply() const;

    /// Returns a string that completes specified comment,
    /// by adding spaces and a comment end at the end.
    /// If the comment end string is empty, it returns empty string.
    const std::string CommentComplete(const std::string& comment) const;
      
    /// Returns the comment begin.
    const auto & GetCommentBegin() const {return m_CommentBegin;};

    /// Returns the comment begin 2.
    const auto & GetCommentBegin2() const {return m_CommentBegin2;};

    /// Returns the comment end.
    const auto & GetCommentEnd() const {return m_CommentEnd;};

    /// Returns the comment end 2.
    const auto & GetCommentEnd2() const {return m_CommentEnd2;};

    /// Returns the display lexer (as shown in dialog).
    const auto & GetDisplayLexer() const {return m_DisplayLexer;};

    /// Returns the edge mode.
    const auto GetEdgeMode() const {return m_EdgeMode;};

    /// Returns the extensions.
    const auto & GetExtensions() const {return m_Extensions;};

    /// Returns the keywords.
    const auto & GetKeywords() const {return m_Keywords;};

    /// Returns the keywords as one large string, 
    const std::string GetKeywordsString(
      /// if keyword_set -1 take all the sets,
      /// otherwise take the specified set.
      int keyword_set = -1,
      /// if min_size 0, use all keywords,
      /// otherwise use keywords with minimim size
      size_t min_size = 0,
      /// prefix keyword should start with
      const std::string& prefix = std::string()) const;

    /// Returns the language.
    const auto & GetLanguage() const {return m_Language;};
    
    /// Returns the line size.
    size_t GetLineSize() const;
    
    /// Returns the properties.
    const auto & GetProperties() const {return m_Properties;};
    
    /// Returns the scintilla lexer.
    const auto & GetScintillaLexer() const {return m_ScintillaLexer;};

    /// Returns the styles.
    const auto & GetStyles() const {return m_Styles;};
    
    /// Is this word a keyword (allways all keywords), case sensitive.
    bool IsKeyword(const std::string& word) const;

    /// Is this lexer valid.
    bool is_ok() const {return m_is_ok;};

    /// Does any keyword (allways all keywords) start with this word,
    /// case insensitive.
    bool KeywordStartsWith(const std::string& word) const;

    /// Returns a lexer comment string with text formatted.
    const std::string MakeComment(
      const std::string& text,
      bool fill_out_with_space = true,
      bool fill_out = true) const;

    /// Returns a lexer comment string with prefix.
    const std::string MakeComment(
      const std::string& prefix,
      const std::string& text) const;
    
    /// Returns a lexer comment string filled out over one line.
    const std::string MakeSingleLineComment(
      const std::string_view& text,
      bool fill_out_with_space = true,
      bool fill_out = true) const;
      
    /// Returns true if the stc component 
    /// associated with this lexer can be previewed.
    bool Previewable() const {return m_Previewable;};

    /// Resets lexer and applies it to stc.
    /// The is ok member is set to false.
    void Reset();

    /// Sets lexer to specified lexer (finds by name from lexers),
    /// invokes the other Set.
    /// Shows error message when lexer could not be set.
    bool Set(const std::string& lexer, bool fold = false);
      
    /// Sets lexer to specified lexer, and applies it to stc if present. 
    /// Returns true if a scintilla lexer has been set.
    bool Set(const lexer& lexer, bool fold = false);
        
    /// Overrides a local property.
    void SetProperty(const std::string& name, const std::string& value);

    /// Returns number of chars that fit on a line, skipping comment chars.
    size_t UsableCharactersPerLine() const;
  private:
    void AutoMatch(const std::string& lexer);
    const std::string GetFormattedText(
      const std::string& lines,
      const std::string& header,
      bool fill_out_with_space,
      bool fill_out) const;
    void Set(const pugi::xml_node* node);

    // The scintilla name for this lexer cannot be const, 
    // as in path the operator= is used on a lexer.
    // Normally the lexer displayed is the scintilla lexer,
    // however this might be different, as with c#.
    // In that case the scintilla lexer is cpp, whereas the display lexer is c#.  
    std::string 
      m_CommentBegin, m_CommentBegin2, m_CommentEnd, m_CommentEnd2, 
      m_DisplayLexer, m_Extensions, m_Language, m_ScintillaLexer;

    // each keyword set in a separate keyword set
    std::map< int, std::set<std::string> > m_KeywordsSet;
    std::set<std::string> m_Keywords;
    std::vector<size_t> m_EdgeColumns; // last one is used for line size
    std::vector<property> m_Properties;
    std::vector<style> m_Styles;
    
    bool m_is_ok {false}, m_Previewable {false};
    edge_mode m_EdgeMode {edge_mode::ABSENT};
    stc* m_STC {nullptr};
  };
};
