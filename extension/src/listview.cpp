////////////////////////////////////////////////////////////////////////////////
// Name:      listview.cpp
// Purpose:   Implementation of wxExListView and related classes
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/artprov.h>
#include <wx/config.h>
#include <wx/dnd.h> 
#include <wx/fdrepdlg.h> // for wxFindDialogEvent
#include <wx/numdlg.h> // for wxGetNumberFromUser
#include <wx/generic/dirctrlg.h> // for wxTheFileIconsTable
#include <wx/imaglist.h>
#include <wx/textfile.h> // for wxTextFile::GetEOL()
#include <wx/tokenzr.h>
#include <wx/extension/listview.h>
#include <wx/extension/defs.h>
#include <wx/extension/frame.h>
#include <wx/extension/frd.h>
#include <wx/extension/interruptable.h>
#include <wx/extension/item.h>
#include <wx/extension/itemdlg.h>
#include <wx/extension/lexer.h>
#include <wx/extension/listitem.h>
#include <wx/extension/menu.h>
#include <wx/extension/printing.h>
#include <wx/extension/util.h>

#if wxUSE_GUI

class ListViewDefaults : public wxExConfigDefaults
{
public:
  ListViewDefaults() 
  : wxExConfigDefaults(std::vector<std::tuple<wxString, wxExItemType, wxAny>> {
    std::make_tuple(_("Background colour"), ITEM_COLOURPICKERWIDGET, *wxWHITE),
    std::make_tuple(_("Foreground colour"), ITEM_COLOURPICKERWIDGET, *wxBLACK),
    std::make_tuple(_("Readonly colour"), ITEM_COLOURPICKERWIDGET, *wxLIGHT_GREY),
    std::make_tuple(_("Header"), ITEM_CHECKBOX, true)}) {;};
};
  
#if wxUSE_DRAG_AND_DROP
// FileDropTarget is already used by wxExFrame.
class DropTarget : public wxFileDropTarget
{
public:
  explicit DropTarget(wxExListView* lv) {m_ListView = lv;}
private:
  virtual bool OnDropFiles(
    wxCoord x, 
    wxCoord y, 
    const wxArrayString& filenames) 
  {
    // Only drop text if nothing is selected,
    // so dropping on your own selection is impossible.
    if (m_ListView->GetSelectedItemCount() == 0)
    {
      for (size_t i = 0; i < filenames.GetCount(); i++)
      {
        m_ListView->ItemFromText(filenames[i]);
      }
    
      return true;
    }
    else
    {
      return false;
    }
  };
        
  wxExListView* m_ListView;
};

// A text drop target, is not used, but could
// be used instead of the file drop target.
class TextDropTarget : public wxTextDropTarget
{
public:
  explicit TextDropTarget(wxExListView* lv) {m_ListView = lv;}
private:
  virtual bool OnDropText(
    wxCoord x, 
    wxCoord y, 
    const wxString& text) {
      // Only drop text if nothing is selected,
      // so dropping on your own selection is impossible.
      return (m_ListView->GetSelectedItemCount() == 0 ?
        m_ListView->ItemFromText(text): false);};
        
  wxExListView* m_ListView;
};
#endif

wxExColumn::wxExColumn()
  : m_Type(COL_INVALID)
  , m_IsSortedAscending(false)
{
  SetColumn(-1);
}

wxExColumn::wxExColumn(
  const wxString& name,
  wxExColumn::wxExColumnType type,
  int width)
  : wxListItem()
  , m_Type(type)
  , m_IsSortedAscending(false)
{
  wxListColumnFormat align = wxLIST_FORMAT_RIGHT;

  switch (m_Type)
  {
    case wxExColumn::COL_FLOAT: 
      align = wxLIST_FORMAT_RIGHT; 
      if (width == 0) width = 80; 
      break;
      
    case wxExColumn::COL_INT: 
      align = wxLIST_FORMAT_RIGHT; 
      if (width == 0) width = 60; 
      break;
      
    case wxExColumn::COL_STRING: 
      align = wxLIST_FORMAT_LEFT;  
      if (width == 0) width = 100; 
      break;
      
    case wxExColumn::COL_DATE: 
      align = wxLIST_FORMAT_LEFT;  
      if (width == 0) width = 150; 
      break;
      
    default: wxFAIL;
  }

  SetColumn(-1); // default value, is set when inserting the col
  SetText(name);
  SetAlign(align);
  SetWidth(width);
}

void wxExColumn::SetIsSortedAscending(wxExSortType type)
{
  switch (type)
  {
    case SORT_ASCENDING: m_IsSortedAscending = true; break;
    case SORT_DESCENDING: m_IsSortedAscending = false; break;
    case SORT_KEEP: break;
    case SORT_TOGGLE: m_IsSortedAscending = !m_IsSortedAscending; break;
    default: wxFAIL; break;
  }
}

// wxWindow::NewControlId() is negative...
const wxWindowID ID_COL_FIRST = 1000;

wxExItemDialog* wxExListView::m_ConfigDialog = nullptr;

wxExListView::wxExListView(wxWindow* parent,
  wxExListType type,
  wxWindowID id,
  const wxExLexer* lexer,
  const wxPoint& pos,
  const wxSize& size,
  long style,
  wxExImageType image_type,
  const wxValidator& validator,
  const wxString &name)
  : wxListView(parent, id, pos, size, style, validator, name)
  , m_FieldSeparator('\t')
  , m_ImageType(type == LIST_NONE ? image_type: IMAGE_FILE_ICON) 
  , m_ImageHeight(16) // not used if IMAGE_FILE_ICON is used, then 16 is fixed
  , m_ImageWidth(16)
  , m_SortedColumnNo(-1)
  , m_ToBeSortedColumnNo(-1)
  , m_Type(type)
  , m_ItemUpdated(false)
  , m_ItemNumber(0)
{
  Initialize(lexer);
  
#if wxUSE_DRAG_AND_DROP
  // We can only have one drop target, we use file drop target,
  // as list items can also be copied and pasted.
  SetDropTarget(new DropTarget(this));
#endif
  
  switch (m_ImageType)
  {
    case IMAGE_NONE: break;
    case IMAGE_ART:
    case IMAGE_OWN:
      AssignImageList(
        new wxImageList(m_ImageWidth, m_ImageHeight, true, 0), 
        wxIMAGE_LIST_SMALL);
      break;
    case IMAGE_FILE_ICON:
      SetImageList(wxTheFileIconsTable->GetSmallImageList(), wxIMAGE_LIST_SMALL);
      break;
    default:
      wxFAIL;
  }

#if wxUSE_STATUSBAR
  wxExFrame::UpdateStatusBar(this);
#endif  

  Bind(wxEVT_FIND, [=](wxFindDialogEvent& event) {
    FindNext(
      wxExFindReplaceData::Get()->GetFindString(), 
      wxExFindReplaceData::Get()->SearchDown());});
      
  Bind(wxEVT_FIND_NEXT, [=](wxFindDialogEvent& event) {
    FindNext(
      wxExFindReplaceData::Get()->GetFindString(), 
      wxExFindReplaceData::Get()->SearchDown());});
      
  if (m_Type != LIST_NONE)
  {
    Bind(wxEVT_IDLE, [=](wxIdleEvent& event) {
      event.Skip();
      if (
        !IsShown() ||
         wxExInterruptable::Running() ||
         GetItemCount() == 0 ||
        !wxConfigBase::Get()->ReadBool("AllowSync", true))
      {
        return;
      }
      if (m_ItemNumber < GetItemCount())
      {
        wxExListItem item(this, m_ItemNumber);
    
        if ( item.GetFileName().FileExists() &&
            (item.GetFileName().GetStat().GetModificationTime() != 
             GetItemText(m_ItemNumber, _("Modified")) ||
             item.GetFileName().GetStat().IsReadOnly() != item.IsReadOnly())
            )
        {
          item.Update();
          wxExLogStatus(item.GetFileName(), STAT_SYNC | STAT_FULLPATH);
          m_ItemUpdated = true;
        }
    
        m_ItemNumber++;
      }
      else
      {
        m_ItemNumber = 0;
    
        if (m_ItemUpdated)
        {
          if (m_Type == LIST_FILE)
          {
            if (
              wxConfigBase::Get()->ReadBool("List/SortSync", true) &&
              GetSortedColumnNo() == FindColumn(_("Modified")))
            {
              SortColumn(_("Modified"), SORT_KEEP);
            }
          }
    
          m_ItemUpdated = false;
        }
      }});
    }

#if wxUSE_DRAG_AND_DROP
  Bind(wxEVT_LIST_BEGIN_DRAG, [=](wxListEvent& event) {
    // Start drag operation.
    wxString text;
    for (long i = GetFirstSelected(); i != -1; i = GetNextSelected(i))
      text += ItemToText(i) + wxTextFile::GetEOL();
    if (!text.empty())
    {
      wxTextDataObject textData(text);
      wxDropSource source(textData, this);
      source.DoDragDrop(wxDragCopy);
    }});
#endif

  Bind(wxEVT_LIST_ITEM_ACTIVATED, [=] (wxListEvent& event) {
    ItemActivated(event.GetIndex());});
  
#if wxUSE_STATUSBAR
  Bind(wxEVT_LIST_ITEM_DESELECTED, [=](wxListEvent& event) {
    wxExFrame::UpdateStatusBar(this);});
#endif  
  
  Bind(wxEVT_LIST_ITEM_SELECTED, [=](wxListEvent& event) {
    if (m_Type != LIST_NONE && GetSelectedItemCount() == 1)
    {
      const wxExFileName fn(wxExListItem(this, event.GetIndex()).GetFileName());
      if (fn.GetStat().IsOk())
      {
        wxExLogStatus(fn, STAT_FULLPATH);
      }
      else
      {
        wxLogStatus(GetItemText(GetFirstSelected()));
      }
    }
    wxExFrame::UpdateStatusBar(this);});
    
  Bind(wxEVT_LIST_COL_CLICK, [=](wxListEvent& event) {
    SortColumn(
      event.GetColumn(),
      (wxExSortType)wxConfigBase::Get()->ReadLong(_("Sort method"), 
         SORT_TOGGLE));});

  Bind(wxEVT_LIST_COL_RIGHT_CLICK, [=](wxListEvent& event) {
    m_ToBeSortedColumnNo = event.GetColumn();

    wxExMenu menu(GetSelectedItemCount() > 0 ? 
      wxExMenu::MENU_IS_SELECTED: 
      wxExMenu::MENU_DEFAULT);
      
    menu.Append(wxID_SORT_ASCENDING);
    menu.Append(wxID_SORT_DESCENDING);

    PopupMenu(&menu);});
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    EditDelete();}, wxID_DELETE);
    
  Bind(wxEVT_MENU,  [=](wxCommandEvent& event) {
    SetItemState(-1, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);}, wxID_SELECTALL);
    
  Bind(wxEVT_MENU,  [=](wxCommandEvent& event) {
    SortColumn(m_ToBeSortedColumnNo, SORT_ASCENDING);}, wxID_SORT_ASCENDING);
    
  Bind(wxEVT_MENU,  [=](wxCommandEvent& event) {
    SortColumn(m_ToBeSortedColumnNo, SORT_DESCENDING);}, wxID_SORT_DESCENDING);
    
  Bind(wxEVT_MENU,  [=](wxCommandEvent& event) {
    for (int i = 0; i < GetItemCount(); i++)
    {
      Select(i, !IsSelected(i));
    }}, ID_EDIT_SELECT_INVERT);
    
  Bind(wxEVT_MENU,  [=](wxCommandEvent& event) {
    for (int i = 0; i < GetItemCount(); i++)
    {
      Select(i, false);
    }}, ID_EDIT_SELECT_NONE);
  
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    CopySelectedItemsToClipboard();
    EditDelete();}, wxID_CUT);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    EditClearAll();}, wxID_CLEAR);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    CopySelectedItemsToClipboard();}, wxID_COPY);
    
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    ItemFromText(wxExClipboardGet());}, wxID_PASTE);
  
  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    wxString defaultPath;
    
    if (GetSelectedItemCount() > 0)
    {
      defaultPath = wxExListItem(
        this, GetFirstSelected()).GetFileName().GetFullPath();
    }
    
    wxDirDialog dir_dlg(
      this,
      _(wxDirSelectorPromptStr),
      defaultPath,
      wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);

    if (dir_dlg.ShowModal() == wxID_OK)
    {
      const int no = (GetSelectedItemCount() > 0 ? 
        GetFirstSelected(): GetItemCount());
       
      wxExListItem(this, dir_dlg.GetPath()).Insert(no);
    }}, wxID_ADD);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    for (int i = GetFirstSelected(); i != -1; i = GetNextSelected(i))
    {
      ItemActivated(i);
    }}, ID_EDIT_OPEN);

  Bind(wxEVT_MENU, [=](wxCommandEvent& event) {
    if (!IsShown() || GetItemCount() == 0) return false;
    
    const wxString& caption = _("Enter Item Number");
    const int initial_value = (GetFirstSelected() == -1 ? 1: GetFirstSelected() + 1);
    long val;

    if ((val = wxGetNumberFromUser(
      _("Input") + wxString::Format(" (1 - %d):", GetItemCount()),
      wxEmptyString,
      caption,
      initial_value,
      1,
      GetItemCount())) > 0)
    {
      if (initial_value >= 1)
      {
        Select(initial_value - 1, false);
      }
      Select(val - 1);
      EnsureVisible(val - 1);
    }
    return true;}, wxID_JUMP_TO);

  Bind(wxEVT_RIGHT_DOWN, [=](wxMouseEvent& event) {
    long style = 0; // otherwise CAN_PASTE already on
    
    if (GetSelectedItemCount() > 0) style |= wxExMenu::MENU_IS_SELECTED;
    if (GetItemCount() == 0) style |= wxExMenu::MENU_IS_EMPTY;
    if (m_Type != LIST_FIND && m_Type != LIST_REPLACE) style |= wxExMenu::MENU_CAN_PASTE;
    
    if (GetSelectedItemCount() == 0 && GetItemCount() > 0) 
    {
      style |= wxExMenu::MENU_ALLOW_CLEAR;
    }

    wxExMenu menu(style);

    BuildPopupMenu(menu);

    if (menu.GetMenuItemCount() > 0)
    {
      PopupMenu(&menu);
    }});
    
  Bind(wxEVT_SET_FOCUS, [=](wxFocusEvent& event) {
    wxExFrame* frame = dynamic_cast<wxExFrame*>(wxTheApp->GetTopWindow());
    if (frame != nullptr)
    {
      frame->SetFindFocus(this);
    }
    event.Skip();});
  
#if wxUSE_STATUSBAR
  Bind(wxEVT_SHOW, [=](wxShowEvent& event) {
    event.Skip();
    wxExFrame::UpdateStatusBar(this);});
#endif  
}    

void wxExListView::AddColumns(const wxExLexer* lexer)
{
  const int col_line_width = 250;

  AppendColumn(wxExColumn(_("File Name"), wxExColumn::COL_STRING));

  switch (m_Type)
  {
    case LIST_FIND:
    case LIST_REPLACE:
      AppendColumn(wxExColumn(_("Line"), wxExColumn::COL_STRING, col_line_width));
      AppendColumn(wxExColumn(_("Match"), wxExColumn::COL_STRING));
      AppendColumn(wxExColumn(_("Line No")));
      
      if (m_Type == LIST_REPLACE)
      {
        AppendColumn(wxExColumn(_("Replaced")));
      }
    break;
    case LIST_KEYWORD:
      for (const auto& it : lexer->GetKeywords())
      {
        AppendColumn(wxExColumn(it));
      }

      AppendColumn(wxExColumn(_("Keywords")));
    break;
    default: break; // to prevent warnings
  }

  AppendColumn(wxExColumn(_("Modified"), wxExColumn::COL_DATE));
  AppendColumn(wxExColumn(_("In Folder"), wxExColumn::COL_STRING, 175));
  AppendColumn(wxExColumn(_("Type"), wxExColumn::COL_STRING));
  AppendColumn(wxExColumn(_("Size")));
}

long wxExListView::AppendColumn(const wxExColumn& col)
{
  wxExColumn mycol(col);
  
  const long index = wxListView::AppendColumn(
    mycol.GetText(), mycol.GetAlign(), mycol.GetWidth());
  
  if (index != -1)
  {
    mycol.SetColumn(GetColumnCount() - 1);
    m_Columns.emplace_back(mycol);
    
    Bind(wxEVT_MENU,  [=](wxCommandEvent& event) {
      SortColumn(event.GetId() - ID_COL_FIRST, SORT_TOGGLE);},
      ID_COL_FIRST + GetColumnCount() - 1);
  }
  
  return index;
}

const wxString wxExListView::BuildPage()
{
  wxString text;
  text << "<TABLE "
       << (((GetWindowStyle() & wxLC_HRULES) || (GetWindowStyle() & wxLC_VRULES)) 
          ? "border=1": "border=0")
       << " cellpadding=4 cellspacing=0 >" << wxTextFile::GetEOL()
       << "<tr>" << wxTextFile::GetEOL();

  for (int c = 0; c < GetColumnCount(); c++)
  {
    wxListItem col;
    GetColumn(c, col);
    text << "<td><i>" << col.GetText() << "</i>" << wxTextFile::GetEOL();
  }

  for (int i = 0; i < GetItemCount(); i++)
  {
    text << "<tr>" << wxTextFile::GetEOL();

    for (int col = 0; col < GetColumnCount(); col++)
    {
      text << "<td>" << wxListView::GetItemText(i, col) << wxTextFile::GetEOL();
    }
  }

  text << "</TABLE>" << wxTextFile::GetEOL();

  return text;
}

void wxExListView::BuildPopupMenu(wxExMenu& menu)
{
  if (GetSelectedItemCount() >= 1 && 
    wxExListItem(this, GetFirstSelected()).GetFileName().GetStat().IsOk())
  {
    menu.Append(ID_EDIT_OPEN, _("&Open"), wxART_FILE_OPEN);
    menu.AppendSeparator();
  }

  menu.AppendSeparator();
  menu.AppendEdit(true);
  
  if (
    GetItemCount() > 0 && 
    GetSelectedItemCount() == 0 &&
    InReportView())
  {
    menu.AppendSeparator();

    wxMenu* menuSort = new wxMenu;

    for (const auto& it : m_Columns)
    {
      menuSort->Append(ID_COL_FIRST + it.GetColumn(), it.GetText());
    }

    menu.AppendSubMenu(menuSort, _("Sort On"));
  }
  
  if (m_Type == LIST_FOLDER && GetSelectedItemCount() <= 1)
  {
    menu.AppendSeparator();
    menu.Append(wxID_ADD);
  }
}

const wxExColumn wxExListView::Column(const wxString& name) const
{
  for (const auto& it : m_Columns)
  {
    if (it.GetText() == name)
    {
      return it;
    }
  }
  
  return wxExColumn();
}

int wxExListView::ConfigDialog(
  wxWindow* parent,
  const wxString& title,
  long button_flags,
  wxWindowID id)
{
  ListViewDefaults use;
  
  static const std::vector<wxExItem> items {
    wxExItem("notebook", wxExItem::ItemsNotebook {
      {_("General"),
        {wxExItem(_("Header"), ITEM_CHECKBOX),
         wxExItem(_("Single selection"), ITEM_CHECKBOX),
         wxExItem(_("Comparator"), ITEM_FILEPICKERCTRL),
         wxExItem(_("Sort method"), std::map<long, const wxString> {
           {SORT_ASCENDING, _("Sort ascending")},
           {SORT_DESCENDING, _("Sort descending")},
           {SORT_TOGGLE, _("Sort toggle")}}),
         wxExItem(_("Rulers"),  std::map<long, const wxString> {
           {wxLC_HRULES, _("Horizontal rulers")},
           {wxLC_VRULES, _("Vertical rulers")}}, false)}},
      {_("Font"),
        {wxExItem(_("List font"), ITEM_FONTPICKERCTRL),
         wxExItem(_("List tab font"), ITEM_FONTPICKERCTRL)}},
      {_("Colour"),
        {wxExItem(_("Background colour"), ITEM_COLOURPICKERWIDGET),
         wxExItem(_("Foreground colour"), ITEM_COLOURPICKERWIDGET),
         wxExItem(_("Readonly colour"), ITEM_COLOURPICKERWIDGET)}}}, ITEM_NOTEBOOK_LIST)};
  
  if (button_flags & wxAPPLY)
  {
    if (m_ConfigDialog == nullptr)
    {
      m_ConfigDialog = new wxExItemDialog(parent, items, title, 0, 1, button_flags, id);
    }
    
    return m_ConfigDialog->Show();
  }
  else
  {
    return wxExItemDialog(parent, items, title, 0, 1, button_flags, id).ShowModal();
  }
}
          
void wxExListView::ConfigGet(bool init)
{
  ListViewDefaults use;
  wxConfigBase* cfg = use.Get();
  
  SetBackgroundColour(cfg->ReadObject(_("Background colour"), wxColour("WHITE")));
  SetFont(cfg->ReadObject(_("List font"), wxSystemSettings::GetFont(wxSYS_DEFAULT_GUI_FONT)));
  SetSingleStyle(wxLC_HRULES, (cfg->ReadLong(_("Rulers"), 0) & wxLC_HRULES) > 0);
  SetSingleStyle(wxLC_VRULES, (cfg->ReadLong(_("Rulers"), 0) & wxLC_VRULES) > 0);
  SetSingleStyle(wxLC_NO_HEADER, !cfg->ReadBool(_("Header"), false));
  SetSingleStyle(wxLC_SINGLE_SEL, cfg->ReadBool(_("Single selection"), false));
  
  ItemsUpdate();
}
  
void wxExListView::CopySelectedItemsToClipboard()
{
  if (GetSelectedItemCount() == 0) return;

  wxBusyCursor wait;
  wxString clipboard;

  for (long i = GetFirstSelected(); i != -1; i = GetNextSelected(i))
    clipboard += ItemToText(i) + wxTextFile::GetEOL();
    
  wxExClipboardAdd(clipboard);
}

void wxExListView::EditClearAll()
{
  DeleteAllItems();

  SortColumnReset();

#if wxUSE_STATUSBAR
  wxExFrame::UpdateStatusBar(this);
#endif
}

void wxExListView::EditDelete()
{
  if (GetSelectedItemCount() == 0) return;

  long i = -1, old_item = -1;

  while ((i = GetNextSelected(i)) != -1)
  {
    DeleteItem(i);
    old_item = i;
    i = -1;
  }

  if (old_item != -1 && old_item < GetItemCount())
    SetItemState(old_item, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);

  ItemsUpdate();
}
  
int wxExListView::FindColumn(const wxString& name) const
{
  return Column(name).GetColumn();
}

bool wxExListView::FindNext(const wxString& text, bool find_next)
{
  if (text.empty())
  {
    return false;
  }

  static bool recursive = false;
  static long start_item;
  static long end_item;

  wxString text_use = text;

  if (!wxExFindReplaceData::Get()->MatchCase())
  {
    text_use.MakeUpper();
  }

  const int firstselected = GetFirstSelected();

  if (find_next)
  {
    if (recursive)
    {
      start_item = 0;
    }
    else
    {
      start_item = (firstselected != -1 ? firstselected + 1: 0);
    }

    end_item = GetItemCount();
  }
  else
  {
    if (recursive)
    {
      start_item = GetItemCount() - 1;
    }
    else
    {
      start_item = (firstselected != -1 ? firstselected - 1: 0);
    }

    end_item = -1;
  }

  int match = -1;

  for (
    int index = start_item;
    index != end_item && match == -1;
    (find_next ? index++: index--))
  {
    for (int col = 0; col < GetColumnCount() && match == -1; col++)
    {
      wxString text = wxListView::GetItemText(index, col);

      if (!wxExFindReplaceData::Get()->MatchCase())
      {
        text.MakeUpper();
      }

      if (wxExFindReplaceData::Get()->MatchWord())
      {
        if (text == text_use)
        {
          match = index;
        }
      }
      else
      {
        if (text.Contains(text_use))
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
    wxExFrame::StatusText(wxExGetFindResult(text, find_next, recursive), wxEmptyString);
    
    if (!recursive)
    {
      recursive = true;
      FindNext(text, find_next);
      recursive = false;
    }
    
    return false;
  }
}

unsigned int wxExListView::GetArtID(const wxArtID& artid)
{
  const auto it = m_ArtIDs.find(artid);

  if (it != m_ArtIDs.end())
  {
    return it->second;
  }
  else
  {
    wxImageList* il = GetImageList(wxIMAGE_LIST_SMALL);
    
    if (il == nullptr)
    {
      wxFAIL;
      return 0;
    }

    m_ArtIDs.insert({artid, il->GetImageCount()});

    return il->Add(wxArtProvider::GetBitmap(
      artid, wxART_OTHER, wxSize(m_ImageWidth, m_ImageHeight)));
  }
}

const wxString wxExListView::GetItemText(long item_number, const wxString& col_name) const 
{
  if (col_name.empty())
  {
    return wxListView::GetItemText(item_number);
  }
  
  const int col = FindColumn(col_name);
  return col < 0 ? wxString(wxEmptyString): wxListView::GetItemText(item_number, col);
}

const wxString wxExListView::GetTypeDescription(wxExListType type)
{
  wxString value;

  switch (type)
  {
    case LIST_FOLDER: value = _("Folder"); break;
    case LIST_FIND: value = _("Find Results"); break;
    case LIST_HISTORY: value = _("History"); break;
    case LIST_KEYWORD: value = _("Keywords"); break;
    case LIST_FILE: value = _("File"); break;
    case LIST_REPLACE: value = _("Replace Results"); break;
    case LIST_NONE: value = _("None"); break;
    default: wxFAIL;
  }

  return value;
}

void wxExListView::Initialize(const wxExLexer* lexer)
{
  ConfigGet(true);
  
  SetName(GetTypeDescription());
  
  switch (m_Type)
  {
    case LIST_FOLDER:
    case LIST_NONE:
      SetSingleStyle(wxLC_LIST);
      break;
    
    case LIST_KEYWORD:
      wxASSERT(lexer != nullptr);
      SetName(GetName() + " " + lexer->GetDisplayLexer());
      // fall through
    default:
      SetSingleStyle(wxLC_REPORT);
      AddColumns(lexer);
      break;
  }
}

void wxExListView::ItemActivated(long item_number)
{
  wxASSERT(item_number >= 0);
  
  if (m_Type == LIST_FOLDER)
  {
    wxDirDialog dir_dlg(
      this,
      _(wxDirSelectorPromptStr),
      wxListView::GetItemText(item_number),
      wxDD_DEFAULT_STYLE | wxDD_DIR_MUST_EXIST);

    if (dir_dlg.ShowModal() == wxID_OK)
    {
      SetItemText(item_number, dir_dlg.GetPath());
      wxExListItem(this, item_number).Update();
    }
  }
  else
  {
    // Cannot be const because of SetItem later on.
    wxExListItem item(this, item_number);
  
    if (item.GetFileName().FileExists())
    {
      wxExFrame* frame = dynamic_cast<wxExFrame*>(wxTheApp->GetTopWindow());
      if (frame != nullptr)
      {
        const wxString line_number_str = GetItemText(item_number, _("Line No"));
        const int line_number = atoi(line_number_str.c_str());
        const wxString match =
          (m_Type == LIST_REPLACE ?
             GetItemText(item_number, _("Replaced")):
             GetItemText(item_number, _("Match")));

        frame->OpenFile(
          item.GetFileName().GetFullPath(),
          line_number, 
          match);
      }
    }
    else if (wxDirExists(item.GetFileName().GetFullPath()))
    {
      wxTextEntryDialog dlg(this,
        _("Input") + ":",
        _("Folder Type"),
        GetItemText(item_number, _("Type")));
  
      if (dlg.ShowModal() == wxID_OK)
      {
        item.SetItem(_("Type"), dlg.GetValue());
      }
    }
    
  }
}

bool wxExListView::ItemFromText(const wxString& text)
{
  if (text.empty())
  {
    return false;
  }

  bool modified = false;
  
  wxStringTokenizer tkz(text, wxTextFile::GetEOL());

  while (tkz.HasMoreTokens())
  {
    modified = true;
    
    if (m_Type != LIST_NONE)
    {
      if (!InReportView())
      {
        wxExListItem(this, tkz.GetNextToken()).Insert();
      }
      else
      {
        const wxString token(tkz.GetNextToken());
        wxStringTokenizer tk(token, GetFieldSeparator());
        
        if (tk.HasMoreTokens())
        {
          const wxString value = tk.GetNextToken();
          wxFileName fn(value);
    
          if (fn.FileExists())
          {
            wxExListItem item(this, fn);
            item.Insert();
    
            // And try to set the rest of the columns 
            // (that are not already set by inserting).
            int col = 1;
            while (tk.HasMoreTokens() && col < GetColumnCount() - 1)
            {
              const wxString value = tk.GetNextToken();
    
              if (col != FindColumn(_("Type")) &&
                  col != FindColumn(_("In Folder")) &&
                  col != FindColumn(_("Size")) &&
                  col != FindColumn(_("Modified")))
              {
                SetItem(item.GetId(), col, value);
              }
    
              col++;
            }
          }
          else
          {
            // Now we need only the first column (containing findfiles). If more
            // columns are present, these are ignored.
            const wxString findfiles =
              (tk.HasMoreTokens() ? tk.GetNextToken(): tk.GetString());
    
            wxExListItem(this, value, findfiles).Insert();
          }
        }
        else
        {
          wxExListItem(this, token).Insert();
        }
      }
    }
    else
    {
      const wxString line = tkz.GetNextToken();
      
      wxStringTokenizer tkz(line, m_FieldSeparator);
      
      if (tkz.HasMoreTokens())
      {
        const wxString value = tkz.GetNextToken();

        InsertItem(GetItemCount(), value);

        // And set the rest of the columns.
        int col = 1;
        while (tkz.HasMoreTokens() && col < GetColumnCount())
        {
          SetItem(GetItemCount(), col, tkz.GetNextToken());
          col++;
        }
      }
      else
      {
        InsertItem(GetItemCount(), line);
      }
    }
  }

  return modified;
}

const wxString wxExListView::ItemToText(long item_number) const
{
  wxString text;
    
  if (item_number == -1)
  {
    for (long i = 0; i < GetItemCount(); i++)
    {
      text += wxListView::GetItemText(i) + wxTextFile::GetEOL();
    }
    
    return text;
  }

  switch (m_Type)
  {
    case LIST_FILE:
    case LIST_HISTORY:
      {
      const wxExListItem item(const_cast< wxExListView * >(this), item_number);
      wxString text = (item.GetFileName().GetStat().IsOk() ? 
        item.GetFileName().GetFullPath(): 
        item.GetFileName().GetFullName());

      if (item.GetFileName().DirExists() && !item.GetFileName().FileExists())
      {
        text += GetFieldSeparator() + GetItemText(item_number, _("Type"));
      }
      }

    case LIST_FOLDER:
      return wxListView::GetItemText(item_number);
      break;
    
    default:
      for (int col = 0; col < GetColumnCount(); col++)
      {
        text += wxListView::GetItemText(item_number, col);

        if (col < GetColumnCount() - 1)
        {
          text += m_FieldSeparator;
        }
      }
    }

  return text.Trim();
}

void wxExListView::ItemsUpdate()
{
  if (m_Type != LIST_NONE)
  {
    for (long i = 0; i < GetItemCount(); i++)
    {
      wxExListItem(this, i).Update();
    }
  }
}

void wxExListView::Print()
{
#if wxUSE_HTML & wxUSE_PRINTING_ARCHITECTURE
  wxBusyCursor wait;
  wxExPrinting::Get()->GetHtmlPrinter()->PrintText(BuildPage());
#endif
}

void wxExListView::PrintPreview()
{
#if wxUSE_HTML & wxUSE_PRINTING_ARCHITECTURE
  wxBusyCursor wait;
  wxExPrinting::Get()->GetHtmlPrinter()->PreviewText(BuildPage());
#endif
}

std::vector<wxString>* pitems;

int wxCALLBACK CompareFunctionCB(wxIntPtr item1, wxIntPtr item2, wxIntPtr sortData)
{
  const bool ascending = (sortData > 0);
  const wxExColumn::wxExColumnType type = 
    (wxExColumn::wxExColumnType)std::abs(sortData);

  switch (type)
  {
  case wxExColumn::COL_DATE:
    if (ascending) return (unsigned long)item1 > (unsigned long)item2;
    else           return (unsigned long)item1 < (unsigned long)item2;
  break;

  case wxExColumn::COL_INT:
  case wxExColumn::COL_FLOAT:
    if (ascending) return item1 > item2;
    else           return item1 < item2;
  break;

  case wxExColumn::COL_STRING:
    {
    const wxString& str1 = (*pitems)[item1];
    const wxString& str2 = (*pitems)[item2];

    if (!wxExFindReplaceData::Get()->MatchCase())
    {
      if (ascending) return strcmp(str1.Upper(), str2.Upper());
      else           return strcmp(str2.Upper(), str1.Upper());
    }
    else
    {
      if (ascending) return strcmp(str1.c_str(), str2.c_str());
      else           return strcmp(str2.c_str(), str1.c_str());
    }
    }
  break;

  default:
    wxFAIL;
  }

  return 0;
}

bool wxExListView::SortColumn(int column_no, wxExSortType sort_method)
{
  if (column_no == -1 || column_no >= (int)m_Columns.size())
  {
    return false;
  }
  
  SortColumnReset();
  
  wxExColumn& sorted_col = m_Columns[column_no];
  
  sorted_col.SetIsSortedAscending(sort_method);

  wxBusyCursor wait;

  // Keeping the items is necessary for sorting strings.
  std::vector<wxString> items;
  pitems = &items;

  for (int i = 0; i < GetItemCount(); i++)
  {
    const wxString val = wxListView::GetItemText(i, column_no);
    items.emplace_back(val);

    switch (sorted_col.GetType())
    {
      case wxExColumn::COL_DATE:
        if (!val.empty())
        {
          wxDateTime dt;

          if (!dt.ParseISOCombined(val, ' '))
          {
            return false;
          }
          else
          {
            SetItemData(i, dt.GetTicks());
          }
        }
        else
        {
          SetItemData(i, 0);
        }
      break;

      case wxExColumn::COL_FLOAT: SetItemData(i, (long)atof(val.c_str())); break;
      case wxExColumn::COL_INT: SetItemData(i, atoi(val.c_str())); break;
      case wxExColumn::COL_STRING: SetItemData(i, i); break;
      default: wxFAIL;
    }
  }

  const wxIntPtr sortdata =
    (sorted_col.GetIsSortedAscending() ?
       sorted_col.GetType():
      (0 - sorted_col.GetType()));

  SortItems(CompareFunctionCB, sortdata);

  m_SortedColumnNo = column_no;

  if (m_ImageType != IMAGE_NONE)
  {
    SetColumnImage(column_no, GetArtID(
      sorted_col.GetIsSortedAscending() ? wxART_GO_DOWN: wxART_GO_UP));
  }

  if (GetItemCount() > 0)
  {
    ItemsUpdate();
    AfterSorting();
  }

  wxLogStatus(_("Sorted on") + ": " + sorted_col.GetText());
  
  return true;
}

void wxExListView::SortColumnReset()
{
  if (m_SortedColumnNo != -1 && !m_ArtIDs.empty()) // only if we are using images
  {
    ClearColumnImage(m_SortedColumnNo);
    m_SortedColumnNo = -1;
  }
}
#endif // wxUSE_GUI
