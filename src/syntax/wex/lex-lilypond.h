////////////////////////////////////////////////////////////////////////////////
// Name:      lex-lilypond.h
// Purpose:   Declaration of Scintilla::lex_lilypond
//            Based on Scintilla::LexLatex
// Author:    Anton van Wezenbeek
// Copyright: (c) 2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

namespace Scintilla
{
/// Collects bool options
class lex_options
{
  friend class lex_option_set;

public:
  bool fold() const { return m_fold; };
  bool fold_comment() const { return m_fold_comment; };
  bool fold_compact() const { return m_fold_compact; };

private:
  bool m_fold{false}, m_fold_comment{false}, m_fold_compact{false};
};

/// Collects option set
class lex_option_set : public OptionSet<lex_options>
{
public:
  /// Default constructor.
  lex_option_set()
  {
    DefineProperty("fold", &lex_options::m_fold);
    DefineProperty("fold.comment", &lex_options::m_fold_comment);
    DefineProperty("fold.compact", &lex_options::m_fold_compact);
  }
};

/// The lilypond lexer class.
class lex_lilypond : public DefaultLexer
{
public:
  /// Static interface.

  /// Returns and creates the lexers object.
  static inline ILexer5* get() { return new lex_lilypond(); }

  /// Returns language.
  static inline int language() { return SCLEX_AUTOMATIC; };

  /// Returns lexer name.
  static inline const char* name() { return "lilypond"; };

private:
  struct fold_save
  {
    fold_save()
      : m_open_begins(8)
    {
      ;
    };

    int to_int() const
    {
      int sum = 0;
      for (int i = 0; i <= m_level; ++i)
        sum += m_open_begins[i];
      return ((sum + m_level + SC_FOLDLEVELBASE) & SC_FOLDLEVELNUMBERMASK);
    }

    std::vector<int> m_open_begins;
    int              m_level{0};
  };

  /// Default constructor.
  lex_lilypond();

  /// Overide methods.

  void SCI_METHOD Fold(
    Sci_PositionU startPos,
    Sci_Position  length,
    int           initStyle,
    IDocument*    pAccess) override;

  void SCI_METHOD Lex(
    Sci_PositionU startPos,
    Sci_Position  length,
    int           initStyle,
    IDocument*    pAccess) override;

  const char* SCI_METHOD PropertyGet(const char* key) override
  {
    return m_option_set.PropertyGet(key);
  };

  const char* SCI_METHOD PropertyNames() override
  {
    return m_option_set.PropertyNames();
  };

  Sci_Position SCI_METHOD PropertySet(const char* key, const char* val) override
  {
    if (m_option_set.PropertySet(&m_options, key, val))
    {
      return 0;
    }

    return -1;
  };

  int SCI_METHOD PropertyType(const char* name) override
  {
    return m_option_set.PropertyType(name);
  };

  /// Other methods.

  void fold_dec(int& lev, fold_save& save) const;
  void fold_inc(int& lev, fold_save& save, bool& need) const;

  int get_mode(Sci_Position line) const
  {
    if (line >= 0 && line < static_cast<Sci_Position>(m_modes.size()))
      return m_modes[line];
    return 0;
  }

  void get_save(Sci_Position line, fold_save& save) const
  {
    if (line >= 0 && line < static_cast<Sci_Position>(m_saves.size()))
      save = m_saves[line];
    else
    {
      save.m_level = 0;
      for (int i = 0; i < 8; ++i)
        save.m_open_begins[i] = 0;
    }
  }

  int mode_to_state(int mode) const;

  void resize_modes(Sci_Position numLines)
  {
    if (static_cast<Sci_Position>(m_modes.size()) > numLines * 2 + 256)
      m_modes.resize(numLines + 128);
  }

  void resize_saves(Sci_Position numLines)
  {
    if (static_cast<Sci_Position>(m_saves.size()) > numLines * 2 + 256)
      m_saves.resize(numLines + 128);
  }

  void set_modes(Sci_Position line, int mode)
  {
    if (line >= static_cast<Sci_Position>(m_modes.size()))
      m_modes.resize(line + 1, 0);
    m_modes[line] = mode;
  }

  void set_saves(Sci_Position line, const fold_save& save)
  {
    if (line >= static_cast<Sci_Position>(m_saves.size()))
      m_saves.resize(line + 1);
    m_saves[line] = save;
  }

  lex_options    m_options;
  lex_option_set m_option_set;

  std::vector<int>       m_modes;
  std::vector<fold_save> m_saves;
};
} // namespace Scintilla
