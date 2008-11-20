/******************************************************************************\
* File:          tool.h
* Purpose:       Declaration of exTool classes
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id: tool.h 71 2008-11-17 18:01:42Z anton $
*
* Copyright (c) 1998-2008, Anton van Wezenbeek
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
class exToolInfo
{
public:
  /// Constructor.
  exToolInfo(
    const wxString& info,
    const wxString& text,
    bool is_basic = true,
    const wxString& helptext = wxEmptyString)
    : m_Info(info)
    , m_HelpText(helptext)
    , m_IsBasic(is_basic)
    , m_Text(text){};

  /// Gets the info.
  const wxString& GetInfo() const {return m_Info;};

  /// Gets the helptext.
  const wxString& GetHelpText() const {return m_HelpText;};

  /// Gets the is basic.
  bool GetIsBasic() const {return m_IsBasic;};

  /// Gets the text.
  const wxString& GetText() const {return m_Text;};
private:
  const wxString m_Info;
  const wxString m_HelpText;
  const bool m_IsBasic;
  const wxString m_Text;
};

/// Offers tool methods and contains the tool info's.
/// A tool with non empty text is used by exMenu::AppendTools.
class exTool
{
public:
  /// Constructor, specify the tool to use.
  exTool(int id);

  /// Adds your own info to the tool.
  /// If you use a %ld in the info string, it is replaced by GetStatistics
  /// with the Actions Completed element.
  static void AddInfo(
    int tool_id,
    const wxString& info,
    const wxString& text = wxEmptyString,
    bool is_basic = true,
    const wxString& helptext = wxEmptyString);

  /// Gets the tool id.
  int GetId() const {return m_Id;};

  /// Gets all the tool info.
  static std::map < int, const exToolInfo > & GetToolInfo() {return m_ToolInfo;};

  /// Gets info about current tool.
  const wxString Info() const;

  // Initializes the tool info map.
  // This is done during exApp::OnInit.
  // Not for doxygen.
  static void Initialize();

  // Type checking.
  /// Is this tool a count type.
  bool IsCountType() const {
    return m_Id == ID_TOOL_REPORT_COUNT;}

  /// Is this tool a find type.
  bool IsFindType() const {
    return m_Id == ID_TOOL_REPORT_FIND || m_Id == ID_TOOL_REPORT_REPLACE;}

  /// Is this tool a header type.
  bool IsHeaderType() const {
    return m_Id == ID_TOOL_HEADER || m_Id == ID_TOOL_REPORT_HEADER;}

  /// Is this tool a report type.
  bool IsReportType() const {
    return m_Id > ID_TOOL_REPORT_FIRST && m_Id < ID_TOOL_REPORT_LAST;}

  /// Is this tool a RCS type.
  bool IsRCSType() const {
    return
      m_Id == ID_TOOL_COMMIT ||
      m_Id == ID_TOOL_REVISION_RECENT ||
      m_Id == ID_TOOL_REPORT_REVISION;}

  /// Is this tool a statistics type.
  bool IsStatisticsType() const {
    return
      m_Id == ID_TOOL_REPORT_COUNT ||
      m_Id == ID_TOOL_REPORT_VERSION  ||
      m_Id == ID_TOOL_REPORT_HEADER ||
      m_Id == ID_TOOL_REPORT_KEYWORD;}
private:
  int m_Id; // cannot be const, as we set tool id using operator= in exTextFile::SetupTool
  static std::map < int, const exToolInfo > m_ToolInfo;
};

#endif
