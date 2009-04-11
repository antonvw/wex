/******************************************************************************\
* File:          listview.cpp
* Purpose:       Implementation of exListView and related classes
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2008 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/numdlg.h> // for wxGetNumberFromUser
#include <wx/stockitem.h> // for wxGetStockLabel
#include <wx/generic/dirctrlg.h> // for wxTheFileIconsTable
#include <wx/textfile.h> // for wxTextFile::GetEOL()
#include <wx/tokenzr.h>
#include <wx/extension/listview.h>

#if wxUSE_GUI

using namespace std;

void exColumn::SetIsSortedAscending(exSortType type)
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

exListItem::exListItem(exListView* lv, const wxString& text)
  : m_ListView(lv)
{
  SetId(m_ListView->GetItemCount());
  SetMask(wxLIST_MASK_TEXT);
  SetText(text);
}

exListItem::exListItem(exListView* lv, const int itemnumber)
  : m_ListView(lv)
{
  SetId(itemnumber);
}

const wxString exListItem::GetColumnText(int col_no)
{
  if (col_no == -1) return wxEmptyString;

  SetColumn(col_no);
  SetMask(wxLIST_MASK_TEXT);
  m_ListView->GetItem(*this);
  return GetText();
}

bool exListItem::SetBackgroundColour(const wxColour& colour)
{
  wxListItem::SetBackgroundColour(colour);

  SetColumn(0);
  SetMask(0);

  if (!m_ListView->SetItem(*this))
  {
    wxFAIL;
    return false;
  }

  return true;
}

void exListItem::SetColumnText(const int col_no, const wxString& text)
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
    exFrame::StatusText(
      "Warning, column max size is 512, column text: ..." +
        text.substr(text.length() - 25) + " ignored");
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

void exListItem::StoreImage(int image)
{
  wxListItem::SetImage(image);
  SetMask(wxLIST_MASK_IMAGE);
  SetColumn(0);

  if (!m_ListView->SetItem(*this))
  {
    wxLogError(FILE_INFO("Could not set image: %d for item: %ld"),
      GetImage(), GetId());
  }
}


const int ID_COL_FIRST = 1000;
const int ID_COL_LAST = ID_COL_FIRST + 255;

BEGIN_EVENT_TABLE(exListView, wxListView)
  EVT_FIND(wxID_ANY, exListView::OnFindDialog)
  EVT_FIND_CLOSE(wxID_ANY, exListView::OnFindDialog)
  EVT_FIND_NEXT(wxID_ANY, exListView::OnFindDialog)
  EVT_LIST_COL_CLICK(wxID_ANY, exListView::OnList)
  EVT_LIST_COL_RIGHT_CLICK(wxID_ANY, exListView::OnList)
  EVT_LIST_ITEM_DESELECTED(wxID_ANY, exListView::OnList)
  EVT_LIST_ITEM_SELECTED(wxID_ANY, exListView::OnList)
  EVT_MENU(wxID_SORT_ASCENDING, exListView::OnCommand)
  EVT_MENU(wxID_SORT_DESCENDING, exListView::OnCommand)
  EVT_MENU_RANGE(wxID_CUT, wxID_PROPERTIES, exListView::OnCommand)
  EVT_MENU_RANGE(ID_EDIT_LOWEST, ID_EDIT_HIGHEST, exListView::OnCommand)
  EVT_MENU_RANGE(ID_COL_FIRST, ID_COL_LAST, exListView::OnCommand)
  EVT_RIGHT_DOWN(exListView::OnMouse)
END_EVENT_TABLE()

exListView::exListView(wxWindow* parent,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size,
  long style,
  const wxValidator& validator,
  int image_type)
  : wxListView(parent, id, pos, size, style, validator)
  , exInterface()
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
    exApp::GetConfig(_("List Font") + "/Size", 8),
    wxFONTFAMILY_DEFAULT,
    wxFONTSTYLE_NORMAL,
    wxFONTWEIGHT_NORMAL,
    false,
    exApp::GetConfig(_("List Font") + "/Name"));

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

exListView::~exListView()
{
  ItemsClear();
}

const wxString exListView::BuildPage()
{
#if wxUSE_HTML & wxUSE_PRINTING_ARCHITECTURE
  exApp::GetPrinter()->SetFooter(PrintFooter());
  exApp::GetPrinter()->SetHeader(PrintHeader());
#endif

  wxString text;

  text << "<TABLE border=1 cellpadding=4 cellspacing=0 ";

  if ((GetWindowStyle() & wxLC_HRULES) ||
      (GetWindowStyle() & wxLC_VRULES))
    text << "rules=\"all\" ";
  else
    text << "rules=\"none\" ";

  text << ">" << wxTextFile::GetEOL();

  text << "<tr>" << wxTextFile::GetEOL();

  for (
    vector<exColumn>::const_iterator it = m_Columns.begin();
    it != m_Columns.end();
    ++it)
  {
    text << "<td><i>" << it->GetText() << "</i>" << wxTextFile::GetEOL();
  }

  for (long i = 0; i < GetItemCount(); i++)
  {
    text << "<tr>" << wxTextFile::GetEOL();

    exListItem item(this, i);

    for (int col = 0; col < GetColumnCount(); col++)
    {
      text << "<td>" << item.GetColumnText(col) << wxTextFile::GetEOL();
    }
  }

  text << "</TABLE>" << wxTextFile::GetEOL();

  return text;
}

void exListView::BuildPopupMenu(exMenu& menu)
{
  if (GetItemCount() > 0)
  {
    menu.Append(ID_LIST_FIND,
      wxGetStockLabel(wxID_FIND, wxSTOCK_WITH_MNEMONIC | wxSTOCK_WITH_ACCELERATOR),
      wxEmptyString,
      wxART_FIND);

    menu.AppendSeparator();
    
    wxMenu* menuSort = new wxMenu;
    
    for (
      vector<exColumn>::const_iterator it = m_Columns.begin(), int i = COL_ID_FIRST;
      it != m_Columns.end();
      ++it, i++)
    {
      menuSort->Append(i, it->GetText();
    }
    
    menu.AppendSubMenu(menuSort, _("Sort"));
    
    menu.AppendSeparator();
  }

  menu.AppendEdit(true);
}

void exListView::CopySelectedItemsToClipboard()
{
  if (GetSelectedItemCount() == 0) return;

  wxBusyCursor wait;
  wxString clipboard;

  int i = -1;
  while ((i = GetNextSelected(i)) != -1)
  {
    clipboard = clipboard + ItemToText(i) + wxTextFile::GetEOL();
  }

  exClipboardAdd(clipboard);
}

void exListView::EditClearAll()
{
  DeleteAllItems();
  ItemsClear();
#if wxUSE_STATUSBAR
  UpdateStatusBar();
#endif
}

void exListView::EditDelete()
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

void exListView::EditInvertAll()
{
  for (int i = 0; i < GetItemCount(); i++)
  {
    Select(i, !IsSelected(i));
  }
}

void exListView::EditSelectAll()
{
  SetItemState(-1, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);
}

long exListView::FindColumn(const wxString& name, bool is_required) const
{
  for (
    vector<exColumn>::const_iterator it = m_Columns.begin();
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
    wxLogError(FILE_INFO("Column: %s not found"), name.c_str());
  }

  return -1;
}

bool exListView::FindNext(const wxString& text, bool find_next)
{
  if (text.empty())
  {
    return false;
  }

  static bool recursive = false;
  static int start_item;
  static int end_item;

  wxString text_use = text;

  if (!exApp::GetConfig()->GetFindReplaceData()->MatchCase())
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
    exListItem item(this, index);

    for (int col = 0; col < GetColumnCount() && match == -1; col++)
    {
      wxString text = item.GetColumnText(col);

      if (!exApp::GetConfig()->GetFindReplaceData()->MatchCase())
      {
        text.MakeUpper();
      }

      if (exApp::GetConfig()->GetFindReplaceData()->MatchWord())
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

unsigned int exListView::GetArtID(wxArtID artid)
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

const exColumn exListView::GetColumn(int column_no) const
{
  for (
    vector<exColumn>::const_iterator it = m_Columns.begin();
    it != m_Columns.end();
    ++it)
  {
    if (it->GetColumnNo() == column_no)
    {
      return *it;
    }
  }

  return exColumn();
}

bool exListView::GotoDialog(const wxString& caption)
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

void exListView::InsertColumn(
  const wxString& name,
  exColumn::exColumnType type,
  int width)
{
  exColumn col;

  col.m_Type = type;
  col.m_IsSortedAscending = false;

  wxListColumnFormat align = wxLIST_FORMAT_RIGHT;

  switch (type)
  {
  case exColumn::COL_FLOAT: align = wxLIST_FORMAT_RIGHT; if (width == 0) width = 80; break;
  case exColumn::COL_INT: align = wxLIST_FORMAT_RIGHT; if (width == 0) width = 80; break;
  case exColumn::COL_STRING: align = wxLIST_FORMAT_LEFT;  if (width == 0) width = 100; break;
  case exColumn::COL_DATE: align = wxLIST_FORMAT_LEFT;  if (width == 0) width = 150; break;
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

void exListView::ItemsClear()
{
  for (
    vector<exListItem*>::iterator it = m_Items.begin();
    it != m_Items.end();
    ++it)
  {
    delete *it;
  }

  m_Items.clear();
}

bool exListView::ItemFromText(const wxString& text)
{
  if (text.empty())
  {
    wxLogError(FILE_INFO("Text is empty"));
    return false;
  }

  wxStringTokenizer tkz(text, m_FieldSeparator);
  if (tkz.HasMoreTokens())
  {
    const wxString value = tkz.GetNextToken();

    exListItem item(this, value);
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
    exListItem item(this, text);
    item.Insert();
  }

  return true;
}

const wxString exListView::ItemToText(int item_number)
{
  wxString text;

  exListItem item(this, item_number);

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

void exListView::OnCommand(wxCommandEvent& event)
{
  if (event.GetId() >= ID_COL_FIRST && event.GetId() <= ID_COL_LAST)
  {
    SortColumn(event.GetId() - ID_COL_FIRST, SOR_TOGGLE);
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
    FindNext(exApp::GetConfig()->GetFindReplaceData()->GetFindString());
    break;
  case ID_LIST_FIND_PREVIOUS:
    FindNext(exApp::GetConfig()->GetFindReplaceData()->GetFindString(), false);
    break;
  default: wxFAIL;
  }

#if wxUSE_STATUSBAR
  UpdateStatusBar();
#endif
}

void exListView::OnFindDialog(wxFindDialogEvent& event)
{
  exInterface::OnFindDialog(event);
}

void exListView::OnList(wxListEvent& event)
{
  if (event.GetEventType() == wxEVT_COMMAND_LIST_COL_CLICK)
  {
    SortColumn(
      event.GetColumn(),
      (exSortType)exApp::GetConfig("List/SortMethod", SORT_TOGGLE));
  }
  else if (event.GetEventType() == wxEVT_COMMAND_LIST_COL_RIGHT_CLICK)
  {
    m_ToBeSortedColumnNo = event.GetColumn();

    exMenu menu(GetSelectedItemCount() > 0 ? exMenu::MENU_IS_SELECTED: exMenu::MENU_DEFAULT);
    menu.Append(wxID_SORT_ASCENDING);
    menu.Append(wxID_SORT_DESCENDING);

    PopupMenu(&menu);
  }
  else if (event.GetEventType() == wxEVT_COMMAND_LIST_ITEM_DESELECTED)
  {
    UpdateStatusBar();
  }
  else if (event.GetEventType() == wxEVT_COMMAND_LIST_ITEM_SELECTED)
  {
    UpdateStatusBar();
  }
}

void exListView::OnMouse(wxMouseEvent& event)
{
  if (event.RightDown())
  {
    int style = exMenu::MENU_DEFAULT;
    if (GetSelectedItemCount() > 0) style |= exMenu::MENU_IS_SELECTED;
    if (GetItemCount() == 0) style |= exMenu::MENU_IS_EMPTY;
    if (GetSelectedItemCount() == 0 && GetItemCount() > 0) style |= exMenu::MENU_ALLOW_CLEAR;

    exMenu menu(style);
    BuildPopupMenu(menu);

    PopupMenu(&menu);
  }
}

void exListView::PasteItemsFromClipboard()
{
  wxStringTokenizer tkz(exClipboardGet(), wxTextFile::GetEOL());

  while (tkz.HasMoreTokens())
  {
    ItemFromText(tkz.GetNextToken());
  }

  if (exApp::GetConfigBool("List/SortSync"))
  {
    SortColumn(exApp::GetConfig("List/SortColumn", FindColumn(_("Modified"))), SORT_KEEP);
  }
}

int sorted_col_no = 0;

int wxCALLBACK CompareFunctionCB(long item1, long item2, long sortData)
{
  const bool ascending = (sortData > 0);
  const exColumn::exColumnType type = (exColumn::exColumnType)abs(sortData);

  switch (type)
  {
  case exColumn::COL_STRING:
    {
    const wxString& str1 = ((exListItem *)item1)->GetColumnText(sorted_col_no);
    const wxString& str2 = ((exListItem *)item2)->GetColumnText(sorted_col_no);

    if (!exApp::GetConfig()->GetFindReplaceData()->MatchCase())
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

  case exColumn::COL_DATE:
    if (ascending) return (unsigned long)item1 > (unsigned long)item2;
    else           return (unsigned long)item1 < (unsigned long)item2;
  break;

  default:
    if (ascending) return item1 > item2;
    else           return item1 < item2;
  }
}

void exListView::SortColumn(int column_no, exSortType sort_method)
{
  sorted_col_no = column_no;

  SortColumnReset();

  exColumn* sorted_col = &m_Columns[column_no];
  sorted_col->SetIsSortedAscending(sort_method);

  wxBusyCursor wait;

  for (long i = 0; i < GetItemCount(); i++)
  {
    // Keeping the items is necessary for sorting strings.
    m_Items.push_back(new exListItem(this, i));
    exListItem* li = m_Items.back();
    const wxString& val = li->GetColumnText(column_no);

    long longval = 0;

    switch (sorted_col->GetType())
    {
    case exColumn::COL_INT: longval = atoi(val.c_str()); break;
    case exColumn::COL_FLOAT: longval = (long)atof(val.c_str()); break;
    case exColumn::COL_DATE:
      if (!val.empty())
      {
        wxDateTime dt;
        wxString::const_iterator end;

        if (!dt.ParseDateTime(val, &end))
        {
          wxLogError(FILE_INFO("Cannot sort, date not known format"));
          return;
        }

        longval = dt.GetTicks();
      }
    break;
    default: longval = (long)li;
    }

    SetItemData(i, longval);
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

  // Readme: Try to set colour for this sorted column. Does not work.
  /*
  wxListItem item;
  item.SetBackgroundColour("blue");
  SetColumn(m_SortedColumnNo, item);*/

  ItemsClear();

  if (GetItemCount() > 0)
  {
    ItemsUpdate();
    AfterSorting();
  }

  exFrame::StatusText(_("Sorted on") + ": " + sorted_col->GetText());
}

void exListView::SortColumnReset()
{
  if (m_SortedColumnNo != -1 && !m_ArtIDs.empty()) // only if we are using images
  {
    ClearColumnImage(m_SortedColumnNo);
  }

  m_SortedColumnNo = -1;
}

#if wxUSE_STATUSBAR
void exListView::UpdateStatusBar()
{
  const wxString text = wxString::Format("%d", GetItemCount()) +
    wxString((GetSelectedItemCount() > 0) ?
      ("," + wxString::Format("%d", GetSelectedItemCount())):
      wxString(wxEmptyString));

  exFrame::StatusText(text, "PaneItems");
}
#endif

#endif // wxUSE_GUI
