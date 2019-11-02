////////////////////////////////////////////////////////////////////////////////
// Name:      tool.h
// Purpose:   Declaration of wex::tool classes
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
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
    /// Dedualt constructor.
    tool_info(
      const std::string& info = std::string(),
      const std::string& text = std::string(),
      const std::string& helptext = std::string())
      : m_info(info)
      , m_help_text(helptext)
      , m_text(text){};

    /// Returns the helptext.
    const auto & help_text() const {return m_help_text;};

    /// Returns the info.
    const auto & info() const {return m_info;};

    /// Returns the text.
    const auto & text() const {return m_text;};
  private:
    std::string 
      m_help_text,
      m_info,
      m_text;
  };

  template <class T> class statistics;

  /// Offers tool methods and contains the tool info's.
  /// A tool with non empty text is used by menu::append_tools.
  class tool
  {
  public:
    /// Default constructor, specify the toolid to use.
    tool(int id = -1)
       : m_id(id) {;};

    /// Adds your own info to the tool.
    /// If you use a %d in the info string, it is replaced by get_statistics
    /// with the Actions Completed element.
    static void add_info(
      int tool_id,
      const std::string& info,
      const std::string& text = std::string(),
      const std::string& helptext = std::string()) {
      m_tool_info[tool_id] = tool_info(info, text, helptext);};

    /// Returns all the tool info.
    const auto & get_tool_info() const {return m_tool_info;};

    /// Returns the tool id.
    auto id() const {return m_id;};

    /// Returns info about current tool.
    const std::string info() const;

    /// Returns info about current tool using specified statistics.
    const std::string info(const statistics<int>* stat) const;
    
    /// Is this tool a find type.
    bool is_find_type() const {
      return m_id == ID_TOOL_REPORT_FIND || m_id == ID_TOOL_REPLACE;}

    /// Is this tool a report type.
    bool is_report_type() const {
      return m_id > ID_TOOL_REPORT_FIRST && m_id < ID_TOOL_REPORT_LAST;}
  private:
    const int m_id;
    static std::map < int, tool_info > m_tool_info;
  };
};
