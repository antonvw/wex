////////////////////////////////////////////////////////////////////////////////
// Name:      find-in-files.cpp
// Purpose:   Implementation of wex::del::frame class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <thread>

#include <wex/common/wex.h>
#include <wex/core/wex.h>
#include <wex/del/wex.h>
#include <wex/stc/wex.h>
#include <wex/ui/wex.h>

void wex::del::frame::find_in_files(window_id dialogid)
{
  const bool      replace = (dialogid == id_replace_in_files);
  const wex::tool tool(replace ? ID_TOOL_REPLACE : ID_TOOL_REPORT_FIND);

  log::status(find_replace_string(replace));

  data::dir::type_t type;
  type.set(data::dir::FILES);

  if (config(m_text_recursive).get(true))
  {
    type.set(data::dir::RECURSIVE);
  }

  if (config(m_text_hidden).get(false))
  {
    type.set(data::dir::HIDDEN);
  }

  find_replace_data::get()->set_regex(
    config(find_replace_data::get()->text_regex()).get(true));

  wex::dir dir(
    path(config(m_text_in_folder).get_first_of()),
    data::dir()
      .find_replace_data(find_replace_data::get())
      .file_spec(config(m_text_in_files).get_first_of())
      .type(type),
    activate_and_clear(tool));

  dir.find_files(tool);
}

bool wex::del::frame::find_in_files(
  const std::vector<path>& files,
  const tool&              tool,
  bool                     show_dialog,
  listview*                report)
{
  if (files.empty())
  {
    return false;
  }

  if (const auto& filename(files[0]);
      show_dialog &&
      find_in_files_dialog(
        tool,
        filename.dir_exists() && !filename.file_exists()) == wxID_CANCEL)
  {
    return false;
  }

#ifdef __WXMSW__
  std::thread t(
    [=, this]
    {
#endif
      statistics<int> stats;

      for (const auto& it : files)
      {
        if (it.file_exists())
        {
          if (wex::stream file(find_replace_data::get(), it, tool, report);
              file.run_tool())
          {
            stats += file.get_statistics().get_elements();
          }
        }
        else if (it.dir_exists())
        {
          wex::dir dir(
            it,
            data::dir().file_spec(config(m_text_in_files).get_first_of()));

          dir.find_files(tool);
          stats += dir.get_statistics().get_elements();
        }
      }

      log::status(tool.info(&stats));

#ifdef __WXMSW__
    });
  t.detach();
#endif

  return true;
}

int wex::del::frame::find_in_files_dialog(const tool& tool, bool add_in_files)
{
  if (get_stc() != nullptr)
  {
    get_stc()->get_find_string();
  }

  if (
    item_dialog(
      {{find_replace_data::get()->text_find(),
        item::COMBOBOX,
        std::any(),
        data::control().is_required(true)},
       (add_in_files ? item(
                         m_text_in_files,
                         item::COMBOBOX,
                         std::any(),
                         data::control().is_required(true)) :
                       item()),
       (tool.id() == ID_TOOL_REPLACE ?
          item(find_replace_data::get()->text_replace_with(), item::COMBOBOX) :
          item()),
       item(m_info)},
      data::window().title(find_in_files_title(tool.id())))
      .ShowModal() == wxID_CANCEL)
  {
    return wxID_CANCEL;
  }

  log::status(find_replace_string(tool.id() == ID_TOOL_REPLACE));

  return wxID_OK;
}

const std::string wex::del::frame::find_in_files_title(window_id id) const
{
  return (
    id == ID_TOOL_REPLACE ? _("Replace In Selection") : _("Find In Selection"));
}
