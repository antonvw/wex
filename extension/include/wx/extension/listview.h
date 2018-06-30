////////////////////////////////////////////////////////////////////////////////
// Name:      listview.h
// Purpose:   Declaration of wxExListView and related classes
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>
#include <vector>
#include <wx/artprov.h> // for wxArtID
#include <wx/listctrl.h>
#include <wx/extension/listview-data.h>

class wxExItemDialog;
class wxExMenu;

/*! \file */
/// Sort types.
enum wxExSortType
{
  SORT_KEEP = 1,   ///< keep current order, just resort
  SORT_ASCENDING,  ///< sort ascending
  SORT_DESCENDING, ///< sort descending
  SORT_TOGGLE      ///< toggle sort order
};

/// Offers a column to be used in a wxListCtrl. Facilitates sorting.
class WXDLLIMPEXP_BASE wxExColumn : public wxListItem
{
public:
  /// Column types.
  enum wxExColumnType
  {
    COL_INVALID, ///< illegal col
    COL_INT = 1, ///< integer, should be different from 0, as inverse is used by sorting!
    COL_DATE,    ///< date
    COL_FLOAT,   ///< float
    COL_STRING   ///< string
  };
  
  /// Default constructor.
  wxExColumn();

  /// Constructor.
  wxExColumn(
    /// name of the column
    const std::string& name,
    /// type of the column
    wxExColumnType type = COL_INT,
    /// width of the column, default width (0) uses a width 
    /// that depends on the column type
    /// if you specify a width other than 0, that one is used.
    int width = 0);

  /// Returns whether sorting is ascending.
  bool GetIsSortedAscending() const {return m_IsSortedAscending;}

  /// Returns the column type.
  auto GetType() const {return m_Type;}

  /// Sets the sort ascending member.
  void SetIsSortedAscending(wxExSortType type);
private:
  wxExColumnType m_Type = COL_INVALID;
  bool m_IsSortedAscending = false;
};

/// Adds printing, popup menu, images, columns and items to wxListView.
/// Allows for sorting on any column.
/// Adds some standard lists, all these lists
/// have items associated with files or folders.
class WXDLLIMPEXP_BASE wxExListView : public wxListView
{
public:
  /// Default constructor.
  wxExListView(const wxExListViewData& data = wxExListViewData());

  /// Appends new columns.
  /// Returns false if appending a column failed.
  bool AppendColumns(const std::vector < wxExColumn >& cols);

  /// Shows a dialog with options, returns dialog return code.
  /// If used modeless, it uses the dialog id as specified,
  /// so you can use that id in wxExFrame::OnCommandItemDialog.
  static int ConfigDialog(const wxExWindowData& data = wxExWindowData());

  /// Sets the configurable parameters to values currently in config.
  void ConfigGet();

  /// If column is not found, -1 is returned,
  int FindColumn(const std::string& name) const {return Column(name).GetColumn();};

  /// Finds next.
  bool FindNext(const std::string& text, bool find_next = true);

  /// Returns associated data.
  const auto& GetData() const {return m_Data;};

  /// Returns the item text using item number and column name.
  /// If you do not specify a column, the item label is returned
  /// (this is also valid in non report mode).
  const std::string GetItemText(
    long item_number,
    const std::string& col_name = std::string()) const;
    
  /// Returns current sorted column no.
  int GetSortedColumnNo() const {return m_SortedColumnNo;};

  /// Inserts item with provided columns.
  /// Returns false if insertings fails, or item is empty.
  bool InsertItem(const std::vector < std::string > & item);

  /// Inserts new item swith column values from text.
  /// Items are separated by newlines, columns by a field separator.
  /// Returns true if successfull.
  virtual bool ItemFromText(const std::string& text);

  /// Copies the specified item (all columns) to text.
  /// If item_number = -1, copies all items.
  virtual const std::string ItemToText(long item_number) const;

  /// Implement this one if you have images that might be changed after sorting etc.
  virtual void ItemsUpdate();

  /// Prints the list.
  void Print();

  /// Previews the list.
  void PrintPreview();

  /// Sets an item string field at a particular column.
  /// Returns false if an error occurred.
  bool SetItem(long index, int column, const std::string &label, int imageId = -1);

  /// Sets the item image, using the image list.
  /// If the listview does not already contain the image, it is added.
  bool SetItemImage(long item_number, const wxArtID& artid) {
    return (m_Data.Image() == IMAGE_ART ?
      wxListView::SetItemImage(item_number, GetArtID(artid)): false);};

  /// Sorts on a column specified by column name.
  /// Returns true if column was sorted.
  bool SortColumn(
    const std::string& column_name, 
    wxExSortType sort_method = SORT_TOGGLE) {  
      return SortColumn(FindColumn(column_name), sort_method);};
      
  /// Sorts on a column.
  /// If you did not specify IMAGE_NONE,
  /// the column that is sorted gets an image (wxART_GO_DOWN or wxART_GO_UP), 
  /// depending on whether
  /// it is sorted ascending or descending.
  /// By using wxArtProvider CreateBitmap you can override this image to 
  /// provide your own one.
  bool SortColumn(
    int column_no, 
    wxExSortType sort_method = SORT_TOGGLE);

  /// Resets column that was used for sorting.
  void SortColumnReset();
protected:
  // Interface.
  /// Invoked after sorting, allows you to do something extra.
  virtual void AfterSorting() {;};

  /// Builds the popup menu.
  virtual void BuildPopupMenu(wxExMenu& menu);

  /// Clears all items.
  void EditClearAll();

  /// Returns the field separator.
  const auto& GetFieldSeparator() const {return m_FieldSeparator;};
private:
  const std::string BuildPage();
  wxExColumn Column(const std::string& name) const;
  void CopySelectedItemsToClipboard();
  void EditDelete();
    
  /// Returns the index of the bitmap in the image list used by this list view.
  /// If the artid is not yet on the image lists, it is added to the image list.
  /// Use only if you setup for IMAGE_ART.
  unsigned int GetArtID(const wxArtID& artid);

  void ItemActivated(long item_number);

  /// Sets the item file icon image.
  bool SetItemImage(long item_number, int iconid) {
    return (m_Data.Image() == IMAGE_FILE_ICON ?
      wxListView::SetItemImage(item_number, iconid): false);};

  const wxUniChar m_FieldSeparator = '\t';
  const int m_ImageHeight;
  const int m_ImageWidth;

  wxExListViewData m_Data;

  bool m_ItemUpdated = false;
  long m_ItemNumber = 0;
  
  int m_SortedColumnNo = -1;
  int m_ToBeSortedColumnNo = -1;
  
  std::map<wxArtID, unsigned int> m_ArtIDs;
  std::vector<wxExColumn> m_Columns;
  
  static wxExItemDialog* m_ConfigDialog;
};
