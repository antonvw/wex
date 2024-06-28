////////////////////////////////////////////////////////////////////////////////
// Name:      listview.cpp
// Purpose:   Implementation of wex::listview and related classes
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>
#include <boost/tokenizer.hpp>
#include <wex/core/chrono.h>
#include <wex/core/config.h>
#include <wex/core/interruptible.h>
#include <wex/core/log.h>
#include <wex/core/regex.h>
#include <wex/core/tokenize.h>
#include <wex/data/find.h>
#include <wex/data/stc.h>
#include <wex/factory/bind.h>
#include <wex/factory/defs.h>
#include <wex/factory/util.h>
#include <wex/syntax/lexers.h>
#include <wex/syntax/printing.h>
#include <wex/syntax/stc.h>
#include <wex/ui/frame.h>
#include <wex/ui/frd.h>
#include <wex/ui/item-dialog.h>
#include <wex/ui/item-vector.h>
#include <wex/ui/listitem.h>
#include <wex/ui/listview.h>
#include <wex/ui/menu.h>
#include <wx/dnd.h>
#include <wx/generic/dirctrlg.h> // for wxTheFileIconsTable
#include <wx/imaglist.h>
#include <wx/numdlg.h> // for wxGetNumberFromUser
#include <wx/settings.h>

#include <algorithm>
#include <cctype>

namespace wex
{
// file_droptarget is already used
class droptarget : public wxFileDropTarget
{
public:
  explicit droptarget(listview* lv)
    : m_listview(lv)
  {
    ;
  }
  bool
  OnDropFiles(wxCoord x, wxCoord y, const wxArrayString& filenames) override
  {
    // Only drop text if nothing is selected,
    // so dropping on your own selection is impossible.
    if (m_listview->GetSelectedItemCount() == 0)
    {
      for (size_t i = 0; i < filenames.GetCount(); i++)
      {
        m_listview->item_from_text(filenames[i]);
      }

      return true;
    }

    return false;
  };

private:
  listview* m_listview;
};

template <typename T> int compare(T x, T y)
{
  if (x > y)
  {
    return 1;
  }

  if (x < y)
  {
    return -1;
  }

  return 0;
}

const std::vector<item> config_items()
{
  return std::vector<item>(
    {{"notebook",
      {{_("General"),
        {{_("list.Header"), item::CHECKBOX, std::any(true)},
         {_("list.Single selection"), item::CHECKBOX},
         {_("list.Comparator"), item::FILEPICKERCTRL},
         {_("list.Sort method"),
          {{SORT_TOGGLE, _("Sort toggle")},
           {SORT_KEEP, _("Sort keep order")},
           {SORT_ASCENDING, _("Sort ascending")},
           {SORT_DESCENDING, _("Sort descending")}}},
         {_("list.Context size"), 0, 80, 10},
         {_("list.Rulers"),
          {{wxLC_HRULES, _("Horizontal rulers")},
           {wxLC_VRULES, _("Vertical rulers")}},
          false}}},
       {_("Font"),
        {{_("list.Font"),
          item::FONTPICKERCTRL,
          wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT)}}},
       {_("Colour"),
        {{_("list.Readonly colour"),
          item::COLOURPICKERWIDGET,
          *wxLIGHT_GREY}}}}}});
}
}; // namespace wex

wex::listview::listview(const data::listview& data)
  : factory::listview(data.window(), data.control())
  , m_col_event_id(1000)
  , m_data(data::listview(data)
             .image(
               data.type() == data::listview::NONE ||
                   data.type() == data::listview::TSV ?
                 data.image() :
                 data::listview::IMAGE_FILE_ICON)
             .set_listview(this))
  , m_frame(dynamic_cast<wex::frame*>(wxTheApp->GetTopWindow()))
{
  config_get();

  m_data.inject();

  // We can only have one drop target, we use file drop target,
  // as list items can also be copied and pasted.
  SetDropTarget(new droptarget(this));

  if (m_data.image() != data::listview::IMAGE_NONE)
  {
    SetImageList(wxTheFileIconsTable->GetSmallImageList(), wxIMAGE_LIST_SMALL);
  }

  m_frame->update_statusbar(this);

  if (
    m_data.type() != data::listview::NONE &&
    m_data.type() != data::listview::TSV)
  {
    Bind(
      wxEVT_IDLE,
      [=, this](wxIdleEvent& event)
      {
        process_idle(event);
      });
  }

  Bind(
    wxEVT_LIST_BEGIN_DRAG,
    [=, this](wxListEvent& event)
    {
      process_list(event, wxEVT_LIST_BEGIN_DRAG);
    });

  Bind(
    wxEVT_LIST_ITEM_ACTIVATED,
    [=, this](wxListEvent& event)
    {
      item_activated(event.GetIndex());
    });

  Bind(
    wxEVT_LIST_ITEM_DESELECTED,
    [=, this](const wxListEvent& event)
    {
      m_frame->update_statusbar(this);
    });

  Bind(
    wxEVT_LIST_ITEM_SELECTED,
    [=, this](const wxListEvent& event)
    {
      process_list(event, wxEVT_LIST_ITEM_SELECTED);
    });

  Bind(
    wxEVT_LIST_COL_CLICK,
    [=, this](const wxListEvent& event)
    {
      sort_column(
        event.GetColumn(),
        (sort_t)config(_("list.Sort method")).get(SORT_TOGGLE));
    });

  Bind(
    wxEVT_LIST_COL_RIGHT_CLICK,
    [=, this](const wxListEvent& event)
    {
      m_to_be_sorted_column_no = event.GetColumn();

      menu menu(
        GetSelectedItemCount() > 0 ? menu::IS_SELECTED : menu::menu_t_def());

      menu.append({{wxID_SORT_ASCENDING}, {wxID_SORT_DESCENDING}});

      PopupMenu(&menu);
    });

  Bind(
    wxEVT_RIGHT_DOWN,
    [=, this](wxMouseEvent& event)
    {
      process_mouse(event);
    });

  bind_set_focus(this);

  Bind(
    wxEVT_SHOW,
    [=, this](wxShowEvent& event)
    {
      event.Skip();
      m_frame->update_statusbar(this);
    });

  bind_other();
}

bool wex::listview::append_columns(const std::vector<column>& cols)
{
  SetSingleStyle(wxLC_REPORT);

  for (const auto& col : cols)
  {
    auto mycol(col);

    if (const auto index =
          AppendColumn(mycol.GetText(), mycol.GetAlign(), mycol.GetWidth());
        index == -1)
    {
      return false;
    }

    mycol.SetColumn(GetColumnCount() - 1);
    m_columns.emplace_back(mycol);

    Bind(
      wxEVT_MENU,
      [=, this](wxCommandEvent& event)
      {
        sort_column(event.GetId() - m_col_event_id, SORT_TOGGLE);
      },
      m_col_event_id + GetColumnCount() - 1);
  }

  return true;
}

void wex::listview::bind_other()
{
  bind(this).frd(
    find_replace_data::get()->wx(),
    [=, this](const std::string& s, bool b)
    {
      find_next(s, b);
    });

  bind(this).command(
    {{[=, this](const wxCommandEvent& event)
      {
        clear();
      },
      wxID_CLEAR},
     {[=, this](const wxCommandEvent& event)
      {
        copy_selection_to_clipboard();
      },
      wxID_COPY},
     {[=, this](const wxCommandEvent& event)
      {
        edit_delete();
      },
      wxID_DELETE},
     {[=, this](const wxCommandEvent& event)
      {
        item_from_text(clipboard_get());
      },
      wxID_PASTE},
     {[=, this](const wxCommandEvent& event)
      {
        SetItemState(-1, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
      },
      wxID_SELECTALL},
     {[=, this](const wxCommandEvent& event)
      {
        sort_column(m_to_be_sorted_column_no, SORT_ASCENDING);
      },
      wxID_SORT_ASCENDING},
     {[=, this](const wxCommandEvent& event)
      {
        sort_column(m_to_be_sorted_column_no, SORT_DESCENDING);
      },
      wxID_SORT_DESCENDING},
     {[=, this](const wxCommandEvent& event)
      {
        copy_selection_to_clipboard();
        edit_delete();
      },
      wxID_CUT},
     {[=, this](const wxCommandEvent& event)
      {
        for (auto i = 0; i < GetItemCount(); i++)
        {
          Select(i, !IsSelected(i));
        }
      },
      ID_EDIT_SELECT_INVERT},
     {[=, this](const wxCommandEvent& event)
      {
        for (auto i = 0; i < GetItemCount(); i++)
        {
          Select(i, false);
        }
      },
      ID_EDIT_SELECT_NONE},
     {[=, this](const wxCommandEvent& event)
      {
        if (on_command(event))
        {
          m_frame->update_statusbar(this);
        }
      },
      wxID_ADD},

     {[=, this](const wxCommandEvent& event)
      {
        process_match(event);
      },
      ID_LIST_MATCH},

     {[=, this](const wxCommandEvent& event)
      {
        for (auto i = GetFirstSelected(); i != -1; i = GetNextSelected(i))
        {
          item_activated(i);
        }
      },
      ID_EDIT_OPEN},
     {[=, this](const wxCommandEvent& event)
      {
        if (!IsShown() || GetItemCount() == 0)
        {
          return;
        }
        if (const auto val(wxGetNumberFromUser(
              _("Input") + " (1 - " + std::to_string(GetItemCount()) + "):",
              wxEmptyString,
              _("Enter Item Number"),
              (GetFirstSelected() == -1 ? 1 : GetFirstSelected() + 1),
              1,
              GetItemCount()));
            val > 0)
        {
          data::listview(data::control().line(val)).set_listview(this).inject();
        }
      },
      wxID_JUMP_TO}});
}

const std::string wex::listview::build_page()
{
  std::stringstream text;

  text << "<TABLE "
       << (((GetWindowStyle() & wxLC_HRULES) ||
            (GetWindowStyle() & wxLC_VRULES)) ?
             "border=1" :
             "border=0")
       << " cellpadding=4 cellspacing=0 >\n"
       << "<tr>\n";

  for (auto c = 0; c < GetColumnCount(); c++)
  {
    wxListItem col;
    GetColumn(c, col);
    text << "<td><i>" << col.GetText() << "</i>\n";
  }

  for (auto i = 0; i < GetItemCount(); i++)
  {
    text << "<tr>\n";

    for (auto col = 0; col < GetColumnCount(); col++)
    {
      text << "<td>" << GetItemText(i, col) << "\n";
    }
  }

  text << "</TABLE>\n";

  return text.str();
}

void wex::listview::build_popup_menu(wex::menu& menu)
{
  if (
    GetSelectedItemCount() >= 1 &&
    listitem(this, GetFirstSelected()).path().stat().is_ok())
  {
    menu.append(
      {{ID_EDIT_OPEN, _("&Open"), data::menu().art(wxART_FILE_OPEN)}, {}});
  }
  else if (GetSelectedItemCount() >= 1 && m_data.type() == data::listview::TSV)
  {
    menu.append({{ID_EDIT_OPEN, _("&Open")}});
  }

  menu.append({{}, {menu_item::EDIT_INVERT}});

  if (GetItemCount() > 0 && GetSelectedItemCount() == 0 && InReportView())
  {
    menu.append({{}});

    auto* menuSort = new wex::menu;

    for (const auto& it : m_columns)
    {
      menuSort->append({{m_col_event_id + it.GetColumn(), it.GetText()}});
    }

    menu.append({{menuSort, _("Sort On")}});
  }

  if (
    (m_data.type() == data::listview::FOLDER && GetSelectedItemCount() <= 1) ||
    m_data.type() == data::listview::TSV)
  {
    menu.append({{}, {wxID_ADD}});
  }
}

void wex::listview::clear()
{
  if (GetItemCount() == 0)
  {
    return;
  }

  DeleteAllItems();

  sort_column_reset();

  m_frame->update_statusbar(this);
}

int wex::listview::config_dialog(const data::window& par)
{
  const data::window data(data::window(par).title(_("List Options")));

  if (m_config_dialog == nullptr)
  {
    m_config_dialog = new item_dialog(config_items(), data);
  }

  return (data.button() & wxAPPLY) ? m_config_dialog->Show() :
                                     m_config_dialog->ShowModal();
}

void wex::listview::config_get()
{
  const auto&       ci(config_items());
  const item_vector iv(&ci);

  lexers::get()->apply_default_style(
    [=, this](const std::string& back)
    {
      SetBackgroundColour(wxColour(back));
    });

  SetFont(iv.find<wxFont>(_("list.Font")));
  SetSingleStyle(
    wxLC_HRULES,
    (iv.find<long>(_("list.Rulers")) & wxLC_HRULES) > 0);
  SetSingleStyle(
    wxLC_VRULES,
    (iv.find<long>(_("list.Rulers")) & wxLC_VRULES) > 0);
  SetSingleStyle(
    wxLC_NO_HEADER,
    !iv.find<bool>(_("list.Header")) || m_data.type() == data::listview::TSV);
  SetSingleStyle(wxLC_SINGLE_SEL, iv.find<bool>(_("list.Single selection")));

  items_update();
}

const std::string wex::listview::context(const std::string& line, int pos) const
{
  int context_size = config(_("list.Context size")).get(10);

  if (pos == -1 || context_size <= 0)
  {
    return line;
  }

  return (context_size > pos ? std::string(context_size - pos, ' ') :
                               std::string()) +
         line.substr(context_size < pos ? pos - context_size : 0);
}

void wex::listview::copy_selection_to_clipboard()
{
  if (GetSelectedItemCount() == 0)
  {
    return;
  }

  wxBusyCursor wait;
  std::string  clipboard;

  for (auto i = GetFirstSelected(); i != -1; i = GetNextSelected(i))
  {
    clipboard += item_to_text(i) + "\n";
  }

  clipboard_add(clipboard);
}

void wex::listview::edit_delete()
{
  if (GetSelectedItemCount() == 0)
  {
    return;
  }

  long old_item = -1;

  for (long i = -1; (i = GetNextSelected(i)) != -1;)
  {
    DeleteItem(i);
    old_item = i;
    i        = -1;
  }

  if (old_item != -1 && old_item < GetItemCount())
  {
    SetItemState(old_item, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
  }

  items_update();
}

bool wex::listview::find_next(const std::string& text, bool forward)
{
  if (text.empty())
  {
    return false;
  }

  const auto  firstselected = GetFirstSelected();
  static long start_item;
  static long end_item;

  data::find find(text, forward);

  if (forward)
  {
    start_item =
      find.recursive() ? 0 : (firstselected != -1 ? firstselected + 1 : 0);
    end_item = GetItemCount();
  }
  else
  {
    start_item = find.recursive() ?
                   GetItemCount() - 1 :
                   (firstselected != -1 ? firstselected - 1 : 0);
    end_item   = -1;
  }

  int match = -1;

  for (int index = start_item; index != end_item && match == -1;
       (forward ? index++ : index--))
  {
    for (int col = 0; col < GetColumnCount() && match == -1; col++)
    {
      if (find_replace_data::get()->match(GetItemText(index, col), find))
      {
        match = index;
      }
    }
  }

  if (match != -1)
  {
    Select(match);
    EnsureVisible(match);

    if (firstselected != -1 && match != firstselected)
    {
      Select(firstselected, false);
    }

    return true;
  }

  find.statustext();

  if (!find.recursive())
  {
    data::find::recursive(true);
    find_next(text, forward);
    data::find::recursive(false);
  }

  return false;
}

unsigned int wex::listview::get_art_id(const wxArtID& artid)
{
  if (const auto& it = m_art_ids.find(artid); it != m_art_ids.end())
  {
    return it->second;
  }

  auto* il = GetImageList(wxIMAGE_LIST_SMALL);
  if (il == nullptr)
  {
    assert(0);
    return 0;
  }

  m_art_ids.insert({artid, il->GetImageCount()});
  return il->Add(wxArtProvider::GetBitmap(artid, wxART_OTHER));
}

wex::column wex::listview::get_column(const std::string& name) const
{
  if (const auto& it = std::find_if(
        m_columns.begin(),
        m_columns.end(),
        [name](auto const& it)
        {
          return it.GetText() == name;
        });
      it != m_columns.end())
  {
    return *it;
  }

  return column();
}

const std::string wex::listview::get_item_text(
  long               item_number,
  const std::string& col_name) const
{
  if (item_number < 0 || item_number >= GetItemCount())
  {
    return std::string();
  }

  if (col_name.empty())
  {
    return GetItemText(item_number);
  }

  const int col = find_column(col_name);
  return col < 0 ? std::string() : GetItemText(item_number, col).ToStdString();
}

bool wex::listview::insert_item(
  const std::vector<std::string>& item,
  long                            requested_index)
{
  if (item.empty() || item.front().empty() || item.size() > m_columns.size())
  {
    return false;
  }

  long index = 0;

  for (int no = 0; const auto& col : item)
  {
    try
    {
      if (!col.empty())
      {
        switch (m_columns[no].type())
        {
          case column::DATE:
            if (const auto& r(chrono().get_time(col)); !r)
            {
              return false;
            }
            break;

          case column::FLOAT:
            (void)std::stof(col);
            break;

          case column::INT:
            (void)std::stoi(col);
            break;

          case column::STRING:
            break;

          default:
            break;
        }

        if (no == 0)
        {
          if (
            (index = InsertItem(
               requested_index == -1 ? GetItemCount() : requested_index,
               col)) == -1)
          {
            log("listview insert") << "index:" << index << "col:" << col;
            return false;
          }
          if (regex v(",fore:(.*)");
              v.match(lexers::get()->get_default_style().value()) > 0)
          {
            SetItemTextColour(index, wxColour(v[0]));
          }
        }
        else
        {
          if (!set_item(index, no, col))
          {
            log("listview set_item") << "index:" << index << "col:" << col;
            return false;
          }
        }
      }

      no++;
    }
    catch (std::exception& e)
    {
      log(e) << "insert_item exception:" << col;
      return false;
    }
  }

  return true;
}

void wex::listview::item_activated(long item_number)
{
  assert(item_number >= 0);

  switch (m_data.type())
  {
    case data::listview::FOLDER:
    {
      wxDirDialog dir_dlg(
        this,
        _(wxDirSelectorPromptStr),
        GetItemText(item_number),
        wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);

      if (dir_dlg.ShowModal() == wxID_OK)
      {
        SetItemText(item_number, dir_dlg.GetPath());
        listitem(this, item_number).update();
      }
    }
    break;

    case data::listview::TSV:
    {
      m_frame->stc_entry_dialog_title(_("Item"));
      m_frame->stc_entry_dialog_component()->set_text(
        item_to_text(item_number));

      if (m_frame->stc_entry_dialog_show(true) == wxID_OK)
      {
        int col = 0;

        boost::tokenizer<boost::char_separator<char>> tok(
          m_frame->stc_entry_dialog_component()->get_text(),
          boost::char_separator<char>(
            std::string(1, m_field_separator).c_str()));

        for (auto it = tok.begin(); it != tok.end() && col < GetColumnCount();
             ++it)
        {
          set_item(item_number, col++, *it);
        }
      }
    }
    break;

    default:
      // Cannot be const because of SetItem later on.
      if (listitem item(this, item_number); item.path().file_exists())
      {
        const auto    no(get_item_text(item_number, _("Line No")));
        data::control data(
          (m_data.type() == data::listview::FIND && !no.empty() ?
             data::control()
               .line(std::stoi(no))
               .find(get_item_text(item_number, _("Match"))) :
             data::control()));

        m_frame->open_file(item.path(), data);
      }
      else if (item.path().dir_exists())
      {
        m_frame->stc_entry_dialog_title(_("Folder Type"));
        m_frame->stc_entry_dialog_component()->set_text(
          get_item_text(item_number, _("Type")));

        if (m_frame->stc_entry_dialog_show(true) == wxID_OK)
        {
          item.set_item(
            _("Type"),
            m_frame->stc_entry_dialog_component()->get_text());
        }
      }
  }
}

bool wex::listview::item_from_text(const std::string& text)
{
  if (text.empty())
  {
    return false;
  }

  bool modified = false;

  for (const auto& it : boost::tokenizer<boost::char_separator<char>>(
         text,
         boost::char_separator<char>("\n")))
  {
    switch (m_data.type())
    {
      case data::listview::NONE:
      case data::listview::TSV:
        if (insert_item(tokenize<std::vector<std::string>>(
              it,
              std::string(1, m_field_separator).c_str())))
        {
          modified = true;
        }
        break;

      default:
        modified = true;

        if (!InReportView())
        {
          listitem(this, path(it)).insert();
        }
        else if (!report_view(it))
        {
          return false;
        }
    }
  }

  m_frame->update_statusbar(this);

  return modified;
}

const std::string wex::listview::item_to_text(long item_number) const
{
  std::string text;

  if (item_number == -1)
  {
    for (auto i = 0; i < GetItemCount(); i++)
    {
      text += GetItemText(i) + "\n";
    }

    return text;
  }

  if (item_number >= GetItemCount())
  {
    return text;
  }

  switch (m_data.type())
  {
    case data::listview::FILE:
    case data::listview::HISTORY:
    {
      const listitem item(const_cast<listview*>(this), item_number);
      text =
        (item.path().stat().is_ok() ? item.path().string() :
                                      item.path().filename());

      if (item.path().dir_exists() && !item.path().file_exists())
      {
        text += field_separator() + get_item_text(item_number, _("Type"));
      }
    }
    break;

    case data::listview::FOLDER:
      return GetItemText(item_number);

    default:
      for (int col = 0; col < GetColumnCount(); col++)
      {
        text += GetItemText(item_number, col);

        if (col < GetColumnCount() - 1)
        {
          text += m_field_separator;
        }
      }
  }

  return text;
}

void wex::listview::items_update()
{
  if (
    m_data.type() != data::listview::NONE &&
    m_data.type() != data::listview::TSV)
  {
    for (auto i = 0; i < GetItemCount(); i++)
    {
      listitem(this, i).update();
    }
  }
}

bool wex::listview::load(const strings_t& l)
{
  clear();

  if (l.empty())
  {
    return true;
  }

  if (m_data.type() == data::listview::TSV && GetColumnCount() == 0)
  {
    // Use front item to set up the columns, so we assume each
    // item in the vector has the same columns.
    boost::tokenizer<boost::char_separator<char>> tok(
      l.front(),
      boost::char_separator<char>("\t"));

    int i = 0;

    std::for_each(
      tok.begin(),
      tok.end(),
      [this, &i](const auto&)
      {
        append_columns({{std::to_string(i++ + 1), column::STRING, 50}});
      });
  }

  for (const auto& it : l)
  {
    item_from_text(it);
  }

  return true;
}

bool wex::listview::on_command(const wxCommandEvent& event)
{
  switch (const long new_index =
            GetSelectedItemCount() > 0 ? GetFirstSelected() : 0;
          data().type())
  {
    case data::listview::TSV:
      m_frame->stc_entry_dialog_title(_("Item"));
      m_frame->stc_entry_dialog_component()->set_text(item_to_text(new_index));

      if (m_frame->stc_entry_dialog_show(true) == wxID_OK)
      {
        return insert_item(
          tokenize<std::vector<std::string>>(
            m_frame->stc_entry_dialog_component()->get_text(),
            std::string(1, field_separator()).c_str()),
          new_index);
      }
      break;

    default:
    {
      std::string defaultPath;

      if (GetSelectedItemCount() > 0)
      {
        defaultPath = listitem(this, GetFirstSelected()).path().string();
      }

      wxDirDialog dir_dlg(
        this,
        _(wxDirSelectorPromptStr),
        defaultPath,
        wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);

      if (dir_dlg.ShowModal() == wxID_OK)
      {
        const auto no =
          (GetSelectedItemCount() > 0 ? GetFirstSelected() : GetItemCount());

        listitem(this, path(dir_dlg.GetPath().ToStdString())).insert(no);
        return true;
      }
    }
  }

  return false;
}

void wex::listview::print()
{
  wxBusyCursor wait;
  printing::get()->get_html_printer()->PrintText(build_page());
}

void wex::listview::print_preview()
{
  wxBusyCursor wait;
  printing::get()->get_html_printer()->PreviewText(build_page());
}

void wex::listview::process_idle(wxIdleEvent& event)
{
  event.Skip();

  if (
    !IsShown() || interruptible::is_running() || GetItemCount() == 0 ||
    !config("AllowSync").get(true))
  {
    return;
  }
  if (m_item_number < GetItemCount())
  {
    if (listitem item(this, m_item_number);
        item.path().file_exists() &&
        (item.path().stat().get_modification_time_str() !=
           get_item_text(m_item_number, _("Modified")) ||
         item.path().stat().is_readonly() != item.is_readonly()))
    {
      item.update();
      log::status() << item.path();
      m_item_updated = true;
    }

    m_item_number++;
  }
  else
  {
    m_item_number = 0;

    if (m_item_updated)
    {
      if (m_data.type() == data::listview::FILE)
      {
        if (
          config("list.SortSync").get(true) &&
          sorted_column_no() == find_column(_("Modified")))
        {
          sort_column(_("Modified"), SORT_KEEP);
        }
      }

      m_item_updated = false;
    }
  }
}

void wex::listview::process_list(const wxListEvent& event, wxEventType type)
{
  if (type == wxEVT_LIST_ITEM_SELECTED)
  {
    if (m_data.type() != data::listview::NONE && GetSelectedItemCount() == 1)
    {
      if (const wex::path fn(listitem(this, event.GetIndex()).path());
          fn.stat().is_ok())
      {
        log::status() << fn;
      }
      else
      {
        log::status(get_item_text(GetFirstSelected()));
      }
    }
  }
  else if (type == wxEVT_LIST_BEGIN_DRAG)
  {
    // Start drag operation.
    std::string text;
    for (auto i = GetFirstSelected(); i != -1; i = GetNextSelected(i))
    {
      text += item_to_text(i) + "\n";
    }

    if (!text.empty())
    {
      wxTextDataObject textData(text);
      wxDropSource     source(textData, this);
      source.DoDragDrop(wxDragCopy);
    }
  }

  m_frame->update_statusbar(this);
}

void wex::listview::process_match(const wxCommandEvent& event)
{
  const auto* m = static_cast<path_match*>(event.GetClientData());
  listitem    item(this, m->path());

  item.insert();
  item.set_item(_("Line No"), std::to_string(m->line_no() + 1));
  item.set_item(_("Line"), context(m->line(), m->pos()));
  item.set_item(_("Match"), find_replace_data::get()->get_find_string());

  delete m;
}

void wex::listview::process_mouse(const wxMouseEvent& event)
{
  menu::menu_t style(menu::menu_t().set(menu::IS_POPUP).set(menu::IS_VISUAL));

  if (GetSelectedItemCount() > 0)
  {
    style.set(menu::IS_SELECTED);
  }
  if (GetItemCount() == 0)
  {
    style.set(menu::IS_EMPTY);
  }
  if (
    m_data.type() != data::listview::FIND &&
    m_data.type() != data::listview::FILE)
  {
    style.set(menu::CAN_PASTE);
  }

  if (GetSelectedItemCount() == 0 && GetItemCount() > 0)
  {
    style.set(menu::ALLOW_CLEAR);
  }

  wex::menu menu(style);

  build_popup_menu(menu);

  if (menu.GetMenuItemCount() > 0)
  {
    PopupMenu(&menu);
  }
}

bool wex::listview::report_view(const std::string& text)
{
  boost::tokenizer<boost::char_separator<char>> tok(
    text,
    boost::char_separator<char>(std::string(1, m_field_separator).c_str()));

  if (auto tt = tok.begin(); tt != tok.end())
  {
    if (path fn(*tt); fn.file_exists())
    {
      listitem item(this, fn);
      item.insert();

      // And try to set the rest of the columns
      // (that are not already set by inserting).
      int col = 1;
      while (++tt != tok.end() && col < GetColumnCount() - 1)
      {
        if (
          col != find_column(_("Type")) && col != find_column(_("In Folder")) &&
          col != find_column(_("Size")) && col != find_column(_("Modified")))
        {
          if (!set_item(item.GetId(), col, *tt))
          {
            return false;
          }
        }

        col++;
      }
    }
    else
    {
      // Now we need only the first column (containing findfiles). If
      // more columns are present, these are ignored.
      const auto& findfiles =
        (std::next(tt) != tok.end() ? *(std::next(tt)) : text);

      listitem(this, path(*tt), findfiles).insert();
    }
  }
  else
  {
    listitem(this, path(text)).insert();
  }

  return true;
}

const wex::strings_t wex::listview::save() const
{
  strings_t l;

  for (auto i = 0; i < GetItemCount(); i++)
  {
    l.emplace_back(item_to_text(i));
  }

  return l;
}

std::vector<std::string>* pitems;

int wxCALLBACK compare_cb(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortData)
{
  const bool  ascending = (sortData > 0);
  const auto& str1      = (*pitems)[item1];
  const auto& str2      = (*pitems)[item2];

  // should return 0 if the items are equal,
  // negative value if the first item is less than the second one
  // and positive value if the first one is greater than the second one

  if (str1.empty() && str2.empty())
  {
    return 0;
  }
  if (str1.empty())
  {
    return -1;
  }
  if (str2.empty())
  {
    return 1;
  }

  switch (const auto type = (wex::column::type_t)std::abs(sortData); type)
  {
    case wex::column::DATE:
    {
      const auto& t1(wex::chrono().get_time(str1));
      const auto& t2(wex::chrono().get_time(str2));
      if (!t1 || !t2)
      {
        return 0;
      }

      return ascending ? wex::compare((unsigned long)*t1, (unsigned long)*t2) :
                         wex::compare((unsigned long)*t2, (unsigned long)*t1);
    }

    case wex::column::FLOAT:
      return ascending ? wex::compare(std::stof(str1), std::stof(str2)) :
                         wex::compare(std::stof(str2), std::stof(str1));

    case wex::column::INT:
      return ascending ? wex::compare(std::stoi(str1), std::stoi(str2)) :
                         wex::compare(std::stoi(str2), std::stoi(str1));

    case wex::column::STRING:
      if (!wex::find_replace_data::get()->match_case())
      {
        return ascending ? wex::icompare(str1, str2) :
                           wex::icompare(str2, str1);
      }
      else
      {
        return ascending ? str1.compare(str2) : str2.compare(str1);
      }

    default:
      assert(0);
  }

  return 0;
}

bool wex::listview::set_item(long index, int column, const std::string& text)
{
  if (text.empty())
  {
    return true;
  }

  try
  {
    switch (m_columns[column].type())
    {
      case column::DATE:
        if (const auto& r(chrono().get_time(text)); !r)
        {
          return false;
        }
        break;

      case column::FLOAT:
        (void)std::stof(text);
        break;

      case column::INT:
        (void)std::stoi(text);
        break;

      case column::STRING:
        break;

      default:
        break;
    }

    return SetItem(index, column, text);
  }
  catch (std::exception& e)
  {
    log(e) << "index:" << index << "col:" << column << ":" << text;
    return false;
  }
}

bool wex::listview::set_item_image(long item_number, const wxArtID& artid)
{
  if (item_number < 0 || item_number >= GetItemCount())
  {
    return false;
  }

  return (
    m_data.image() == data::listview::IMAGE_ART ?
      SetItemImage(item_number, get_art_id(artid)) :
      false);
}

bool wex::listview::sort_column(int column_no, sort_t sort_method)
{
  if (column_no == -1 || column_no >= static_cast<int>(m_columns.size()))
  {
    return false;
  }

  wxBusyCursor wait;

  sort_column_reset();

  auto& sorted_col = m_columns[column_no];

  sorted_col.set_is_sorted_ascending(sort_method);

  std::vector<std::string> items;
  pitems = &items;

  for (int i = 0; i < GetItemCount(); i++)
  {
    items.emplace_back(GetItemText(i, column_no));
    SetItemData(i, i);
  }

  try
  {
    const auto sortdata =
      (sorted_col.is_sorted_ascending() ? sorted_col.type() :
                                          (0 - sorted_col.type()));

    SortItems(compare_cb, sortdata);
    ShowSortIndicator(column_no, sorted_col.is_sorted_ascending());

    m_sorted_column_no = column_no;

    if (GetItemCount() > 0)
    {
      items_update();
      after_sorting();
    }

    log::status(_("Sorted on")) << sorted_col.GetText().ToStdString();
    return true;
  }
  catch (std::exception& e)
  {
    log(e) << "sort:" << sorted_col.GetText().ToStdString();
    return false;
  }
}

void wex::listview::sort_column_reset()
{
  if (m_sorted_column_no != -1)
  {
    RemoveSortIndicator();
    m_sorted_column_no = -1;
  }
}
