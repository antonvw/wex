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
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/defs.h>

/// This class defines our tool info.
/// It is stored in the tool info map available from the wxExTool class.
class wxExToolInfo
{
public:
  /// Constructor.
  wxExToolInfo(
    const wxString& info,
    const wxString& text,
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
  const wxString m_Info;
  const wxString m_HelpText;
  const wxString m_Text;
};

/// Offers tool methods and contains the tool info's.
/// A tool with non empty text is used by wxExMenu::AppendTools.
class wxExTool
{
public:
  /// Constructor, specify the wxExToolId to use.
  wxExTool(int id);

  /// Adds your own info to the tool.
  /// If you use a %ld in the info string, it is replaced by GetStatistics
  /// with the Actions Completed element.
  static void AddInfo(
    int tool_id,
    const wxString& info,
    const wxString& text = wxEmptyString,
    const wxString& helptext = wxEmptyString);

  /// Gets the tool id.
  int GetId() const {return m_Id;};

  /// Gets all the tool info.
  static std::map < int, const wxExToolInfo > & GetToolInfo() {return m_ToolInfo;};

  /// Gets info about current tool.
  const wxString Info() const;

  // Initializes the tool info map.
  // This is done during wxExApp::OnInit.
  // Not for doxygen.
  static void Initialize();

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
private:
  const int m_Id;
  static std::map < int, const wxExToolInfo > m_ToolInfo;
};
#endif
