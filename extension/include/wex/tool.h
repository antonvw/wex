////////////////////////////////////////////////////////////////////////////////
// Name:      tool.h
// Purpose:   Declaration of wex::tool classes
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>
#include <string>
#include <wex/defs.h>

namespace wex
{
  /// This class defines our tool info.
  /// It is stored in the tool info map available from the wex::tool class.
  class tool_info
  {
  public:
    /// Default constructor.
    tool_info(
      const std::string& info = std::string(),
      const std::string& text = std::string(),
      const std::string& helptext = std::string())
      : m_Info(info)
      , m_HelpText(helptext)
      , m_Text(text){};

    /// Returns the helptext.
    const auto & help_text() const {return m_HelpText;};

    /// Returns the info.
    const auto & info() const {return m_Info;};

    /// Returns the text.
    const auto & text() const {return m_Text;};
  private:
    std::string m_HelpText;
    std::string m_Info;
    std::string m_Text;
  };

  template <class T> class statistics;

  /// Offers tool methods and contains the tool info's.
  /// A tool with non empty text is used by menu::append_tools.
  class tool
  {
  public:
    /// Default constructor, specify the toolid to use.
    tool(int id = -1);

    /// Adds your own info to the tool.
    /// If you use a %d in the info string, it is replaced by get_statistics
    /// with the Actions Completed element.
    void add_info(
      int tool_id,
      const std::string& info,
      const std::string& text = std::string(),
      const std::string& helptext = std::string()) {
      m_ToolInfo[tool_id] = tool_info(info, text, helptext);};

    /// Returns the tool id.
    int id() const {return m_Id;};

    /// Returns all the tool info.
    const auto & get_tool_info() const {return m_ToolInfo;};

    /// Returns info about current tool.
    const std::string info() const;

    /// Returns info about current tool using specified statistics.
    const std::string info(const statistics<int>* stat) const;
    
    /// Is this tool a find type.
    bool is_find_type() const {
      return m_Id == ID_TOOL_REPORT_FIND || m_Id == ID_TOOL_REPLACE;}

    /// Is this tool a report type.
    bool is_report_type() const {
      return m_Id > ID_TOOL_REPORT_FIRST && m_Id < ID_TOOL_REPORT_LAST;}
  private:
    const int m_Id;
    static std::map < int, tool_info > m_ToolInfo;
  };
};
