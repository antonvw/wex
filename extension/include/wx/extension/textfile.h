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

/// Offers file statistics for elements and keywords.
/// Used in wxExTextFile to keep statistics like comments and lines of code.
/// These are stored as elements.
class WXDLLIMPEXP_BASE wxExFileStatistics
{
  friend class wxExTextFile;
public:
  /// Adds other statistics.
  wxExFileStatistics& operator+=(const wxExFileStatistics& s) {
    m_Elements += s.m_Elements;
    m_Keywords += s.m_Keywords;
    return *this;}

  /// Gets all items as a string. All items are returned as a string,
  /// with newlines separating items.
  const wxString Get() const {
    return m_Elements.Get() + m_Keywords.Get();};

  /// Gets the key, first the elements are tried,
  /// if not present, the keywords are tried, if not present
  /// 0 is returned.
  long Get(const wxString& key) const;

  /// Gets the const elements.
  const wxExStatistics<long>& GetElements() const {return m_Elements;};

  /// Gets the const keywords.
  const wxExStatistics<long>& GetKeywords() const {return m_Keywords;};
private:
  wxExStatistics<long> m_Elements;
  wxExStatistics<long> m_Keywords;
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
  virtual bool Parse();
  
  // Virtual report generators.
  /// This one is invoked during parsing of lines.
  virtual void Report(size_t WXUNUSED(line)) {;};
protected:
  /// Increments the actions completed.
  void IncActionsCompleted(long inc_value = 1) {
    m_Stats.m_Elements.Inc(_("Actions Completed"), inc_value);};
    
  /// Increments keyword.
  void IncKeyword(const wxString& keyword) {
    m_Stats.m_Keywords.Inc(keyword);};

  bool m_Modified;
private:
  bool IsWordCharacter(int c) const {
    return isalnum(c) || c == '_';};
  bool MatchLine(wxString& line);
  /// Returns true if char is alphanumeric or a _ sign.

  wxExFileName m_FileName;
  wxExFileStatistics m_Stats;
  const wxExTool m_Tool;

  std::string m_FindString;
};
#endif
