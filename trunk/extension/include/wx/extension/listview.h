/******************************************************************************\
* File:          listview.h
* Purpose:       Declaration of wxExListView and related classes
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2008, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EXLISTVIEW_H
#define _EXLISTVIEW_H

#include <vector>
#include <wx/imaglist.h>
#include <wx/listctrl.h>
#include <wx/extension/base.h> // for wxExInterface

#if wxUSE_GUI
class wxExListItem;
class wxExListView;

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
class wxExColumn : public wxListItem
{
  friend class wxExListView;
public:
  /// Column types.
  enum wxExColumnType
  {
    COL_INT = 1, ///< integer, should be different from 0, as inverse is used by sorting!
    COL_DATE,    ///< date
    COL_FLOAT,   ///< float
    COL_STRING,  ///< string
  };

  /// Assignment operator.
  /// Visual Studio needs this, though gcc does not.
  wxExColumn& operator=(const wxExColumn& p)
  {
    m_Type = p.m_Type;
    m_ColumnNo = p.m_ColumnNo;
    m_IsSortedAscending = p.m_IsSortedAscending;
    return *this;
  };

  /// Gets the column no.
  int GetColumnNo() const {return m_ColumnNo;}

  /// Returns whether sorting is ascending.
  bool GetIsSortedAscending() const {return m_IsSortedAscending;}

  /// Gets the column type.
  wxExColumnType GetType() const {return m_Type;}

  /// Sets the sort ascending member.
  void SetIsSortedAscending(wxExSortType type);
private:
  wxExColumnType m_Type;
  int m_ColumnNo;
  bool m_IsSortedAscending;
};

/// Adds printing, popup menu, images, columns and items to wxListView.
/// Allows for sorting on any column.
/// Small images have size 16,16 and large images size 32,32.
class wxExListView : public wxListView, public wxExInterface
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
    const wxValidator& validator = wxDefaultValidator,
    wxExImageType image_type = IMAGE_ART);

  /// Destructor.
  virtual ~wxExListView();

  // Interface.
  /// Implement this one if you have images that might be changed after sorting etc.
  virtual void ItemsUpdate() {;};

  /// If column is not found, -1 is returned,
  /// if column is not found and is_required, wxFAIL is invoked.
  int FindColumn(
    const wxString& name, 
    bool is_required = true) const;

  /// Returns the index of the bitmap in the image list used by this list view.
  /// If the artid is not yet on the image lists, it is added to both image lists.
  /// Use only if you setup for IMAGE_ART.
  unsigned int GetArtID(wxArtID artid);

  /// Gets the specified column.
  const wxExColumn GetColumn(int column_no) const;

  /// Gets the field separator.
  const wxChar GetFieldSeparator() const {return m_FieldSeparator;};

  /// Gets image type.
  wxExImageType GetImageType() const {return m_ImageType;};

  /// Gets the sorted column.
  int GetSortedColumnNo() const {return m_SortedColumnNo;};

  /// Asks for an item number and goes to the item.
  bool GotoDialog(const wxString& caption = _("Enter Item Number"));

  /// Inserts a column, default width is set by the column type.
  /// If you specify a width, that one is used.
  void InsertColumn(
    const wxString& name,
    wxExColumn::wxExColumnType type = wxExColumn::COL_INT,
    int width = 0);

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
    wxExSortType sort_method = SORT_TOGGLE);
  
#if wxUSE_STATUSBAR
  /// Updates pane items field on the statusbar.
  void UpdateStatusBar();
#endif
protected:
  // Interface.
  /// Invoked after sorting, allows you to do something extra.
  virtual void AfterSorting() {;};

  /// Inserts a new item with column values from text.
  virtual bool ItemFromText(const wxString& text);

  /// Copies this item (all columns) to text.
  virtual const wxString ItemToText(int item_number);

  /// Interface from wxExInterface.
  virtual bool FindNext(const wxString& text, bool find_next = true);

  /// Builds the popup menu.
  void BuildPopupMenu(wxExMenu& menu);

  /// Clears all items.
  void EditClearAll();

  /// Deletes selected items.
  void EditDelete();

  /// Inverts all items from selected to not selected.
  void EditInvertAll();

  /// Selects all items.
  void EditSelectAll();

  /// Resets column that was used for sorting.
  void SortColumnReset();

  // Events.
  void OnCommand(wxCommandEvent& event);
  void OnFindDialog(wxFindDialogEvent& event);
  void OnList(wxListEvent& event);
  void OnMouse(wxMouseEvent& event);
  void OnShow(wxShowEvent& event);
private:
  // Interface, for wxExInterface overriden methods.
  virtual const wxString BuildPage();

  void CopySelectedItemsToClipboard();
  void PasteItemsFromClipboard();

  const wxChar m_FieldSeparator;

  const wxExImageType m_ImageType;
  const int m_ImageHeightSmall;
  const int m_ImageWidthSmall;
  const int m_ImageHeightLarge;
  const int m_ImageWidthLarge;

  int m_SortedColumnNo;
  int m_ToBeSortedColumnNo;

  std::map<wxArtID, unsigned int> m_ArtIDs;
  // Do not make a const of it, does not compile on Linux.
  std::vector<wxExColumn> m_Columns;

  DECLARE_EVENT_TABLE()
};

/// Offers an item on an wxExListView.
class wxExListItem: public wxListItem
{
public:
  /// Constructor, sets the text for item at the end of the listview.
  wxExListItem(wxExListView* lv, const wxString& text);

  /// Constructor, sets the id.
  wxExListItem(wxExListView* lv, const int id);

  /// Inserts the item on the list.
  long Insert() {
    return m_ListView->InsertItem(*this);}

  /// Gets the column using column number.
  const wxString GetColumnText(int col_no);

  /// Gets the column using column name.
  const wxString GetColumnText(
    const wxString& col_name,
    bool is_required = true) {
    return GetColumnText(m_ListView->FindColumn(col_name, is_required));};

  /// Gets the list view.
  wxExListView* GetListView() {return m_ListView;};

  /// Sets the column text using column number.
  void SetColumnText(const int col_no, const wxString& text);

  /// Sets the column text using column name.
  void SetColumnText(
    const wxString& col_name,
    const wxString& text,
    bool is_required = true) {
    SetColumnText(m_ListView->FindColumn(col_name, is_required), text);};

  /// Sets the image for this item, using the image list from list view.
  /// If the listview does not already contain the image, it is added.
  void SetImage(wxArtID artid) {
    if (m_ListView->GetImageType() == wxExListView::IMAGE_ART)
      return StoreImage(m_ListView->GetArtID(artid));
    else
       wxFAIL;
    };

  /// Sets the file icon image for this item.
  void SetImage(int iconid) {
    if (m_ListView->GetImageType() == wxExListView::IMAGE_FILE_ICON)
      return StoreImage(iconid);
    else
       wxFAIL;
    };
private:
  void StoreImage(int image);

  wxExListView* m_ListView; // cannot be a wxListCtrl, as FindColumn is used from wxExListView
};
#endif // wx_USE_GUI
#endif
