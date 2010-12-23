/******************************************************************************\
* File:          tool.h
* Purpose:       Declaration of wxExTool classes
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EXTOOL_H
#define _EXTOOL_H

#include <map>
#include <wx/extension/defs.h>
#include <wx/filename.h>

/// This class defines our tool info.
/// It is stored in the tool info map available from the wxExTool class.
class WXDLLIMPEXP_BASE wxExToolInfo
{
public:
  /// Default constructor.
  wxExToolInfo(
    const wxString& info = wxEmptyString,
    const wxString& text = wxEmptyString,
    const wxString& helptext = wxEmptyString)
    : m_Info(info)
    , m_HelpText(helptext)
    , m_Text(text){};

  /// Gets the info.
  const wxString& GetInfo() const {return m_Info;};

  /// Gets the helptext.
  const wxString& GetHelpText() const {return m_HelpText;};

  /// Gets the text.
  const wxString& GetText() const {return m_Text;};
private:
  wxString m_Info;
  wxString m_HelpText;
  wxString m_Text;
};

template <class T> class wxExStatistics;

/// Offers tool methods and contains the tool info's.
/// A tool with non empty text is used by wxExMenu::AppendTools.
class WXDLLIMPEXP_BASE wxExTool
{
public:
  /// Constructor, specify the wxExToolId to use.
  wxExTool(int id);

  /// Adds your own info to the tool.
  /// If you use a %ld in the info string, it is replaced by GetStatistics
  /// with the Actions Completed element.
  void AddInfo(
    int tool_id,
    const wxString& info,
    const wxString& text = wxEmptyString,
    const wxString& helptext = wxEmptyString) {
    m_ToolInfo[tool_id] = wxExToolInfo(info, text, helptext);};

  /// Gets the tool object.
  static wxExTool* Get(bool createOnDemand = true);

  /// Gets the tool id.
  int GetId() const {return m_Id;};

  /// Returns the log filename.
  const wxFileName GetLogfileName() const;

  /// Gets all the tool info.
  const std::map < int, wxExToolInfo > & GetToolInfo() const {return m_ToolInfo;};

  /// Gets info about current tool.
  const wxString Info() const;

  // Type checking.
  /// Is this tool the report count.
  bool IsCount() const {
    return m_Id == ID_TOOL_REPORT_COUNT;}

  /// Is this tool a find type.
  bool IsFindType() const {
    return m_Id == ID_TOOL_REPORT_FIND || m_Id == ID_TOOL_REPORT_REPLACE;}

  /// Is this tool a report type.
  bool IsReportType() const {
    return m_Id > ID_TOOL_REPORT_FIRST && m_Id < ID_TOOL_REPORT_LAST;}

  /// Is this tool a RCS type.
  bool IsRCSType() const {
    return
      m_Id == ID_TOOL_REVISION_RECENT ||
      m_Id == ID_TOOL_REPORT_REVISION;}

  /// Is this tool a statistics type.
  bool IsStatisticsType() const {
    return
      m_Id == ID_TOOL_REPORT_COUNT ||
      m_Id == ID_TOOL_REPORT_KEYWORD;}

  /// Logs the statistics to
  /// the statusbar (always) and to the statistics logfile (if specified).
  void Log(
    const wxExStatistics<long>* stat, 
    const wxString& caption = wxEmptyString, 
    bool log_to_file = true) const;

  /// Sets the object as the current one, returns the pointer 
  /// to the previous current object (both the parameter and returned value may be NULL). 
  static wxExTool* Set(wxExTool* tool);
private:
  const int m_Id;
  std::map < int, wxExToolInfo > m_ToolInfo;
  static wxExTool* m_Self;
};
#endif
