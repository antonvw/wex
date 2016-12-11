////////////////////////////////////////////////////////////////////////////////
// Name:      stream.h
// Purpose:   Declaration of wxExStream class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/extension/filename.h>
#include <wx/extension/stream-statistics.h>
#include <wx/extension/tool.h>

class wxExFindReplaceData;

/// Adds RunTool methods and statistics to a file stream.
class WXDLLIMPEXP_BASE wxExStream
{
public:
  /// Constructor.
  wxExStream(
    const wxExFileName& filename,
    const wxExTool& tool);
  
  /// Destructor.
 ~wxExStream() {;};

  /// Returns the filename.
  const auto & GetFileName() const {return m_FileName;};

  /// Returns the statistics.
  const auto & GetStatistics() const {return m_Stats;}

  /// Returns the tool.
  const auto & GetTool() const {return m_Tool;};
  
  /// Runs the tool.
  bool RunTool();
protected:
  /// Processes line.
  /// The default performs a ID_TOOL_REPORT_FIND or REPLACE.
  virtual bool Process(
    /// contents of the line
    std::string& line, 
    /// line number
    size_t line_no);
  
  /// Override to do action before processing begins.
  /// The default checks correct tool for find and replace.
  virtual bool ProcessBegin();
  
  /// Override to do action after processing has ended.
  virtual void ProcessEnd() {;};
  
  /// Override to do action for a match.
  /// The line contains matching line, data is 
  /// available in find replace data.
  virtual void ProcessMatch(const std::string& line, size_t line_no) {;};
protected:
  /// Increments the actions completed.
  void IncActionsCompleted(int inc_value = 1) {
    m_Stats.m_Elements.Inc(_("Actions Completed").ToStdString(), inc_value);};
    
  /// Increments statistics keyword.
  void IncStatistics(const std::string& keyword) {
    m_Stats.m_Elements.Inc(keyword);};
private:
  bool IsWordCharacter(int c) const {return isalnum(c) || c == '_';};

  const wxExFileName m_FileName;
  const wxExTool m_Tool;

  wxExStreamStatistics m_Stats;

  int m_Prev;
  bool m_Write = false;
  
  wxExFindReplaceData* m_FRD;

  std::string m_FindString;
};
