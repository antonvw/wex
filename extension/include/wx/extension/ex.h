////////////////////////////////////////////////////////////////////////////////
// Name:      ex.h
// Purpose:   Declaration of class wxExEx
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <functional>
#include <map>
#include <string>
#include <utility>
#include <vector>
#include <wx/extension/ex-command.h>
#include <wx/extension/marker.h>

#if wxUSE_GUI

class ex_evaluator;
class wxExCTags;
class wxExManagedFrame;
class wxExSTC;
class wxExSTCEntryDialog;
class wxExViMacros;
class wxExViMacrosMode;

enum class wxExInfoMessage;

/// Offers a class that adds ex editor to wxExSTC.
class WXDLLIMPEXP_BASE wxExEx
{
  friend class wxExViMacrosMode;
public:
  /// Constructor. 
  /// Sets ex mode.
  wxExEx(wxExSTC* stc);
  
  /// Destructor.
  virtual ~wxExEx();
  
  /// Adds text (to STC or register, if register is active).
  void AddText(const std::string& text);
 
  /// Returns calculated value of text.
  double Calculator(
    /// text used for calculation
    const std::string& text, 
    /// width, or precision, for doubles
    int& width);
  
  /// Cleans up.
  void Cleanup();

  /// Executes ex: command that was entered on the command line,
  /// or present as modeline command inside a file.
  /// Returns true if the command was executed.
  virtual bool Command(const std::string& command);

  /// Copies data from other component.
  void Copy(const wxExEx* ex);
  
  /// Cuts selected text to yank register,
  /// and updates delete registers.
  void Cut(bool show_message = true);

  /// Returns command.
  const auto & GetCommand() const {return m_Command;};

  /// Returns the ctags.
  auto & GetCTags() {return m_CTags;};
  
  /// Returns frame.
  auto* GetFrame() {return m_Frame;};
  
  /// Returns whether ex is active.
  auto GetIsActive() const {return m_IsActive;};
  
  /// Returns last entered command.
  const auto & GetLastCommand() const {return m_LastCommand;};
  
  /// Returns the macros.
  static auto & GetMacros() {return m_Macros;};

  /// Returns register name.
  const auto GetRegister() const {return m_Register;};
  
  /// Returns text to be inserted.
  const std::string GetRegisterInsert() const;
  
  /// Returns text from current register (or yank register if no register active).
  const std::string GetRegisterText() const;
  
  /// Returns search flags.
  auto GetSearchFlags() const {return m_SearchFlags;};
  
  /// Returns selected text as a string.
  const std::string GetSelectedText() const;
  
  /// Returns STC component.
  auto * GetSTC() {return m_Command.STC();};

  /// Writes info message.
  void InfoMessage() const;
  
  /// Adds marker at the specified line.
  /// Returns true if marker could be added.
  bool MarkerAdd(
    /// marker
    char marker,
    /// line to add marker, default current line
    int line = -1);
  
  /// Deletes specified marker.
  /// Returns true if marker was deleted.
  bool MarkerDelete(char marker);
  
  /// Goes to specified marker.
  /// Returns true if marker exists.
  bool MarkerGoto(char marker);
  
  /// Returns line for specified marker.
  /// Returns -1 if marker does not exist.
  int MarkerLine(char marker) const;
  
  /// Prints text in the dialog.
  void Print(const std::string& text);

  /// Resets search flags.
  void ResetSearchFlags();
  
  /// Sets delete registers 1 - 9 (if value not empty).
  void SetRegistersDelete(const std::string& value) const;
  
  /// Sets insert register (if value not empty).
  void SetRegisterInsert(const std::string& value) const;
  
  /// Sets yank register (if value not empty).
  void SetRegisterYank(const std::string& value) const;
  
  /// Set using ex mode.
  void Use(bool mode) {m_IsActive = mode;};
  
  /// Yanks selected text to yank register, default to yank register.
  /// Returns false if no text was selected.
  bool Yank(const char name = '0', bool show_message = true) const;
protected:
  /// Sets last command.
  void SetLastCommand(
    const std::string& command,
    bool always = false);
  
  /// Sets register name.
  /// Setting register 0 results in
  /// disabling current register.
  void SetRegister(const char name) {m_Register = name;};

  wxExExCommand m_Command;
private:
  bool CommandHandle(const std::string& command) const;
  bool CommandAddress(const std::string& command);
  template <typename S, typename T> 
    bool HandleContainer(
      const std::string& kind,
      const std::string& command,
      const T * container,
      std::function<bool(const std::string&, const std::string&)> cb);
  void InfoMessage(const std::string& text, wxExInfoMessage type) const;
  template <typename S, typename T>
  std::string ReportContainer(const T & container) const;
  void ShowDialog(
    const std::string& title, const std::string& text, bool prop_lexer = false);
    
  const wxExMarker m_MarkerSymbol = wxExMarker(0);

  // relate a marker to identifier
  std::map<char, int> m_MarkerIdentifiers;
  // relate a marker to mark number
  std::map<char, int> m_MarkerNumbers;
  
  static std::string m_LastCommand;
  static wxExSTCEntryDialog* m_Dialog;
  static wxExViMacros m_Macros;
  static ex_evaluator m_Evaluator;

  bool m_IsActive {true}; // are we actively using ex mode?
  bool m_Copy {false}; // this is a copy, result of split
  
  int m_SearchFlags;
  
  char m_Register {0};
  
  wxExManagedFrame* m_Frame;  
  wxExCTags* m_CTags {nullptr};

  const std::vector<std::pair<
    const std::string, 
    std::function<bool(const std::string& command)>>> m_Commands;
};
#endif // wxUSE_GUI
