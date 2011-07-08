////////////////////////////////////////////////////////////////////////////////
// Name:      listview.h
// Purpose:   Declaration of wxExListView and related classes
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXLISTVIEW_H
#define _EXLISTVIEW_H

#include <map>
#include <vector>
#include <wx/fdrepdlg.h> // for wxFindDialogEvent
#include <wx/listctrl.h>
#include <wx/extension/menu.h> // for wxExMenu

#if wxUSE_GUI

/*! \file */
/// Sort types.
enum wxExSortType
{
  SORT_KEEP = 1,   ///< keep current order, just resort
  SORT_ASCENDING,  ///< sort ascending
  SORT_DESCENDING, ///< sort descending
  SORT_TOGGLE,     ///< toggle sort order
};

/// Offers a column to be used in a wxListCtrl. Facilitates sorting.
class WXDLLIMPEXP_BASE wxExColumn : public wxListItem
{
public:
  /// Column types.
  enum wxExColumnType
  {
    COL_INT = 1, ///< integer, should be different from 0, as inverse is used by sorting!
    COL_DATE,    ///< date
    COL_FLOAT,   ///< float
    COL_STRING,  ///< string
  };

  /// Constructor.
  /// Default width is set by the column type.
  /// If you specify a width, that one is used.
  wxExColumn(
    const wxString& name = wxEmptyString,
    wxExColumnType type = COL_INT,
    int width = 0);

  /// Assignment operator.
  /// Visual Studio needs this, though gcc does not.
  wxExColumn& operator=(const wxExColumn& p)
  {
    m_Type = p.m_Type;
    m_IsSortedAscending = p.m_IsSortedAscending;
    return *this;
  };

  /// Returns whether sorting is ascending.
  bool GetIsSortedAscending() const {return m_IsSortedAscending;}

  /// Gets the column type.
  wxExColumnType GetType() const {return m_Type;}

  /// Sets the sort ascending member.
  void SetIsSortedAscending(wxExSortType type);
private:
  wxExColumnType m_Type;
  bool m_IsSortedAscending;
};

/// Adds printing, popup menu, images, columns and items to wxListView.
/// Allows for sorting on any column.
class WXDLLIMPEXP_BASE wxExListView : public wxListView
{
public:
  /// Which images to use.
  enum wxExImageType
  {
    IMAGE_NONE,
    IMAGE_ART,       ///< using wxArtProvider
    IMAGE_FILE_ICON, ///< using the wxFileIconsTable
    IMAGE_OWN,       ///< use your own images
  };

  /// Constructor.
  wxExListView(wxWindow* parent,
    wxWindowID id = wxID_ANY,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxLC_LIST | wxLC_HRULES | wxLC_VRULES | wxSUNKEN_BORDER,
    wxExImageType image_type = IMAGE_ART,
    const wxValidator& validator = wxDefaultValidator,
    const wxString &name = wxListCtrlNameStr);

  /// If column is not found, -1 is returned,
  int FindColumn(const wxString& name) const;

  /// Finds next.
  bool FindNext(const wxString& text, bool find_next = true);

  /// Returns the index of the bitmap in the image list used by this list view.
  /// If the artid is not yet on the image lists, it is added to the image list.
  /// Use only if you setup for IMAGE_ART.
  unsigned int GetArtID(const wxArtID& artid);

  /// Gets the field separator.
  const wxUniChar& GetFieldSeparator() const {return m_FieldSeparator;};

  /// Gets image type.
  wxExImageType GetImageType() const {return m_ImageType;};

  /// Gets the item text using item number and column name.
  const wxString GetItemText(
    long item_number,
    const wxString& col_name) const;

  /// Asks for an item number and goes to the item.
  bool GotoDialog(const wxString& caption = _("Enter Item Number"));

  /// Inserts a column.
  void InsertColumn(const wxExColumn& col);

  /// Inserts a new item with column values from text.
  virtual bool ItemFromText(const wxString& text);

  /// Copies this item (all columns) to text.
  virtual const wxString ItemToText(long item_number) const;

  /// Implement this one if you have images that might be changed after sorting etc.
  virtual void ItemsUpdate() {;};

  /// Prints the list.
  void Print();

  /// Previews the list.
  void PrintPreview();

  /// Sets the item image, using the image list.
  /// If the listview does not already contain the image, it is added.
  bool SetItemImage(long item_number, const wxArtID& artid) {
    return (GetImageType() == IMAGE_ART ?
      wxListView::SetItemImage(item_number, GetArtID(artid)): false);};

  /// Sets the item file icon image.
  bool SetItemImage(long item_number, int iconid) {
    return (GetImageType() == IMAGE_FILE_ICON ?
      wxListView::SetItemImage(item_number, iconid): false);};

  /// Sorts on a column.
  /// If you specified use_images,
  /// the column that is sorted gets an image (wxART_GO_DOWN or wxART_GO_UP), depending on whether
  /// it is sorted ascending or descending.
  /// By using wxArtProvider CreateBitmap you can override this image to provide your own one.
  void SortColumn(
    int column_no, 
    wxExSortType sort_method = SORT_TOGGLE);

  /// Sorts on a column specified by column name.
  void SortColumn(
    const wxString& column_name, 
    wxExSortType sort_method = SORT_TOGGLE) {  
      SortColumn(FindColumn(column_name), sort_method);};
protected:
  // Interface.
  /// Invoked after sorting, allows you to do something extra.
  virtual void AfterSorting() {;};

  /// Builds the popup menu.
  virtual void BuildPopupMenu(wxExMenu& menu);

  /// Clears all items.
  void EditClearAll();

  /// Resets column that was used for sorting.
  void SortColumnReset();

  // Events.
  void OnCommand(wxCommandEvent& event);
  void OnFindDialog(wxFindDialogEvent& event);
  void OnList(wxListEvent& event);
  void OnMouse(wxMouseEvent& event);
  void OnShow(wxShowEvent& event);
private:
  const wxString BuildPage();
  void CopySelectedItemsToClipboard();
  void EditDelete();
  void EditInvertAll() {
    for (auto i = 0; i < GetItemCount(); i++)
    {
      Select(i, !IsSelected(i));
    }}
  void EditSelectAll() {
    SetItemState(-1, wxLIST_STATE_SELECTED, wxLIST_STATE_SELECTED);};

  const wxUniChar m_FieldSeparator;
  const wxExImageType m_ImageType;
  const int m_ImageHeight;
  const int m_ImageWidth;

  int m_SortedColumnNo;
  int m_ToBeSortedColumnNo;
  
  std::map<wxArtID, unsigned int> m_ArtIDs;
  // Do not make a const of it, does not compile on Linux.
  std::vector<wxExColumn> m_Columns;

  DECLARE_EVENT_TABLE()
};
#endif // wx_USE_GUI
#endif
