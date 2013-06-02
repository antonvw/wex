////////////////////////////////////////////////////////////////////////////////
// Name:      textfile.h
// Purpose:   Declaration of wxExTextFile class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2012 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXTEXTFILE_H
#define _EXTEXTFILE_H

#include <wx/textfile.h>
#include <wx/extension/filename.h>
#include <wx/extension/statistics.h>
#include <wx/extension/tool.h>

class wxExTextFile;

/// Offers file statistics.
/// Used by wxExTextFile to keep statistics.
class WXDLLIMPEXP_BASE wxExFileStatistics
{
  friend class wxExTextFile;
public:
  /// Adds other statistics.
  wxExFileStatistics& operator+=(const wxExFileStatistics& s) {
    m_Elements += s.m_Elements;
    return *this;}

  /// Gets all items as a string. All items are returned as a string,
  /// with newlines separating items.
  const wxString Get() const {return m_Elements.Get();};

  /// Gets the key, if not present 0 is returned.
  int Get(const wxString& key) const {
#ifdef wxExUSE_CPP0X  
    const auto it = m_Elements.GetItems().find(key);
#else
    std::map<wxString, int>::const_iterator it = m_Elements.GetItems().find(key);  
#endif  
    return (it != m_Elements.GetItems().end() ? it->second: 0);};

  /// Gets the elements.
  const wxExStatistics<int>& GetElements() const {return m_Elements;};
private:
  wxExStatistics<int> m_Elements;
};

/// Adds file tool methods to wxTextFile.
/// In your derived class just implement the Report, and take
/// care that the strings are added to your component.
class WXDLLIMPEXP_BASE wxExTextFile : public wxTextFile
{
public:
  /// Constructor.
  wxExTextFile(
    const wxExFileName& filename,
    const wxExTool& tool);

  /// Gets the filename.
  const wxExFileName& GetFileName() const {return m_FileName;};

  /// Gets the statistics.
  const wxExFileStatistics& GetStatistics() const {return m_Stats;}

  /// Gets the tool.
  const wxExTool& GetTool() const {return m_Tool;};

  /// Runs the tool (opens the file before running and closes afterwards).
  bool RunTool();
protected:
  /// Parses the file (already opened).
  /// This implementation offers find and replace in files.
  virtual bool Parse();
  
  /// This one is invoked during parsing of lines when a find or replace
  /// match is found. The line contains matching line, data is also
  /// available in find replace data.
  virtual void Report(size_t WXUNUSED(line)) {;};
protected:
  /// Increments the actions completed.
  void IncActionsCompleted(int inc_value = 1) {
    m_Stats.m_Elements.Inc(_("Actions Completed"), inc_value);};
    
  /// Increments statistics keyword.
  void IncStatistics(const wxString& keyword) {
    m_Stats.m_Elements.Inc(keyword);};

  bool m_Modified;
private:
  /// Returns true if char is alphanumeric or a _ sign.
  bool IsWordCharacter(int c) const {
    return isalnum(c) || c == '_';};
  bool MatchLine(wxString& line);

  const wxExFileName m_FileName;
  const wxExTool m_Tool;
  
  wxExFileStatistics m_Stats;

  std::string m_FindString;
};
#endif
