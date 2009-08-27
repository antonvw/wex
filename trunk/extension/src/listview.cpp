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

#include <wx/numdlg.h> // for wxGetNumberFromUser
#include <wx/stockitem.h> // for wxGetStockLabel
#include <wx/generic/dirctrlg.h> // for wxTheFileIconsTable
#include <wx/textfile.h> // for wxTextFile::GetEOL()
#include <wx/tokenzr.h>
#include <wx/extension/listview.h>
#include <wx/extension/app.h>

#if wxUSE_GUI

using namespace std;

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

wxExListItem::wxExListItem(wxExListView* lv, const wxString& text)
  : m_ListView(lv)
{
  SetId(m_ListView->GetItemCount());
  SetMask(wxLIST_MASK_TEXT);
  SetText(text);
}

wxExListItem::wxExListItem(wxExListView* lv, const int itemnumber)
  : m_ListView(lv)
{
  SetId(itemnumber);
}

const wxString wxExListItem::GetColumnText(int col_no)
{
  if (col_no == -1) return wxEmptyString;

  SetColumn(col_no);
  SetMask(wxLIST_MASK_TEXT);

  if (!m_ListView->GetItem(*this))
  {
    wxFAIL;
    return wxEmptyString;
  }

  return GetText();
}

void wxExListItem::SetColumnText(const int col_no, const wxString& text)
{
  if (col_no >= m_ListView->GetColumnCount())
  {
    wxFAIL;
    return;
  }

#ifdef __WXDEBUG__
  // Readme: 512 should be a constant from the wx lib.
  if (text.length() >= 512)
  {
#if wxUSE_STATUSBAR
    wxExFrame::StatusText(
      "Warning, column max size is 512, column text: ..." +
        text.substr(text.length() - 25) + " ignored");
#endif
  }
#endif

  SetColumn(col_no);
  SetMask(wxLIST_MASK_TEXT);
  SetText(text);

  if (!m_ListView->SetItem(*this))
  {
    wxFAIL;
  }
}

void wxExListItem::StoreImage(int image)
{
  wxListItem::SetImage(image);
  SetMask(wxLIST_MASK_IMAGE);
  SetColumn(0);

  if (!m_ListView->SetItem(*this))
  {
    wxFAIL;
  }
}

const int ID_COL_FIRST = 1000;
const int ID_COL_LAST = ID_COL_FIRST + 255;

BEGIN_EVENT_TABLE(wxExListView, wxListView)
  EVT_FIND(wxID_ANY, wxExListView::OnFindDialog)
  EVT_FIND_CLOSE(wxID_ANY, wxExListView::OnFindDialog)
  EVT_FIND_NEXT(wxID_ANY, wxExListView::OnFindDialog)
  EVT_LIST_COL_CLICK(wxID_ANY, wxExListView::OnList)
  EVT_LIST_COL_RIGHT_CLICK(wxID_ANY, wxExListView::OnList)
  EVT_LIST_ITEM_DESELECTED(wxID_ANY, wxExListView::OnList)
  EVT_LIST_ITEM_SELECTED(wxID_ANY, wxExListView::OnList)
  EVT_MENU(wxID_SORT_ASCENDING, wxExListView::OnCommand)
  EVT_MENU(wxID_SORT_DESCENDING, wxExListView::OnCommand)
  EVT_MENU_RANGE(wxID_CUT, wxID_PROPERTIES, wxExListView::OnCommand)
  EVT_MENU_RANGE(ID_EDIT_LOWEST, ID_EDIT_HIGHEST, wxExListView::OnCommand)
  EVT_MENU_RANGE(ID_COL_FIRST, ID_COL_LAST, wxExListView::OnCommand)
  EVT_RIGHT_DOWN(wxExListView::OnMouse)
  EVT_SHOW(wxExListView::OnShow)
END_EVENT_TABLE()

wxExListView::wxExListView(wxWindow* parent,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size,
  long style,
  const wxValidator& validator,
  wxExImageType image_type)
  : wxListView(parent, id, pos, size, style, validator)
  , wxExInterface()
  , m_FieldSeparator('\t')
  , m_ImageType(image_type)
  , m_ImageHeightSmall(16)
  , m_ImageWidthSmall(16)
  , m_ImageHeightLarge(32)
  , m_ImageWidthLarge(32)
  , m_SortedColumnNo(-1)
{
  if (image_type != IMAGE_NONE)
  {
    if (image_type == IMAGE_ART || image_type == IMAGE_OWN)
    {
      AssignImageList(new wxImageList(m_ImageWidthLarge, m_ImageHeightLarge, true, 0), wxIMAGE_LIST_NORMAL);
      AssignImageList(new wxImageList(m_ImageWidthSmall, m_ImageHeightSmall, true, 0), wxIMAGE_LIST_SMALL);
    }
    else if (image_type == IMAGE_FILE_ICON)
    {
      // At least in wxWidgets 2.7.0 there is no large file icons table image list.
      SetImageList(wxTheFileIconsTable->GetSmallImageList(), wxIMAGE_LIST_SMALL);
    }
    else
    {
      wxFAIL;
    }
  }

  wxFont font(
    wxExApp::GetConfig(_("List Font") + "/Size", 8),
    wxFONTFAMILY_DEFAULT,
    wxFONTSTYLE_NORMAL,
    wxFONTWEIGHT_NORMAL,
    false,
    wxExApp::GetConfig(_("List Font") + "/Name"));

  SetFont(font);

  wxAcceleratorEntry entries[7];

  entries[0].Set(wxACCEL_NORMAL, WXK_DELETE, wxID_DELETE);
  entries[1].Set(wxACCEL_CTRL, WXK_INSERT, wxID_COPY);
  entries[2].Set(wxACCEL_SHIFT, WXK_INSERT, wxID_PASTE);
  entries[3].Set(wxACCEL_SHIFT, WXK_DELETE, wxID_CUT);
  entries[4].Set(wxACCEL_NORMAL, WXK_F3, ID_LIST_FIND_NEXT);
  entries[5].Set(wxACCEL_NORMAL, WXK_F4, ID_LIST_FIND_PREVIOUS);
  entries[6].Set(wxACCEL_NORMAL, WXK_F5, ID_LIST_FIND);

  wxAcceleratorTable accel(7, entries);
  SetAcceleratorTable(accel);
}

wxExListView::~wxExListView()
{
}

const wxString wxExListView::BuildPage()
{
#if wxUSE_HTML & wxUSE_PRINTING_ARCHITECTURE
  wxExApp::GetPrinter()->SetFooter(PrintFooter());
  wxExApp::GetPrinter()->SetHeader(PrintHeader());
#endif

  wxString text;

  text << "<TABLE ";

  if ((GetWindowStyle() & wxLC_HRULES) ||
      (GetWindowStyle() & wxLC_VRULES))
    text << "border=1";
  else
    text << "border=0";

  text << " cellpadding=4 cellspacing=0 >" << wxTextFile::GetEOL();

  text << "<tr>" << wxTextFile::GetEOL();

  for (
    vector<wxExColumn>::const_iterator it = m_Columns.begin();
    it != m_Columns.end();
    ++it)
  {
    text << "<td><i>" << it->GetText() << "</i>" << wxTextFile::GetEOL();
  }

  for (long i = 0; i < GetItemCount(); i++)
  {
    text << "<tr>" << wxTextFile::GetEOL();

    wxExListItem item(this, i);

    for (int col = 0; col < GetColumnCount(); col++)
    {
      text << "<td>" << item.GetColumnText(col) << wxTextFile::GetEOL();
    }
  }

  text << "</TABLE>" << wxTextFile::GetEOL();

  return text;
}

void wxExListView::BuildPopupMenu(wxExMenu& menu)
{
  if (GetItemCount() > 0)
  {
    menu.Append(ID_LIST_FIND,
      wxGetStockLabel(wxID_FIND, wxSTOCK_WITH_MNEMONIC | wxSTOCK_WITH_ACCELERATOR),
      wxEmptyString,
      wxART_FIND);

    menu.AppendSeparator();

    if (GetSelectedItemCount() == 0)
    {
      wxMenu* menuSort = new wxMenu;

      int i = ID_COL_FIRST;
      for (
        vector<wxExColumn>::const_iterator it = m_Columns.begin();
        it != m_Columns.end();
        ++it)
      {
        menuSort->Append(i++, it->GetText());
      }

      menu.AppendSubMenu(menuSort, _("Sort On"));
      menu.AppendSeparator();
    }
  }

  menu.AppendEdit(true);
}

void wxExListView::CopySelectedItemsToClipboard()
{
  if (GetSelectedItemCount() == 0) return;

  wxBusyCursor wait;
  wxString clipboard;

  int i = -1;
  while ((i = GetNextSelected(i)) != -1)
  {
    clipboard = clipboard + ItemToText(i) + wxTextFile::GetEOL();
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

  int i = -1, old_item = -1;
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

void wxExListView::EditInvertAll()
{
  for (int i = 0; i < GetItemCount(); i++)
  {
    Select(i, !IsSelected(i));
  }
}

void wxExListView::EditSelectAll()
{
  SetItemState(-1, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
}

long wxExListView::FindColumn(const wxString& name, bool is_required) const
{
  for (
    vector<wxExColumn>::const_iterator it = m_Columns.begin();
    it != m_Columns.end();
    ++it)
  {
    if (it->GetText() == name)
    {
      return it->GetColumnNo();
    }
  }

  if (is_required)
  {
    wxLogError("Column: %s not found", name.c_str());
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
  static int start_item;
  static int end_item;

  wxString text_use = text;

  if (!wxExApp::GetConfig()->GetFindReplaceData()->MatchCase())
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
    wxExListItem item(this, index);

    for (int col = 0; col < GetColumnCount() && match == -1; col++)
    {
      wxString text = item.GetColumnText(col);

      if (!wxExApp::GetConfig()->GetFindReplaceData()->MatchCase())
      {
        text.MakeUpper();
      }

      if (wxExApp::GetConfig()->GetFindReplaceData()->MatchWord())
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
    return FindResult(text, find_next, recursive);
  }
}

unsigned int wxExListView::GetArtID(wxArtID artid)
{
  if (GetImageList(wxIMAGE_LIST_NORMAL) == NULL ||
      GetImageList(wxIMAGE_LIST_SMALL) == NULL ||
      m_ImageType != IMAGE_ART)
  {
    wxFAIL;
    return 0;
  }

  map<wxArtID, unsigned int>::const_iterator it = m_ArtIDs.find(artid);

  if (it != m_ArtIDs.end())
  {
    return it->second;
  }
  else
  {
    m_ArtIDs.insert(make_pair(artid, GetImageList(wxIMAGE_LIST_SMALL)->GetImageCount()));

    const wxSize largesize(m_ImageWidthLarge, m_ImageHeightLarge);
    const wxSize smallsize(m_ImageWidthSmall, m_ImageHeightSmall);

    GetImageList(wxIMAGE_LIST_SMALL)->Add(wxArtProvider::GetBitmap(artid, wxART_OTHER, smallsize));
    return GetImageList(wxIMAGE_LIST_NORMAL)->Add(wxArtProvider::GetBitmap(artid, wxART_OTHER, largesize));
  }
}

const wxExColumn wxExListView::GetColumn(int column_no) const
{
  for (
    vector<wxExColumn>::const_iterator it = m_Columns.begin();
    it != m_Columns.end();
    ++it)
  {
    if (it->GetColumnNo() == column_no)
    {
      return *it;
    }
  }

  return wxExColumn();
}

bool wxExListView::GotoDialog(const wxString& caption)
{
  long initial_value = GetFirstSelected();

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

void wxExListView::InsertColumn(
  const wxString& name,
  wxExColumn::wxExColumnType type,
  int width)
{
  wxExColumn col;

  col.m_Type = type;
  col.m_IsSortedAscending = false;

  wxListColumnFormat align = wxLIST_FORMAT_RIGHT;

  switch (type)
  {
  case wxExColumn::COL_FLOAT: align = wxLIST_FORMAT_RIGHT; if (width == 0) width = 80; break;
  case wxExColumn::COL_INT: align = wxLIST_FORMAT_RIGHT; if (width == 0) width = 80; break;
  case wxExColumn::COL_STRING: align = wxLIST_FORMAT_LEFT;  if (width == 0) width = 100; break;
  case wxExColumn::COL_DATE: align = wxLIST_FORMAT_LEFT;  if (width == 0) width = 150; break;
  default: wxFAIL;
  }

  col.SetText(name);
  col.SetAlign(align);
  col.SetWidth(width);

  wxListView::InsertColumn(GetColumnCount(), col);

  // Using return value of listctrl->InsertColumn does not work for wxGTK!
  col.m_ColumnNo = GetColumnCount() - 1;

  m_Columns.push_back(col);
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

    wxExListItem item(this, value);
    item.Insert();

    // And set the rest of the columns.
    int col = 1;
    while (tkz.HasMoreTokens() && col < GetColumnCount())
    {
      item.SetColumnText(col, tkz.GetNextToken());
      col++;
    }
  }
  else
  {
    wxExListItem item(this, text);
    item.Insert();
  }

  return true;
}

const wxString wxExListView::ItemToText(int item_number)
{
  wxString text;

  wxExListItem item(this, item_number);

  for (int col = 0; col < GetColumnCount(); col++)
  {
    text += item.GetColumnText(col);

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
    for (int i = 0; i < GetItemCount(); i++)
    {
      Select(i, false);
    }
    break;
  case ID_LIST_FIND:
    FindDialog(this);
    break;
  case ID_LIST_FIND_NEXT:
    FindNext(wxExApp::GetConfig()->GetFindReplaceData()->GetFindString());
    break;
  case ID_LIST_FIND_PREVIOUS:
    FindNext(wxExApp::GetConfig()->GetFindReplaceData()->GetFindString(), false);
    break;
  default: wxFAIL;
  }

#if wxUSE_STATUSBAR
  UpdateStatusBar();
#endif
}

void wxExListView::OnFindDialog(wxFindDialogEvent& event)
{
  wxExInterface::OnFindDialog(event);
}

void wxExListView::OnList(wxListEvent& event)
{
  if (event.GetEventType() == wxEVT_COMMAND_LIST_COL_CLICK)
  {
    SortColumn(
      event.GetColumn(),
      (wxExSortType)wxExApp::GetConfig("List/SortMethod", SORT_TOGGLE));
  }
  else if (event.GetEventType() == wxEVT_COMMAND_LIST_COL_RIGHT_CLICK)
  {
    m_ToBeSortedColumnNo = event.GetColumn();

    wxExMenu menu(GetSelectedItemCount() > 0 ? wxExMenu::MENU_IS_SELECTED: wxExMenu::MENU_DEFAULT);
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
}

void wxExListView::OnMouse(wxMouseEvent& event)
{
  if (event.RightDown())
  {
    int style = wxExMenu::MENU_DEFAULT;
    if (GetSelectedItemCount() > 0) style |= wxExMenu::MENU_IS_SELECTED;
    if (GetItemCount() == 0) style |= wxExMenu::MENU_IS_EMPTY;
    if (GetSelectedItemCount() == 0 && GetItemCount() > 0) style |= wxExMenu::MENU_ALLOW_CLEAR;

    wxExMenu menu(style);
    BuildPopupMenu(menu);

    PopupMenu(&menu);
  }
}

void wxExListView::OnShow(wxShowEvent& event)
{
  // Next code does not work if list are part of a notebook,
  // then if you select another page, the old is seems to be hidden the last,
  // and therefore items pane will be empty.
  /* 
  if (event.IsShown())
  {
#if wxUSE_STATUSBAR
    UpdateStatusBar();
#endif
  }
  else
  {
#if wxUSE_STATUSBAR
    wxExFrame::StatusText(wxEmptyString, "PaneItems");
#endif
  }
  */
}

void wxExListView::PasteItemsFromClipboard()
{
  wxStringTokenizer tkz(wxExClipboardGet(), wxTextFile::GetEOL());

  while (tkz.HasMoreTokens())
  {
    ItemFromText(tkz.GetNextToken());
  }

  if (wxExApp::GetConfigBool("List/SortSync"))
  {
    SortColumn(wxExApp::GetConfig("List/SortColumn", FindColumn(_("Modified"))), SORT_KEEP);
  }
}

std::vector<wxString>* pitems;

int wxCALLBACK CompareFunctionCB(long item1, long item2, long sortData)
{
  const bool ascending = (sortData > 0);
  const wxExColumn::wxExColumnType type = (wxExColumn::wxExColumnType)abs(sortData);

  switch (type)
  {
  case wxExColumn::COL_STRING:
    {
    const wxString& str1 = (*pitems)[item1];
    const wxString& str2 = (*pitems)[item2];

    if (!wxExApp::GetConfig()->GetFindReplaceData()->MatchCase())
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

  default:
    if (ascending) return item1 > item2;
    else           return item1 < item2;
  }
}

void wxExListView::SortColumn(int column_no, wxExSortType sort_method)
{
  SortColumnReset();

  wxExColumn* sorted_col = &m_Columns[column_no];
  sorted_col->SetIsSortedAscending(sort_method);

  wxBusyCursor wait;

  // Keeping the items is necessary for sorting strings.
  std::vector<wxString> items;
  pitems = &items;

  for (long i = 0; i < GetItemCount(); i++)
  {
    const wxString val = wxExListItem(this, i).GetColumnText(column_no);
    items.push_back(val);

    switch (sorted_col->GetType())
    {
    case wxExColumn::COL_INT: SetItemData(i, atoi(val.c_str())); break;
    case wxExColumn::COL_FLOAT: SetItemData(i, (long)atof(val.c_str())); break;
    case wxExColumn::COL_DATE:
      if (!val.empty())
      {
        wxDateTime dt;
        wxString::const_iterator end;

        if (!dt.ParseDateTime(val, &end))
        {
          wxLogError("Cannot sort, date not known format");
          return;
        }

        SetItemData(i, dt.GetTicks());
      }
    break;
    default: SetItemData(i, i);
    }
  }

  const long sortdata =
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
void wxExListView::UpdateStatusBar()
{
  const wxString text = wxString::Format("%d", GetItemCount()) +
    wxString((GetSelectedItemCount() > 0) ?
      ("," + wxString::Format("%d", GetSelectedItemCount())):
      wxString(wxEmptyString));

  wxExFrame::StatusText(text, "PaneItems");
}
#endif

#endif // wxUSE_GUI
