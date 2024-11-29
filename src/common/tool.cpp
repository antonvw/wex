////////////////////////////////////////////////////////////////////////////////
// Name:      tool.cpp
// Purpose:   Implementation of wex::tool class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2008-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>
#include <wex/common/statistics.h>
#include <wex/common/tool.h>
#include <wex/core/log.h>
#include <wx/translation.h>

wex::tool::tool_info_t wex::tool::m_tool_info{
  {ID_TOOL_ADD, {_("Added")}},
  {ID_TOOL_REPORT_FIND, {_("Found %d matches in")}},
  {ID_TOOL_REPLACE, {_("Replaced %d matches in")}}};

const std::string wex::tool::info() const
{
  if (const auto& it = m_tool_info.find(m_id); it != m_tool_info.end())
  {
    return it->second.info();
  }

  log("tool::info") << (int)m_id;
  return std::string();
}

const std::string wex::tool::info(const wex::statistics<int>* stat) const
{
  std::stringstream ss;

  ss << boost::algorithm::replace_all_copy(
          info(),
          "%d",
          std::to_string(stat->get(_("Actions Completed"))))
     << " " << std::to_string(stat->get(_("Files"))) << " " << _("file(s)");

  if (const auto folders(stat->get(_("Folders"))); folders > 0)
  {
    ss << " " << _("and") << " " << std::to_string(folders) << " "
       << _("folders(s)");
  }

  return ss.str();
}
