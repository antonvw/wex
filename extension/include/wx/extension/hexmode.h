////////////////////////////////////////////////////////////////////////////////
// Name:      hexmode.h
// Purpose:   Declaration of class wxExHexMode
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>

class wxExHexModeLine;
class wxExSTC;

/// Offers a hex mode.
class WXDLLIMPEXP_BASE wxExHexMode
{
  friend wxExHexModeLine;
public:
  /// Constructor.
  wxExHexMode(
    /// stc to view in hexmode
    wxExSTC* stc,
    /// hex field                                       ascii field
    /// 23 69 6e 63 6c 75 64 65 20 3c 77 78 2f 63 6d 64 #include <wx/cmd
    /// 6c 69 6e 65 2e 68 3e 20 2f 2f 20 66 6f 72 20 77 line.h> // for w
    /// 78 43 6d 64 4c 69 6e 65 50 61 72 73 65 72 0a 23 xCmdLineParser #
    /// <---------------------------------------------> bytesPerLine
    wxFileOffset bytesPerLine = 16);
  
  /// Returns true if hex mode is on.
  bool Active() const {return m_Active;};
  
  /// If hex mode is on, appends hex mode lines to stc component. 
  /// The text should be normal ascii text, it is encoded while appending.
  void AppendText(const std::string& text);
  
  /// Shows a control char dialog.  
  void ControlCharDialog(const std::string& caption);
  
  /// Deletes chars at current index at current line for
  /// both ascii and hex field.
  bool Delete(int count = 1, int pos = -1);
  
  /// Returns the buffer.
  /// The buffer contains the normal text, without hex info.
  const auto & GetBuffer() const {return m_Buffer;};
  
  /// Returns info about current index,
  /// depending on which field is current.
  const std::string GetInfo();

  /// Asks for a byte offset goes to that byte.
  bool GotoDialog();

  /// Returns STC component.
  auto * GetSTC() {return m_STC;};
  
  /// Highlights the corresponding char for the other field
  /// for the current position.
  bool HighlightOther();

  /// Highlights the corresponding char for the other field.
  bool HighlightOther(int pos);

  /// Inserts ascii text at position.
  /// Insert is only possible at ascii field.
  bool Insert(const std::string& text, int pos = -1);
  
  /// Replaces current line at current index (if pos -1) with char for
  /// both ascii and hex field. Otherwise at specified pos.
  bool Replace(char c, int pos = -1);
  
  /// Replaces target with replacement text.
  /// This is only possible for hex the field, 
  /// therefore the target start and target end should be within
  /// the hex field.
  /// Returns false if target outside area, or replacement
  /// text has invalid chars, or doc is readonly.
  bool ReplaceTarget(
    /// should contain hex codes (uppercase): 303AFF.
    const std::string& replacement,
    /// invokes SetText after replacing.
    bool settext = true);

  /// Sets hex mode.  
  void Set(bool on);

  /// Sets caret pos on stc, depending on event and 
  /// where we are.
  void SetPos(const wxKeyEvent& event);
  
  /// Sets text, if hex mode is on. 
  /// The text should be normal ascii text, it is encoded while appending.
  void SetText(const std::string text);

  /// Undo change, sets the buffer to the original buffer.
  void Undo();
private:
  void Activate();
  void Deactivate();
  
  const wxFileOffset m_BytesPerLine, m_EachHexField;

  bool m_Active = false;
  int m_Goto = 0;
  
  std::string m_Buffer, m_BufferOriginal;
  wxExSTC* m_STC;
};
