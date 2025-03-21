////////////////////////////////////////////////////////////////////////////////
// Name:      lex-rfw.h
// Purpose:   Declaration of Scintilla::lex_rfw class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <memory>
#include <string>
#include <utility>
#include <vector>

#include <wex/core/regex-part.h>

#include "lex-rfw-access.h"
#include "lex-rfw-defs.h"

namespace Scintilla
{
/// The sections known by RFW.
enum section_t
{
  SECTION_UNKNOWN,
  SECTION_COMMENT,
  SECTION_KEYWORD,
  SECTION_SETTING,
  SECTION_TASK,
  SECTION_TESTCASE,
  SECTION_VARIABLE,
};

/// Collects bool options
class options_rfw
{
  friend class option_set_rfw;

public:
  /// Access to members.

  bool fold() const { return m_fold; }
  bool fold_comment() const { return m_fold_comment; }
  bool fold_compact() const { return m_fold_compact; }
  bool fold_pipes() const { return m_fold_pipes; }
  bool fold_tabs() const { return m_fold_tabs; }
  bool vi_script() const { return m_vi_script; }

private:
  bool m_fold{false}, m_fold_comment{false}, m_fold_compact{false},
    m_fold_pipes{false}, m_fold_tabs{false}, m_vi_script{false};
};

/// Collects option set
class option_set_rfw : public OptionSet<options_rfw>
{
public:
  /// Returns keywords.
  static auto keywords() { return m_keywords; }

  /// Default constructor.
  option_set_rfw()
  {
    DefineProperty("fold", &options_rfw::m_fold);
    DefineProperty("fold.comment", &options_rfw::m_fold_comment);
    DefineProperty("fold.compact", &options_rfw::m_fold_compact);
    DefineProperty("fold.pipes", &options_rfw::m_fold_pipes);
    DefineProperty("fold.tabs", &options_rfw::m_fold_tabs);
    DefineProperty("vi.Script", &options_rfw::m_vi_script);

    DefineWordListSets(m_keywords);
  }

private:
  static inline const char* m_keywords[] = {
    "Primary Keywords",
    "Secondary Keywords",
    0};
};

/// Collects section info.
class rfw_section
{
public:
  /// Gets current section id.
  auto id() const { return m_id; }

  /// Returns true if id is testcase or task.
  bool is_case() const
  {
    return m_id == SECTION_TESTCASE || m_id == SECTION_TASK;
  };

  /// Resets section.
  void reset() { m_id = SECTION_UNKNOWN; }

  /// Starts a new section.
  void start(section_t id)
  {
    m_id = id;
    ;
  }

private:
  section_t m_id{SECTION_UNKNOWN};
};

/// The robotframework lexer class.
class lex_rfw : public DefaultLexer
{
public:
  /// Static interface.

  /// Returns and creates the lexers object.
  static inline ILexer5* get() { return new lex_rfw(); }

  /// Returns language.
  static inline int language() { return SCLEX_AUTOMATIC; }

  /// Returns lexer name.
  static inline const char* name() { return "rfw"; }

  /// Subable styles.
  static inline char style_subable[] = {SCE_SH_IDENTIFIER, SCE_SH_SCALAR, 0};

private:
  /// Default constructor.
  lex_rfw();

  /// Destructor.
  virtual ~lex_rfw() = default;

  /// Override methods.

  int SCI_METHOD AllocateSubStyles(int styleBase, int numberStyles) override
  {
    return m_sub_styles.Allocate(styleBase, numberStyles);
  }

  const char* SCI_METHOD DescribeProperty(const char* name) override
  {
    return m_option_set.DescribeProperty(name);
  }

  const char* SCI_METHOD DescribeWordListSets() override
  {
    return m_option_set.DescribeWordListSets();
  }

  int SCI_METHOD DistanceToSecondaryStyles() override { return 0; }

  void SCI_METHOD Fold(
    Sci_PositionU startPos,
    Sci_Position  length,
    int           initStyle,
    IDocument*    pAccess) override;

  void SCI_METHOD FreeSubStyles() override { m_sub_styles.Free(); }

  const char* SCI_METHOD GetSubStyleBases() override { return style_subable; }

  void SCI_METHOD Lex(
    Sci_PositionU startPos,
    Sci_Position  length,
    int           initStyle,
    IDocument*    pAccess) override;

  int SCI_METHOD PrimaryStyleFromStyle(int style) override { return style; }

  void* SCI_METHOD PrivateCall(int, void*) override { return 0; }

  const char* SCI_METHOD PropertyGet(const char* key) override
  {
    return m_option_set.PropertyGet(key);
  };

  const char* SCI_METHOD PropertyNames() override
  {
    return m_option_set.PropertyNames();
  };

  int SCI_METHOD PropertyType(const char* name) override
  {
    return m_option_set.PropertyType(name);
  };

  Sci_Position SCI_METHOD PropertySet(const char* key, const char* val) override
  {
    if (m_option_set.PropertySet(&m_options, key, val))
    {
      return 0;
    }

    return -1;
  };

  void SCI_METHOD Release() override;

  void SCI_METHOD SetIdentifiers(int style, const char* identifiers) override
  {
    m_sub_styles.SetIdentifiers(style, identifiers);
  };

  int SCI_METHOD StyleFromSubStyle(int subStyle) override
  {
    const int styleBase = m_sub_styles.BaseStyle(subStyle);
    return styleBase;
  };

  int SCI_METHOD SubStylesStart(int styleBase) override
  {
    return m_sub_styles.Start(styleBase);
  };

  int SCI_METHOD SubStylesLength(int styleBase) override
  {
    return m_sub_styles.Length(styleBase);
  };

  Sci_Position SCI_METHOD WordListSet(int n, const char* wl) override;

  // Other methods.

  void init();

  void keywords_update();

  void parse_keyword(StyleContext& sc, int cmdState, int& cmd_state_new);

  typedef std::pair<std::string, section_t> section_keyword_t;
  typedef std::vector<section_keyword_t>    section_keywords_t;

  bool section_keywords_detect(
    const std::string& word,
    StyleContext&      sc,
    int&               cmd_state_new);
  void section_start(
    const section_keyword_t& section,
    StyleContext&            sc,
    int&                     cmd_state_new);
  bool spaced_keywords_detect(
    const std::string& word,
    StyleContext&      sc,
    int&               cmd_state_new);

  void state_check(StyleContext& sc, int state, int& state_new);
  bool state_check_continue(StyleContext& sc, int& state);

  SubStyles m_sub_styles;

  WordList m_keywords1, m_keywords2, m_cmd_delimiter;

  options_rfw    m_options;
  option_set_rfw m_option_set;
  rfw_section    m_section;

  bool m_visual_mode{true};
  int  m_style_prev{-1};

  std::unique_ptr<quote>           m_quote;
  std::unique_ptr<quote_stack>     m_quote_stack;
  std::unique_ptr<wex::regex_part> m_regex_section_begin, m_regex_section_end;
  std::unique_ptr<LexAccessor>     m_accessor;

  section_keywords_t m_section_keywords;

  std::vector<std::string> m_spaced_keywords;
};
} // namespace Scintilla
