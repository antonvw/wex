////////////////////////////////////////////////////////////////////////////////
// Name:      listview.cpp
// Purpose:   Implementation of wex::listview and related classes
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <cctype>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/dnd.h> 
#include <wx/fdrepdlg.h> // for wxFindDialogEvent
#include <wx/numdlg.h> // for wxGetNumberFromUser
#include <wx/generic/dirctrlg.h> // for wxTheFileIconsTable
#include <wx/imaglist.h>
#include <wex/listview.h>
#include <wex/config.h>
#include <wex/defs.h>
#include <wex/frame.h>
#include <wex/frd.h>
#include <wex/interruptable.h>
#include <wex/item.h>
#include <wex/item-vector.h>
#include <wex/itemdlg.h>
#include <wex/lexer.h>
#include <wex/lexers.h>
#include <wex/log.h>
#include <wex/listitem.h>
#include <wex/menu.h>
#include <wex/printing.h>
#include <wex/tokenizer.h>
#include <wex/util.h>

namespace wex
{
  // file_droptarget is already used by wex::frame.
  class droptarget : public wxFileDropTarget
  {
  public:
    explicit droptarget(listview* lv)
      : m_listview(lv) {;}
    bool OnDropFiles(wxCoord x, wxCoord y, 
      const wxArrayString& filenames) override
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
      else
      {
        return false;
      }
    };
  private:
    listview* m_listview;
  };

  template <typename T> int compare(T x, T y)
  {
    if (x > y) return 1;
    else if (x < y) return -1;
    else return 0;
  }

  std::string ignore_case(const std::string& text)
  {
    std::string output(text);

    if (!wex::find_replace_data::get()->match_case())
    {
      for (auto & c : output) c = std::toupper(c);
    }

    return output;
  };

  const std::vector < item > config_items() 
  {
    return std::vector < item > ({{"notebook", {
      {_("General"),
        {{_("list.Header"), item::CHECKBOX, std::any(true)},
         {_("list.Single selection"), item::CHECKBOX},
         {_("list.Comparator"), item::FILEPICKERCTRL},
         {_("list.Sort method"), {
           {SORT_ASCENDING, _("Sort ascending")},
           {SORT_DESCENDING, _("Sort descending")},
           {SORT_TOGGLE, _("Sort toggle")}}},
         {_("list.Context size"), 0, 80, 10},
         {_("list.Rulers"),  {
           {wxLC_HRULES, _("Horizontal rulers")},
           {wxLC_VRULES, _("Vertical rulers")}}, false}}},
      {_("Font"),
        {{_("list.Font"), 
          item::FONTPICKERCTRL, 
          wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT)}}},
      {_("Colour"),
        {{_("list.Readonly colour"), 
          item::COLOURPICKERWIDGET, 
          *wxLIGHT_GREY}}}}}});
    }
};

wex::column::column()
{
  SetColumn(-1);
}

wex::column::column(
  const std::string& name,
  type_t type,
  int width)
  : m_type(type)
{
  wxListColumnFormat align;

  switch (m_type)
  {
    case column::FLOAT: 
      align = wxLIST_FORMAT_RIGHT; 
      if (width == 0) width = 80; 
      break;
      
    case column::INT: 
      align = wxLIST_FORMAT_RIGHT;
      if (width == 0) width = 60; 
      break;
      
    case column::STRING: 
      align = wxLIST_FORMAT_LEFT;  
      if (width == 0) width = 100; 
      break;
      
    case column::DATE: 
      align = wxLIST_FORMAT_LEFT;  
      if (width == 0) width = 150; 
      break;
      
    default: assert(0);
  }

  SetColumn(-1); // default value, is set when inserting the col
  SetText(name);
  SetAlign(align);
  SetWidth(width);
}

void wex::column::set_is_sorted_ascending(sort_t type)
{
  switch (type)
  {
    case SORT_ASCENDING: m_is_sorted_ascending = true; break;
    case SORT_DESCENDING: m_is_sorted_ascending = false; break;
    case SORT_KEEP: break;
    case SORT_TOGGLE: m_is_sorted_ascending = !m_is_sorted_ascending; break;
    default: assert(0); break;
  }
}

// wxWindow::NewControlId() is negative...
const wxWindowID ID_COL_FIRST = 1000;

wex::listview::listview(const listview_data& data)
  : wxListView(
      data.window().parent(), 
      data.window().id(), 
      data.window().pos(), 
      data.window().size(), 
      data.window().style() == DATA_NUMBER_NOT_SET ? 
        wxLC_REPORT: data.window().style(), 
      data.control().validator() != nullptr ? 
        *data.control().validator(): wxDefaultValidator, 
      data.window().name())
  , m_image_height(16) // not used if IMAGE_FILE_ICON is used, then 16 is fixed
  , m_image_width(16)
  , m_data(this, listview_data(data).image(data.type() == listview_data::NONE ? 
      data.image(): listview_data::IMAGE_FILE_ICON))
{
  config_get();

  m_data.inject();

  // We can only have one drop target, we use file drop target,
  // as list items can also be copied and pasted.
  SetDropTarget(new droptarget(this));

  wxAcceleratorEntry entries[4];

  entries[0].Set(wxACCEL_NORMAL, WXK_DELETE, wxID_DELETE);
  entries[1].Set(wxACCEL_CTRL, WXK_INSERT, wxID_COPY);
  entries[2].Set(wxACCEL_SHIFT, WXK_INSERT, wxID_PASTE);
  entries[3].Set(wxACCEL_SHIFT, WXK_DELETE, wxID_CUT);

  wxAcceleratorTable accel(WXSIZEOF(entries), entries);
  SetAcceleratorTable(accel);
  
  switch (m_data.image())
  {
    case listview_data::IMAGE_NONE: 
      break;
    case listview_data::IMAGE_ART:
    case listview_data::IMAGE_OWN:
      AssignImageList(
        new wxImageList(m_image_width, m_image_height, true, 0), 
          wxIMAGE_LIST_SMALL);
      break;
    case listview_data::IMAGE_FILE_ICON:
      SetImageList(
        wxTheFileIconsTable->GetSmallImageList(), wxIMAGE_LIST_SMALL);
      break;
    default:
      assert(0);
  }

  frame::update_statusbar(this);

  Bind(wxEVT_FIND, [=](wxFindDialogEvent& event) {
    find_next(
      find_replace_data::get()->get_find_string(), 
      find_replace_data::get()->search_down());});
      
  Bind(wxEVT_FIND_NEXT, [=](wxFindDialogEvent& event) {
    find_next(
      find_replace_data::get()->get_find_string(), 
      find_replace_data::get()->search_down());});
      
  if (m_data.type() != listview_data::NONE)
  {
    Bind(wxEVT_IDLE, [=](wxIdleEvent& event) {
      event.Skip();
      if (
        !IsShown() ||
         interruptable::is_running() ||
         GetItemCount() == 0 ||
        !config("AllowSync").get(true))
      {
        return;
      }
      if (m_item_number < GetItemCount())
      {
        if (listitem item(this, m_item_number); 
            item.get_filename().file_exists() &&
            (item.get_filename().stat().get_modification_time() != 
             get_item_text(m_item_number, _("Modified")) ||
             item.get_filename().stat().is_readonly() != item.is_readonly())
           )
        {
          item.update();
          log::status() << item.get_filename();
          m_item_updated = true;
        }
    
        m_item_number++;
      }
      else
      {
        m_item_number = 0;
    
        if (m_item_updated)
        {
          if (m_data.type() == listview_data::FILE)
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
      }});
    }

  Bind(wxEVT_LIST_BEGIN_DRAG, [=](wxListEvent& event) {
    // Start drag operation.
    std::string text;
    for (auto i = GetFirstSelected(); i != -1; i = GetNextSelected(i))
      text += item_to_text(i) + "\n";
    if (!text.empty())
    {
      wxTextDataObject textData(text);
      wxDropSource source(textData, this);
      source.DoDragDrop(wxDragCopy);
    }});

  Bind(wxEVT_LIST_ITEM_ACTIVATED, [=] (wxListEvent& event) {
    item_activated(event.GetIndex());});
  
  Bind(wxEVT_LIST_ITEM_DESELECTED, [=](wxListEvent& event) {
    frame::update_statusbar(this);});
  
  Bind(wxEVT_LIST_ITEM_SELECTED, [=](wxListEvent& event) {
    if (m_data.type() != listview_data::NONE && GetSelectedItemCount() == 1)
    {
      if (const wex::path fn(listitem(this, event.GetIndex()).get_filename());
        fn.stat().is_ok())
      {
        log::status() << fn;
      }
      else
      {
        log::status(get_item_text(GetFirstSelected()));
      }
    }
    frame::update_statusbar(this);});
    
  Bind(wxEVT_LIST_COL_CLICK, [=](wxListEvent& event) {
    sort_column(
      event.GetColumn(),
      (sort_t)config(_("list.Sort method")).get(SORT_TOGGLE));});

  Bind(wxEVT_LIST_COL_RIGHT_CLICK, [=](wxListEvent& event) {
    m_to_be_sorted_column_no = event.GetColumn();

    menu menu(GetSelectedItemCount() > 0 ? 
      menu::IS_SELECTED: 
      menu::DEFAULT);
      
    menu.append({{wxID_SORT_ASCENDING}, {wxID_SORT_DESCENDING}});

    PopupMenu(&menu);});
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {clear();}, wxID_CLEAR);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {copy_selection_to_clipboard();},
    wxID_COPY);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {edit_delete();}, wxID_DELETE);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    item_from_text(clipboard_get());}, wxID_PASTE);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    SetItemState(-1, 
      wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);}, wxID_SELECTALL);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    sort_column(m_to_be_sorted_column_no, SORT_ASCENDING);}, wxID_SORT_ASCENDING);
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    sort_column(m_to_be_sorted_column_no, SORT_DESCENDING);}, wxID_SORT_DESCENDING);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    copy_selection_to_clipboard();
    edit_delete();}, wxID_CUT);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    for (auto i = 0; i < GetItemCount(); i++)
    {
      Select(i, !IsSelected(i));
    }}, ID_EDIT_SELECT_INVERT);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    for (auto i = 0; i < GetItemCount(); i++)
    {
      Select(i, false);
    }}, ID_EDIT_SELECT_NONE);
  
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    std::string defaultPath;
    if (GetSelectedItemCount() > 0)
    {
      defaultPath = listitem(
        this, GetFirstSelected()).get_filename().string();
    }
    wxDirDialog dir_dlg(
      this,
      _(wxDirSelectorPromptStr),
      defaultPath,
      wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);
    if (dir_dlg.ShowModal() == wxID_OK)
    {
      const auto no = (GetSelectedItemCount() > 0 ? 
        GetFirstSelected(): GetItemCount());
       
      listitem(this, dir_dlg.GetPath().ToStdString()).insert(no);
    }}, wxID_ADD);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    for (auto i = GetFirstSelected(); i != -1; i = GetNextSelected(i))
    {
      item_activated(i);
    }}, ID_EDIT_OPEN);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (!IsShown() || GetItemCount() == 0) return false;
    if (const auto val(wxGetNumberFromUser(
      _("Input") + " (1 - " + std::to_string(GetItemCount()) + "):",
      wxEmptyString,
      _("Enter Item Number"),
      (GetFirstSelected() == -1 ? 1: GetFirstSelected() + 1),
      1,
      GetItemCount())); val > 0)
    {
      listview_data(control_data().line(val), this).inject();
    }
    return true;}, wxID_JUMP_TO);

  Bind(wxEVT_RIGHT_DOWN, [=](wxMouseEvent& event) {
    menu::menu_t style(menu::menu_t().set(menu::IS_POPUP));
    if (GetSelectedItemCount() > 0) style.set(menu::IS_SELECTED);
    if (GetItemCount() == 0) style.set(menu::IS_EMPTY);
    if (m_data.type() != listview_data::FIND) style.set(menu::CAN_PASTE);
    if (GetSelectedItemCount() == 0 && GetItemCount() > 0) 
    {
      style.set(menu::ALLOW_CLEAR);
    }
    wex::menu menu(style);
    build_popup_menu(menu);
    if (menu.GetMenuItemCount() > 0)
    {
      PopupMenu(&menu);
    }});
    
  Bind(wxEVT_SET_FOCUS, [=](wxFocusEvent& event) {
    if (auto* frame = dynamic_cast<wex::frame*>(wxTheApp->GetTopWindow()); 
      frame != nullptr)
    {
      frame->set_find_focus(this);
    }
    event.Skip();});
  
  Bind(wxEVT_SHOW, [=](wxShowEvent& event) {
    event.Skip();
    frame::update_statusbar(this);});
}    

bool wex::listview::append_columns(const std::vector <column>& cols)
{
  SetSingleStyle(wxLC_REPORT);

  for (const auto& col : cols)
  {
    auto mycol(col);
    
    if (const auto index = wxListView::AppendColumn(
      mycol.GetText(), mycol.GetAlign(), mycol.GetWidth());
      index == -1) return false;

    mycol.SetColumn(GetColumnCount() - 1);
    m_columns.emplace_back(mycol);
      
    Bind(wxEVT_MENU,  [=](wxCommandEvent& event) {
      sort_column(event.GetId() - ID_COL_FIRST, SORT_TOGGLE);},
      ID_COL_FIRST + GetColumnCount() - 1);
  }
  
  return true;
}

const std::string wex::listview::build_page()
{
  std::stringstream text;

  text << "<TABLE "
       << (((GetWindowStyle() & wxLC_HRULES) || (GetWindowStyle() & wxLC_VRULES)) 
          ? "border=1": "border=0")
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
  if (GetSelectedItemCount() >= 1 && 
    listitem(this, GetFirstSelected()).get_filename().stat().is_ok())
  {
    menu.append({{ID_EDIT_OPEN, _("&Open"), wxART_FILE_OPEN}, {}});
  }

  menu.append({{}, {menu_item::EDIT_INVERT}});
  
  if (
    GetItemCount() > 0 && 
    GetSelectedItemCount() == 0 &&
    InReportView())
  {
    menu.append({{}});

    auto* menuSort = new wex::menu;

    for (const auto& it : m_columns)
    {
      menuSort->append({{ID_COL_FIRST + it.GetColumn(), it.GetText()}});
    }

    menu.append({{menuSort, _("Sort On")}});
  }
  
  if (m_data.type() == listview_data::FOLDER && GetSelectedItemCount() <= 1)
  {
    menu.append({{}, {wxID_ADD}});
  }
}

void wex::listview::clear()
{
  DeleteAllItems();

  sort_column_reset();

  frame::update_statusbar(this);
}

int wex::listview::config_dialog(const window_data& par)
{
  const window_data data(window_data(par).title(_("List Options")));

  if (m_config_dialog == nullptr)
  {
    m_config_dialog = new item_dialog(config_items(), data);
  }

  return (data.button() & wxAPPLY) ?
    m_config_dialog->Show(): m_config_dialog->ShowModal();
}
          
void wex::listview::config_get()
{
  const auto& ci (config_items());
  const item_vector& iv(&ci);

  lexers::get()->apply_default_style(
    [=](const std::string& back) {
      SetBackgroundColour(wxColour(back));});
  
  SetFont(iv.find<wxFont>(_("list.Font")));
  SetSingleStyle(wxLC_HRULES, (iv.find<long>(_("list.Rulers")) & wxLC_HRULES) > 0);
  SetSingleStyle(wxLC_VRULES, (iv.find<long>(_("list.Rulers")) & wxLC_VRULES) > 0);
  SetSingleStyle(wxLC_NO_HEADER, !iv.find<bool>(_("list.Header")));
  SetSingleStyle(wxLC_SINGLE_SEL, iv.find<bool>(_("list.Single selection")));
  
  items_update();
}
  
void wex::listview::copy_selection_to_clipboard()
{
  if (GetSelectedItemCount() == 0) return;

  wxBusyCursor wait;
  std::string clipboard;

  for (auto i = GetFirstSelected(); i != -1; i = GetNextSelected(i))
    clipboard += item_to_text(i) + "\n";
    
  clipboard_add(clipboard);
}

void wex::listview::edit_delete()
{
  if (GetSelectedItemCount() == 0) return;

  long old_item = -1;

  for (long i = -1; (i = GetNextSelected(i)) != -1; )
  {
    DeleteItem(i);
    old_item = i;
    i = -1;
  }

  if (old_item != -1 && old_item < GetItemCount())
    SetItemState(old_item, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);

  items_update();
}
  
bool wex::listview::find_next(const std::string& text, bool forward)
{
  if (text.empty())
  {
    return false;
  }

  std::string text_use = ignore_case(text);

  const auto firstselected = GetFirstSelected();
  static bool recursive = false;
  static long start_item;
  static long end_item;

  if (forward)
  {
    start_item = recursive ? 0: (firstselected != -1 ? firstselected + 1: 0);
    end_item = GetItemCount();
  }
  else
  {
    start_item = recursive ? GetItemCount() - 1: (firstselected != -1 ? firstselected - 1: 0);
    end_item = -1;
  }

  int match = -1;

  for (
    int index = start_item;
    index != end_item && match == -1;
    (forward ? index++: index--))
  {
    std::string text;

    for (int col = 0; col < GetColumnCount() && match == -1; col++)
    {
      text = ignore_case(std::string(GetItemText(index, col)));

      if (find_replace_data::get()->match_word())
      {
        if (text == text_use)
        {
          match = index;
        }
      }
      else
      {
        if (text.find(text_use) != std::string::npos)
        {
          match = index;
        }
      }
    }
  }

  if (match != -1)
  {
    recursive = false;

    Select(match);
    EnsureVisible(match);

    if (firstselected != -1 && match != firstselected)
    {
      Select(firstselected, false);
    }

    return true;
  }
  else
  {
    frame::statustext(get_find_result(text, forward, recursive), std::string());
    
    if (!recursive)
    {
      recursive = true;
      find_next(text, forward);
      recursive = false;
    }
    
    return false;
  }
}

unsigned int wex::listview::get_art_id(const wxArtID& artid)
{
  if (const auto& it = m_art_ids.find(artid); it != m_art_ids.end())
  {
    return it->second;
  }
  else
  {
    if (auto* il = GetImageList(wxIMAGE_LIST_SMALL); il == nullptr)
    {
      assert(0);
      return 0;
    }
    else 
    {
      m_art_ids.insert({artid, il->GetImageCount()});
      return il->Add(wxArtProvider::GetBitmap(
        artid, wxART_OTHER, wxSize(m_image_width, m_image_height)));
    }
  }
}

wex::column wex::listview::get_column(const std::string& name) const
{
  for (const auto& it : m_columns)
  {
    if (it.GetText() == name)
    {
      return it;
    }
  }
  
  return column();
}

const std::string wex::listview::get_item_text(
  long item_number, const std::string& col_name) const 
{
  if (col_name.empty())
  {
    return GetItemText(item_number);
  }
  
  const int col = find_column(col_name);
  return col < 0 ? 
    std::string(): GetItemText(item_number, col).ToStdString();
}

bool wex::listview::insert_item(const std::vector < std::string > & item)
{
  if (
    item.empty() || 
    item.front().empty() || 
    item.size() > m_columns.size()) 
  {
    return false;
  }

  int no = 0;
  long index = 0;

  for (const auto& col : item)
  {
    try
    {
      if (!col.empty())
      {
        switch (m_columns[no].type())
        {
          case column::DATE:
            if (const auto& [r, t] = get_time(col); !r) return false;
            break;
          case column::FLOAT: std::stof(col); 
            break;
          case column::INT: std::stoi(col); 
            break;
          case column::STRING: 
            break;
          default: 
            break;
        }

        if (no == 0)
        {
          if ((index = InsertItem(GetItemCount(), col)) == -1)
          {
            return false;
          }

          if (std::vector<std::string> v; match(",fore:(.*)", 
            lexers::get()->get_default_style().value(), v) > 0)
          {
            SetItemTextColour(index, wxColour(v[0]));
          }
        }
        else
        {
          if (!set_item(index, no, col)) return false;
        }
      }
      
      no++;
    }
    catch (std::exception& e)
    {
      log::verbose(e) << "insert_item exception:" << col;
      return false;
    }
  }

  return true;
}

void wex::listview::item_activated(long item_number)
{
  assert(item_number >= 0);
  
  if (m_data.type() == listview_data::FOLDER)
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
  else
  {
    // Cannot be const because of SetItem later on.
    if (listitem item(this, item_number);
      item.get_filename().file_exists())
    {
      if (auto* frame = dynamic_cast<wex::frame*>(wxTheApp->GetTopWindow());
        frame != nullptr)
      {
        const auto no(get_item_text(item_number, _("Line No")));
        auto data(
          (m_data.type() == listview_data::FIND && !no.empty() ?
             control_data().
               line(std::stoi(no)). 
               find(get_item_text(item_number, _("Match"))): 
             control_data()));

        frame->open_file(item.get_filename(), data);
      }
    }
    else if (item.get_filename().dir_exists())
    {
      wxTextEntryDialog dlg(this,
        _("Input") + ":",
        _("Folder Type"),
        get_item_text(item_number, _("Type")));
  
      if (dlg.ShowModal() == wxID_OK)
      {
        item.set_item(_("Type"), dlg.GetValue());
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
  
  for (tokenizer tkz(text, "\n"); tkz.has_more_tokens(); )
  {
    if (m_data.type() != listview_data::NONE)
    {
      modified = true;
    
      if (!InReportView())
      {
        listitem(this, tkz.get_next_token()).insert();
      }
      else
      {
        const auto token(tkz.get_next_token());
        tokenizer tk(token, std::string(1, field_separator()));
        
        if (tk.has_more_tokens())
        {
          const auto value = tk.get_next_token();
    
          if (path fn(value); fn.file_exists())
          {
            listitem item(this, fn);
            item.insert();
    
            // And try to set the rest of the columns 
            // (that are not already set by inserting).
            int col = 1;
            while (tk.has_more_tokens() && col < GetColumnCount() - 1)
            {
              const auto value = tk.get_next_token();
    
              if (col != find_column(_("Type")) &&
                  col != find_column(_("In Folder")) &&
                  col != find_column(_("Size")) &&
                  col != find_column(_("Modified")))
              {
                if (!set_item(item.GetId(), col, value)) return false;
              }
    
              col++;
            }
          }
          else
          {
            // Now we need only the first column (containing findfiles). If more
            // columns are present, these are ignored.
            const auto findfiles =
              (tk.has_more_tokens() ? tk.get_next_token(): tk.get_string());
    
            listitem(this, value, findfiles).insert();
          }
        }
        else
        {
          listitem(this, token).insert();
        }
      }
    }
    else
    {
      if (const auto line = tkz.get_next_token();
        insert_item(tokenizer(line, std::string(1, m_field_separator)).
        tokenize<std::vector < std::string >>()))
      {
        modified = true;
      }
    }
  }

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

  switch (m_data.type())
  {
    case listview_data::FILE:
    case listview_data::HISTORY: {
      const listitem item(const_cast< listview * >(this), item_number);
      text = (item.get_filename().stat().is_ok() ? 
        item.get_filename().string(): 
        item.get_filename().fullname());

      if (item.get_filename().dir_exists() && !item.get_filename().file_exists())
      {
        text += field_separator() + get_item_text(item_number, _("Type"));
      }}
      break;

    case listview_data::FOLDER:
      return GetItemText(item_number);
      break;
    
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
  if (m_data.type() != listview_data::NONE)
  {
    for (auto i = 0; i < GetItemCount(); i++)
    {
      listitem(this, i).update();
    }
  }
}

bool wex::listview::load(const std::list<std::string> & l)
{
  clear();

  for (const auto& it : l)
  {
    item_from_text(it);
  }
  
  return true;
}
  
void wex::listview::print()
{
#if wxUSE_HTML & wxUSE_PRINTING_ARCHITECTURE
  wxBusyCursor wait;
  printing::get()->get_html_printer()->PrintText(build_page());
#endif
}

void wex::listview::print_preview()
{
#if wxUSE_HTML & wxUSE_PRINTING_ARCHITECTURE
  wxBusyCursor wait;
  printing::get()->get_html_printer()->PreviewText(build_page());
#endif
}

const std::list<std::string> wex::listview::save() const
{
  std::list<std::string> l;

  for (auto i = 0; i < GetItemCount(); i++)
  {
    l.push_back(GetItemText(i));
  }
  
  return l;
}
  
std::vector<std::string>* pitems;

int wxCALLBACK compare_cb(
  wxIntPtr item1, wxIntPtr item2, wxIntPtr sortData)
{
  const bool ascending = (sortData > 0);
  const auto& str1 = (*pitems)[item1];
  const auto& str2 = (*pitems)[item2];

  // should return 0 if the items are equal, 
  // negative value if the first item is less than the second one 
  // and positive value if the first one is greater than the second one
  
  if (str1.empty() && str2.empty())
  {
    return 0;
  }
  else if (str1.empty())
  {
    return -1;
  }
  else if (str2.empty())
  {
    return 1;
  }
  
  switch (const auto type = 
    (wex::column::type_t)std::abs(sortData); type) 
  {
    case wex::column::DATE:
      {
        const auto& [r1, t1] = wex::get_time(str1);
        const auto& [r2, t2] = wex::get_time(str2);
        if (!r1 || !r2) return 0;
        if (ascending) return wex::compare((unsigned long)t1, (unsigned long)t2);
        else           return wex::compare((unsigned long)t2, (unsigned long)t1);
      }
    break;

    case wex::column::FLOAT:
      if (ascending) return wex::compare(std::stof(str1), std::stof(str2));
      else           return wex::compare(std::stof(str2), std::stof(str1));
    break;

    case wex::column::INT:
      if (ascending) return wex::compare(std::stoi(str1), std::stoi(str2));
      else           return wex::compare(std::stoi(str2), std::stoi(str1));
    break;

    case wex::column::STRING:
      if (!wex::find_replace_data::get()->match_case())
      {
        if (ascending) 
          return wex::ignore_case(str1).compare(wex::ignore_case(str2));
        else           
          return wex::ignore_case(str2).compare(wex::ignore_case(str1));
      }
      else
      {
        if (ascending) 
          return str1.compare(str2);
        else           
          return str2.compare(str1);
      }
    break;

    default:
      assert(0);
  }

  return 0;
}

bool wex::listview::set_item(
  long index, int column, const std::string& text, int imageId)
{
  if (text.empty()) return true;

  try
  {
    switch (m_columns[column].type())
    {
      case column::DATE:
        if (const auto& [r, t] = get_time(text); !r) return false;
        break;
      case column::FLOAT: std::stof(text); break;
      case column::INT: std::stoi(text); break;
      case column::STRING: break;
      default: break;
    }

    return wxListView::SetItem(index, column, text, imageId);
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
  
  return (m_data.image() == listview_data::IMAGE_ART ?
    SetItemImage(item_number, get_art_id(artid)): false);
}
        
bool wex::listview::sort_column(int column_no, sort_t sort_method)
{
  if (column_no == -1 || column_no >= (int)m_columns.size())
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

  const wxIntPtr sortdata =
    (sorted_col.is_sorted_ascending() ?
       sorted_col.type(): (0 - sorted_col.type()));

  try
  {
    SortItems(compare_cb, sortdata);

    m_sorted_column_no = column_no;

    if (m_data.image() != listview_data::IMAGE_NONE)
    {
      SetColumnImage(column_no, get_art_id(
        sorted_col.is_sorted_ascending() ? wxART_GO_DOWN: wxART_GO_UP));
    }

    if (GetItemCount() > 0)
    {
      items_update();
      after_sorting();
    }

    log::status(_("Sorted on")) << sorted_col.GetText();
  }
  catch (std::exception& e)
  {
    log(e) << "sort:" << sorted_col.GetText();
    return false;
  }
  
  return true;
}

void wex::listview::sort_column_reset()
{
  if (m_sorted_column_no != -1 && !m_art_ids.empty()) // only if we are using images
  {
    ClearColumnImage(m_sorted_column_no);
    m_sorted_column_no = -1;
  }
}
