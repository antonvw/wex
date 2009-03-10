/******************************************************************************\
* File:          file.h
* Purpose:       Declaration of wxWidgets file extension classes
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2008, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EXFILE_H
#define _EXFILE_H

#include <sys/stat.h> // for stat
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/file.h>
#include <wx/filename.h>
#include <wx/extension/lexer.h>

class exConfigDialog;

/// Adds IsOk to the stat base class, and several methods
/// to get/update on the stat members.
class exStat : public stat
{
public:
  /// Default constructor. Calls Update.
  exStat(const wxString& fullpath = wxEmptyString) {
    Update(fullpath);}

#if wxUSE_GUI
  /// Shows a dialog with colour options, returns dialog return code.
  /// Always modeless, it uses the dialog id as specified,
  /// so you can use that id in exFrame::ConfigDialogApplied.
  static int ConfigDialog(
    const wxString& title = _("Colour Options"),
    wxWindow* parent = wxTheApp->GetTopWindow(),
    wxWindowID id = wxID_ANY);
#endif

  /// Returns a colour depending on modification time.
  /// Colour setup is taken from config.
  /// Method GetColouring is available through exLexer, and returns syntax colouring.
  /// Default the colours are stored under the config group as specified.
  const wxColour GetColour() const;

  /// Gets fullpath member.
  const wxString& GetFullPath() const {return m_FullPath;};
  
  /// Returns a colour if this is a link.
  const wxColour GetLinkColour() const;

  /// Gets the modification time.
  /// From wxFileName class GetModificationTime is available as well,
  /// this one returns string and only uses the stat member, and is fast.
  const wxString GetModificationTime(
    const wxString& format = wxDefaultDateTimeFormat) const;

  /// Returns true if this stat is a link.
  bool IsLink() const;

  /// Returns true if the stat is okay (last update was okay).
  bool IsOk() const {return m_IsOk;};

  /// Returns true if this stat is readonly.
  bool IsReadOnly() const {
    return (m_IsOk && ((st_mode & wxS_IWUSR) == 0));};

  /// Sets readonly as specified.
  bool SetReadOnly(const bool read_only);

  /// Updates stat only, returns result and keeps result in IsOk.
  bool Sync() {
    m_IsOk = (::stat(m_FullPath.c_str(), this) != -1);
    return m_IsOk;};

  /// Updates fullpath member, returns result and keeps result in IsOk.
  bool Update(const wxString& fullpath) {
    m_FullPath = fullpath;
    m_IsOk = (::stat(m_FullPath.c_str(), this) != -1);
    return m_IsOk;};
private:
#if wxUSE_GUI
  static exConfigDialog* m_ConfigDialog;
#endif
  wxString m_FullPath;
  bool m_IsOk;
};

/// Flags for exStatusText.
enum exStatusFlags
{
  STAT_DEFAULT  = 0x0000, ///< shows 'modified' and file 'fullname'
  STAT_SYNC     = 0x0001, ///< shows 'synchronized' instead of 'modified'
  STAT_FULLPATH = 0x0002, ///< shows file 'fullpath' instead of 'fullname'
};

/// Adds an exStat and an exLexer member to wxFileName.
class exFileName : public wxFileName
{
public:
  /// Default constructor.
  exFileName(
    const wxString& fullpath = wxEmptyString,
    wxPathFormat format = wxPATH_NATIVE)
    : wxFileName(fullpath, format)
    , m_Stat(fullpath) {
    SetLexer();}

  /// Copy constructor from a wxFileName.
  exFileName(const wxFileName& filename)
    : wxFileName(filename) 
    , m_Stat(filename.GetFullPath()) {
    SetLexer();}

  /// Gets the icon index for this filename (uses the file extension to get it).
  int GetIcon() const;

  /// Gets the lexer.
  exLexer& GetLexer() {return m_Lexer;};

  /// Gets the lexer.
  const exLexer& GetLexer() const {return m_Lexer;};

  /// Gets the stat.
  const exStat& GetStat() const {return m_Stat;};

  /// Gets the stat.
  exStat& GetStat() {return m_Stat;};

  /// If specified lexer is empty, use one of the lexers from config
  /// according to match on the file fullname.
  /// Otherwise use specified lexer.
  /// Text is used if lexer is empty, to override settings from config
  /// if no match was found, to match special cases.
  void SetLexer(
    const wxString& lexer = wxEmptyString,
    const wxString& text = wxEmptyString);

#if wxUSE_STATUSBAR
  /// Shows filename info on the statusbar.
  void StatusText(long flags = STAT_DEFAULT) const;
#endif // wxUSE_STATUSBAR

private:
  exLexer m_Lexer;
  exStat m_Stat;
};

/// Adds an exFileName, an exStat member that can be used for synchronization,
/// and several File* methods to wxFile. All the File* methods update
/// the exStat member.
class exFile : public wxFile
{
public:
  /// Default constructor.
  exFile();

  /// Opens a file with a filename.
  exFile(const wxString& filename, wxFile::OpenMode mode = wxFile::read);

  /// Destructor.
  /// NB: for wxFile the destructor is not virtual so you should not use wxFile polymorphically.
  /// So do it here.
  virtual ~exFile() {;};

  /// Asks for continue, sets the filename member.
  virtual bool FileNew(const exFileName& filename = exFileName());

  /// Asks for continue, sets the filename member and opens the file.
  virtual bool FileOpen(const exFileName& filename);

  /// Invoked by FileSaveAs, allows you to save your file.
  /// The default closes the file.
  virtual bool FileSave();

  /// Shows file dialog and calls FileSave.
  virtual bool FileSaveAs();

  /// Called if file needs to be synced.
  /// The default calls FileOpen, and updates status text.
  virtual void FileSync();
  
  /// Returns whether contents have been changed.
  /// Default returns false.
  virtual bool GetContentsChanged() {return false;};

  /// Reset contents changed.
  /// Default does nothing.
  virtual void ResetContentsChanged() {;};

  /// Invoked ShowModal on dialog, and returns dialog return code.
  int AskFileOpen(wxFileDialog& dlg, bool ask_for_continue = true);

  /// Returns true if this file should be synced.
  /// Invokes FileSync if true.
  bool CheckSyncNeeded();
  
  /// Shows dialog if file contents was changed, and returns true if
  /// you accepted to save changes.
  bool Continue();

  /// Gets the file name.
  const exFileName& GetFileName() const {return m_FileName;}

  /// Gets the stat.
  /// By comparing this with the stat from GetFileName
  /// you can detect whether the file needs to be synced.
  const exStat& GetStat() const {return m_Stat;};

  /// Reads this file into a buffer.
  /// The buffer is allocated by the lib, you should delete the string after using.
  wxString* Read(wxFileOffset seek_position = 0);

  /// Sets the wild card member.
  void SetWildcard(const wxString& wildcard) {m_Wildcard = wildcard;};
protected:
  exFileName m_FileName; ///< the filename
private:
  // Take care that filename and stat are in sync.
  bool MakeAbsolute();
  
  exStat m_Stat;
  wxString m_Message;
  wxString m_Wildcard;
};
#endif
