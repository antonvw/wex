/******************************************************************************\
* File:          listview.cpp
* Purpose:       Implementation of wxExListView and related classes
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/dnd.h> 
#include <wx/numdlg.h> // for wxGetNumberFromUser
#include <wx/generic/dirctrlg.h> // for wxTheFileIconsTable
#include <wx/imaglist.h>
#include <wx/textfile.h> // for wxTextFile::GetEOL()
#include <wx/tokenzr.h>
#include <wx/extension/listview.h>
#include <wx/extension/frame.h>
#include <wx/extension/frd.h>
#include <wx/extension/printing.h>
#include <wx/extension/util.h>

#if wxUSE_GUI

#if wxUSE_DRAG_AND_DROP
class ListViewDropTarget : public wxTextDropTarget
{
public:
  ListViewDropTarget(wxExListView* owner) {m_Owner = owner;}
private:
  virtual bool OnDropText(
    wxCoord x, 
    wxCoord y, 
    const wxString& data) {
    bool modified = false;
    wxStringTokenizer tkz(data, wxTextFile::GetEOL());

    while (tkz.HasMoreTokens())
    {
      if (m_Owner->ItemFromText(tkz.GetNextToken()))
      {
        modified = true;
      }
    }

    if (
      modified && 
      wxConfigBase::Get()->ReadBool("List/SortSync", true))
    {
      m_Owner->SortColumn(_("Modified"), SORT_KEEP);
    }

    return true;} 
  wxExListView* m_Owner;
};
#endif

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

  SetColumn(-1); // default value, should be overriden when inserting the col
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

const int ID_COL_FIRST = 1000;
const int ID_COL_LAST = ID_COL_FIRST + 255;

BEGIN_EVENT_TABLE(wxExListView, wxListView)
  EVT_SET_FOCUS(wxExListView::OnFocus)
  EVT_KILL_FOCUS(wxExListView::OnFocus)
  EVT_LIST_BEGIN_DRAG(wxID_ANY, wxExListView::OnList)
  EVT_LIST_COL_CLICK(wxID_ANY, wxExListView::OnList)
  EVT_LIST_COL_RIGHT_CLICK(wxID_ANY, wxExListView::OnList)
  EVT_LIST_ITEM_DESELECTED(wxID_ANY, wxExListView::OnList)
  EVT_LIST_ITEM_SELECTED(wxID_ANY, wxExListView::OnList)
  EVT_MENU(wxID_DELETE, wxExListView::OnCommand)
  EVT_MENU(wxID_SELECTALL, wxExListView::OnCommand)
  EVT_MENU(wxID_SORT_ASCENDING, wxExListView::OnCommand)
  EVT_MENU(wxID_SORT_DESCENDING, wxExListView::OnCommand)
  EVT_MENU(ID_EDIT_SELECT_INVERT, wxExListView::OnCommand)
  EVT_MENU(ID_EDIT_SELECT_NONE, wxExListView::OnCommand)
  EVT_MENU_RANGE(wxID_CUT, wxID_CLEAR, wxExListView::OnCommand)
  EVT_MENU_RANGE(ID_COL_FIRST, ID_COL_LAST, wxExListView::OnCommand)
  EVT_RIGHT_DOWN(wxExListView::OnMouse)
  EVT_SHOW(wxExListView::OnShow)
END_EVENT_TABLE()

wxExListView::wxExListView(wxWindow* parent,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size,
  long style,
  wxExImageType image_type,
  const wxValidator& validator,
  const wxString &name)
  : wxListView(parent, id, pos, size, style, validator, name)
  , m_FieldSeparator('\t')
  , m_ImageType(image_type)
  , m_ImageHeight(16)
  , m_ImageWidth(16)
  , m_SortedColumnNo(-1)
{
  SetSingleStyle(wxLC_REPORT);

#if wxUSE_DRAG_AND_DROP
  SetDropTarget(new ListViewDropTarget(this));
#endif

  if (image_type != IMAGE_NONE)
  {
    if (image_type == IMAGE_ART || image_type == IMAGE_OWN)
    {
      AssignImageList(
        new wxImageList(
          m_ImageWidth, 
          m_ImageHeight, true, 0), 
        wxIMAGE_LIST_SMALL);
    }
    else if (image_type == IMAGE_FILE_ICON)
    {
      SetImageList(
        wxTheFileIconsTable->GetSmallImageList(), 
        wxIMAGE_LIST_SMALL);
    }
    else
    {
      wxFAIL;
    }
  }

  SetFont(wxConfigBase::Get()->ReadObject(
    _("List Font"), wxSystemSettings::GetFont(wxSYS_OEM_FIXED_FONT)));

  wxAcceleratorEntry entries[4];

  entries[0].Set(wxACCEL_NORMAL, WXK_DELETE, wxID_DELETE);
  entries[1].Set(wxACCEL_CTRL, WXK_INSERT, wxID_COPY);
  entries[2].Set(wxACCEL_SHIFT, WXK_INSERT, wxID_PASTE);
  entries[3].Set(wxACCEL_SHIFT, WXK_DELETE, wxID_CUT);

  wxAcceleratorTable accel(WXSIZEOF(entries), entries);
  SetAcceleratorTable(accel);
}

wxExListView::~wxExListView()
{
}

const wxString wxExListView::BuildPage()
{
  wxString text;

  text << "<TABLE ";

  if ((GetWindowStyle() & wxLC_HRULES) ||
      (GetWindowStyle() & wxLC_VRULES))
    text << "border=1";
  else
    text << "border=0";

  text << " cellpadding=4 cellspacing=0 >" << wxTextFile::GetEOL();

  text << "<tr>" << wxTextFile::GetEOL();

  for (auto c = 0; c < GetColumnCount(); c++)
  {
    wxListItem col;
    GetColumn(c, col);
    text << "<td><i>" << col.GetText() << "</i>" << wxTextFile::GetEOL();
  }

  for (auto i = 0; i < GetItemCount(); i++)
  {
    text << "<tr>" << wxTextFile::GetEOL();

    for (auto col = 0; col < GetColumnCount(); col++)
    {
      text << "<td>" << GetItemText(i, col) << wxTextFile::GetEOL();
    }
  }

  text << "</TABLE>" << wxTextFile::GetEOL();

  return text;
}

void wxExListView::BuildPopupMenu(wxExMenu& menu)
{
  if (GetItemCount() > 0)
  {
    menu.AppendSeparator();
  }

  menu.AppendEdit(true);

  if (GetItemCount() > 0 && GetSelectedItemCount() == 0)
  {
    menu.AppendSeparator();
    menu.Append(wxID_FIND);
    menu.AppendSeparator();

    wxMenu* menuSort = new wxMenu;
    
    for (auto i = 0; i < GetColumnCount(); i++)
    {
      wxListItem item;
      item.SetMask(wxLIST_MASK_TEXT);
      GetColumn(i, item);  
      menuSort->Append(ID_COL_FIRST + i, item.GetText());
    }

    menu.AppendSubMenu(menuSort, _("Sort On"));
  }
}

void wxExListView::CopySelectedItemsToClipboard()
{
  if (GetSelectedItemCount() == 0) return;

  wxBusyCursor wait;
  wxString clipboard;

  long i = -1;
  while ((i = GetNextSelected(i)) != -1)
  {
    clipboard += ItemToText(i) + wxTextFile::GetEOL();
  }

  wxExClipboardAdd(clipboard);
}

void wxExListView::EditClearAll()
{
  DeleteAllItems();

#if wxUSE_STATUSBAR
  UpdateStatusBar();
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
  for (auto i = 0; i < GetColumnCount(); i++)
  {
    wxListItem item;
    item.SetMask(wxLIST_MASK_TEXT);
    GetColumn(i, item);

    if (item.GetText() == name)
    {
      return i;
    }
  }

  return -1;
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

  const auto firstselected = GetFirstSelected();

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
    auto index = start_item;
    index != end_item && match == -1;
    (find_next ? index++: index--))
  {
    for (auto col = 0; col < GetColumnCount() && match == -1; col++)
    {
      wxString text = GetItemText(index, col);

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
    wxExFindResult(text, find_next, recursive);
    
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
  if (GetImageList(wxIMAGE_LIST_SMALL) == NULL ||
      m_ImageType != IMAGE_ART)
  {
    wxFAIL;
    return 0;
  }

  const auto it = m_ArtIDs.find(artid);

  if (it != m_ArtIDs.end())
  {
    return it->second;
  }
  else
  {
    m_ArtIDs.insert(std::make_pair(artid, 
      GetImageList(wxIMAGE_LIST_SMALL)->GetImageCount()));

    const wxSize smallsize(m_ImageWidth, m_ImageHeight);

    return GetImageList(wxIMAGE_LIST_SMALL)->Add(
      wxArtProvider::GetBitmap(artid, wxART_OTHER, smallsize));
  }
}

const wxString wxExListView::GetItemText(
  long item_number,
  int col_number) const
{
  if (col_number < 0) 
  {
    // We cannot wxFAIL here, as e.g. "Line No" column is used
    // to get line number, in several lists, als in lists without that column.
    return wxEmptyString;
  }

  wxListItem item;

  item.SetId(item_number);
  item.SetColumn(col_number);
  item.SetMask(wxLIST_MASK_TEXT);

  if (!GetItem(item))
  {
    wxFAIL;
    return wxEmptyString;
  }

  return item.GetText();
}

bool wxExListView::GotoDialog(const wxString& caption)
{
  auto initial_value = GetFirstSelected();

  if (initial_value == -1) // nothing selected
  {
    initial_value = 1;
  }
  else
  {
    initial_value += 1;
  }

  long val;

  if ((val = wxGetNumberFromUser(
    _("Input") + wxString::Format(" (1 - %d):", GetItemCount()),
    wxEmptyString,
    caption,
    initial_value,
    1,
    GetItemCount())) < 0)
  {
    return false;
  }

  if (initial_value >= 1)
  {
    Select(initial_value - 1, false);
  }

  Select(val - 1);
  EnsureVisible(val - 1);

  return true;
}

void wxExListView::InsertColumn(const wxExColumn& col)
{
  wxExColumn mycol(col);
  
  wxListView::InsertColumn(GetColumnCount(), mycol);
  mycol.SetColumn(GetColumnCount() - 1);
  m_Columns.push_back(mycol);
}

bool wxExListView::ItemFromText(const wxString& text)
{
  if (text.empty())
  {
    return false;
  }

  wxStringTokenizer tkz(text, m_FieldSeparator);
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
    InsertItem(GetItemCount(), text);
  }

  return true;
}

const wxString wxExListView::ItemToText(long item_number) const
{
  wxString text;

  for (auto col = 0; col < GetColumnCount(); col++)
  {
    text += GetItemText(item_number, col);

    if (col < GetColumnCount() - 1)
    {
      text += m_FieldSeparator;
    }
  }

  return text;
}

void wxExListView::OnCommand(wxCommandEvent& event)
{
  if (event.GetId() >= ID_COL_FIRST && event.GetId() <= ID_COL_LAST)
  {
    SortColumn(event.GetId() - ID_COL_FIRST, SORT_TOGGLE);
    return;
  }

  switch (event.GetId())
  {
  case wxID_CLEAR:
    EditClearAll();
  break;
  case wxID_CUT:
    CopySelectedItemsToClipboard();
    EditDelete();
    break;
  case wxID_COPY:
    CopySelectedItemsToClipboard();
    break;
  case wxID_DELETE:
    EditDelete();
    break;
  case wxID_PASTE:
    PasteItemsFromClipboard();
    break;
  case wxID_SELECTALL:
    EditSelectAll();
    break;
  case wxID_SORT_ASCENDING:
    SortColumn(m_ToBeSortedColumnNo, SORT_ASCENDING);
    break;
  case wxID_SORT_DESCENDING:
    SortColumn(m_ToBeSortedColumnNo, SORT_DESCENDING);
    break;
  case ID_EDIT_SELECT_INVERT:
    EditInvertAll();
    break;
  case ID_EDIT_SELECT_NONE:
    for (auto i = 0; i < GetItemCount(); i++)
    {
      Select(i, false);
    }
    break;
  default: wxFAIL;
  }

#if wxUSE_STATUSBAR
  UpdateStatusBar();
#endif
}

void wxExListView::OnFocus(wxFocusEvent& event)
{
  event.Skip();

  wxCommandEvent focusevent(wxEVT_COMMAND_MENU_SELECTED, ID_FOCUS_LISTVIEW);

  if (event.GetEventType() == wxEVT_SET_FOCUS)
  {
    focusevent.SetEventObject(this);
  }
  else
  {
    focusevent.SetEventObject(NULL);
  }
}

void wxExListView::OnList(wxListEvent& event)
{
  if (event.GetEventType() == wxEVT_COMMAND_LIST_COL_CLICK)
  {
    SortColumn(
      event.GetColumn(),
      (wxExSortType)wxConfigBase::Get()->ReadLong("List/SortMethod", 
         SORT_TOGGLE));
  }
  else if (event.GetEventType() == wxEVT_COMMAND_LIST_COL_RIGHT_CLICK)
  {
    m_ToBeSortedColumnNo = event.GetColumn();

    wxExMenu menu(GetSelectedItemCount() > 0 ? 
      wxExMenu::MENU_IS_SELECTED: 
      wxExMenu::MENU_DEFAULT);
      
    menu.Append(wxID_SORT_ASCENDING);
    menu.Append(wxID_SORT_DESCENDING);

    PopupMenu(&menu);
  }
  else if (event.GetEventType() == wxEVT_COMMAND_LIST_ITEM_DESELECTED)
  {
#if wxUSE_STATUSBAR
    UpdateStatusBar();
#endif
  }
  else if (event.GetEventType() == wxEVT_COMMAND_LIST_ITEM_SELECTED)
  {
#if wxUSE_STATUSBAR
    UpdateStatusBar();
#endif
  }
  else if (event.GetEventType() == wxEVT_COMMAND_LIST_BEGIN_DRAG)
  {
#if wxUSE_DRAG_AND_DROP
    // Start drag operation.
    wxString text;

    long i = -1;
    while ((i = GetNextSelected(i)) != -1)
    {
      text += ItemToText(i) + wxTextFile::GetEOL();
    }

    if (!text.empty())
    {
      wxTextDataObject textData(text);
      wxDropSource source(textData, this);
      wxDragResult result = source.DoDragDrop(wxDragCopy);

      if (result != wxDragError &&
          result != wxDragNone &&
           result != wxDragCancel)
      {
      }
    }
#endif
  }
}

void wxExListView::OnMouse(wxMouseEvent& event)
{
  if (event.RightDown())
  {
    int style = wxExMenu::MENU_DEFAULT;
    if (GetSelectedItemCount() > 0) style |= wxExMenu::MENU_IS_SELECTED;
    if (GetItemCount() == 0) style |= wxExMenu::MENU_IS_EMPTY;
    if (GetSelectedItemCount() == 0 && GetItemCount() > 0) 
    {
      style |= wxExMenu::MENU_ALLOW_CLEAR;
    }

    wxExMenu menu(style);

    BuildPopupMenu(menu);

    if (menu.GetMenuItemCount() > 0)
    {
      PopupMenu(&menu);
    }
  }
}

void wxExListView::OnShow(wxShowEvent& event)
{
  event.Skip();
  UpdateStatusBar();
}

void wxExListView::PasteItemsFromClipboard()
{
  wxStringTokenizer tkz(wxExClipboardGet(), wxTextFile::GetEOL());

  while (tkz.HasMoreTokens())
  {
    ItemFromText(tkz.GetNextToken());
  }

  if (wxConfigBase::Get()->ReadBool("List/SortSync", true))
  {
    SortColumn(_("Modified"), SORT_KEEP);
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

int wxCALLBACK CompareFunctionCB(long item1, long item2, wxIntPtr sortData)
{
  const bool ascending = (sortData > 0);
  const wxExColumn::wxExColumnType type = 
    (wxExColumn::wxExColumnType)abs(sortData);

  switch (type)
  {
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

  case wxExColumn::COL_DATE:
    if (ascending) return (unsigned long)item1 > (unsigned long)item2;
    else           return (unsigned long)item1 < (unsigned long)item2;
  break;

  case wxExColumn::COL_INT:
  case wxExColumn::COL_FLOAT:
    if (ascending) return item1 > item2;
    else           return item1 < item2;
  break;

  default:
    wxFAIL;
  }

  return 0;
}

void wxExListView::SortColumn(int column_no, wxExSortType sort_method)
{
  if (column_no == -1)
  {
    return;
  }

  SortColumnReset();

  wxExColumn* sorted_col = &m_Columns[column_no];
  sorted_col->SetIsSortedAscending(sort_method);

  wxBusyCursor wait;

  // Keeping the items is necessary for sorting strings.
  std::vector<wxString> items;
  pitems = &items;

  for (auto i = 0; i < GetItemCount(); i++)
  {
    const wxString val = GetItemText(i, column_no);
    items.push_back(val);

    switch (sorted_col->GetType())
    {
    case wxExColumn::COL_INT: 
    SetItemData(i, atoi(val.c_str())); 
    break;

    case wxExColumn::COL_FLOAT: 
    SetItemData(i, (long)atof(val.c_str())); 
    break;

    case wxExColumn::COL_DATE:
      if (!val.empty())
      {
        wxDateTime dt;

        if (!dt.ParseISOCombined(val, ' '))
        {
          SetItemData(i, 0);
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

    case wxExColumn::COL_STRING: 
      SetItemData(i, i); 
    break;
    default: 
      wxFAIL;
    }
  }

  const wxIntPtr sortdata =
    (sorted_col->GetIsSortedAscending() ?
       sorted_col->GetType():
      (0 - sorted_col->GetType()));

  SortItems(CompareFunctionCB, sortdata);

  m_SortedColumnNo = column_no;

  // Only use an image if the list items have images as well.
  // Otherwise the list items get a sorting image as well.
  if (!m_ArtIDs.empty())
  {
    SetColumnImage(column_no,
      GetArtID(sorted_col->GetIsSortedAscending() ? wxART_GO_DOWN: wxART_GO_UP));
  }

  if (GetItemCount() > 0)
  {
    ItemsUpdate();
    AfterSorting();
  }

#if wxUSE_STATUSBAR
  wxExFrame::StatusText(_("Sorted on") + ": " + sorted_col->GetText());
#endif
}

void wxExListView::SortColumnReset()
{
  if (m_SortedColumnNo != -1 && !m_ArtIDs.empty()) // only if we are using images
  {
    ClearColumnImage(m_SortedColumnNo);
  }

  m_SortedColumnNo = -1;
}

#if wxUSE_STATUSBAR
void wxExListView::UpdateStatusBar() const
{
  if (IsShown())
  {
    const wxString text = (GetSelectedItemCount() == 0 ?
      wxString::Format("%d", GetItemCount()):
      wxString::Format("%d,%d", GetItemCount(), GetSelectedItemCount()));
      
    wxExFrame::StatusText(text, "PaneItems");
  }
  else
  {
    wxExFrame::StatusText(wxEmptyString, "PaneItems");
  }
}
#endif

#endif // wxUSE_GUI
