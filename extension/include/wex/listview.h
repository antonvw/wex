////////////////////////////////////////////////////////////////////////////////
// Name:      listview.h
// Purpose:   Declaration of wex::listview and related classes
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>
#include <vector>
#include <wx/artprov.h> // for wxArtID
#include <wx/listctrl.h>
#include <wex/listview-data.h>

namespace wex
{
  class item_dialog;
  class menu;

  /*! \file */
  /// Sort types.
  enum sort_t
  {
    SORT_KEEP = 1,   ///< keep current order, just resort
    SORT_ASCENDING,  ///< sort ascending
    SORT_DESCENDING, ///< sort descending
    SORT_TOGGLE      ///< toggle sort order
  };

  /// Offers a column to be used in a wxListCtrl. Facilitates sorting.
  class column : public wxListItem
  {
  public:
    /// Column types.
    enum type_t
    {
      INVALID, ///< illegal col
      INT = 1, ///< integer, should be different from 0, as inverse is used by sorting!
      DATE,    ///< date
      FLOAT,   ///< float
      STRING   ///< string
    };
    
    /// Default constructor.
    column();

    /// Constructor.
    column(
      /// name of the column
      const std::string& name,
      /// type of the column
      type_t type = INT,
      /// width of the column, default width (0) uses a width 
      /// that depends on the column type
      /// if you specify a width other than 0, that one is used.
      int width = 0);

    /// Returns whether sorting is ascending.
    bool is_sorted_ascending() const {return m_IsSortedAscending;}

    /// Sets the sort ascending member.
    void set_is_sorted_ascending(sort_t type);
    
    /// Returns the column type.
    auto type() const {return m_Type;}
  private:
    type_t m_Type = INVALID;
    bool m_IsSortedAscending = false;
  };

  /// Adds printing, popup menu, images, columns and items to wxListView.
  /// Allows for sorting on any column.
  /// Adds some standard lists, all these lists
  /// have items associated with files or folders.
  class listview : public wxListView
  {
  public:
    /// Default constructor.
    listview(const listview_data& data = listview_data());
    
    /// Virtual interface

    /// Inserts new item swith column values from text.
    /// Items are separated by newlines, columns by a field separator.
    /// Returns true if successfull.
    virtual bool item_from_text(const std::string& text);

    /// Copies the specified item (all columns) to text.
    /// If item_number = -1, copies all items.
    virtual const std::string item_to_text(long item_number) const;

    /// Implement this one if you have images that might be changed after sorting etc.
    virtual void items_update();
    
    /// Other methods

    /// Appends new columns.
    /// Returns false if appending a column failed.
    bool append_columns(const std::vector < column >& cols);

    /// Shows a dialog with options, returns dialog return code.
    /// If used modeless, it uses the dialog id as specified,
    /// so you can use that id in frame::on_command_item_dialog.
    static int config_dialog(const window_data& data = window_data());

    /// Sets the configurable parameters to values currently in config.
    void config_get();

    /// If column is not found, -1 is returned,
    int find_column(const std::string& name) const {return Column(name).GetColumn();};

    /// Finds next.
    bool find_next(const std::string& text, bool find_next = true);

    /// Returns associated data.
    const auto& data() const {return m_Data;};

    /// Returns the item text using item number and column name.
    /// If you do not specify a column, the item label is returned
    /// (this is also valid in non report mode).
    const std::string get_item_text(
      long item_number,
      const std::string& col_name = std::string()) const;
      
    /// Inserts item with provided columns.
    /// Returns false if insertings fails, or item is empty.
    bool insert_item(const std::vector < std::string > & item);

    /// Prints the list.
    void print();

    /// Previews the list.
    void print_preview();

    /// Sets an item string field at a particular column.
    /// Returns false if an error occurred.
    bool set_item(long index, int column, const std::string &label, int imageId = -1);

    /// Sets the item image, using the image list.
    /// If the listview does not already contain the image, it is added.
    bool set_item_image(long item_number, const wxArtID& artid) {
      return (m_Data.image() == listview_data::IMAGE_ART ?
        SetItemImage(item_number, GetArtID(artid)): false);};

    /// Sorts on a column specified by column name.
    /// Returns true if column was sorted.
    bool sort_column(
      const std::string& column_name, 
      sort_t sort_method = SORT_TOGGLE) {  
        return sort_column(find_column(column_name), sort_method);};
        
    /// Sorts on a column.
    /// If you did not specify IMAGE_NONE,
    /// the column that is sorted gets an image (wxART_GO_DOWN or wxART_GO_UP), 
    /// depending on whether
    /// it is sorted ascending or descending.
    /// By using wxArtProvider CreateBitmap you can override this image to 
    /// provide your own one.
    bool sort_column(
      int column_no, 
      sort_t sort_method = SORT_TOGGLE);

    /// Resets column that was used for sorting.
    void sort_column_reset();
    
    /// Returns current sorted column no.
    int sorted_column_no() const {return m_SortedColumnNo;};
  protected:
    // Interface.
    /// Invoked after sorting, allows you to do something extra.
    virtual void after_sorting() {;};

    /// Builds the popup menu.
    virtual void build_popup_menu(menu& menu);

    /// Clears all items.
    void clear();

    /// Returns the field separator.
    const auto& field_separator() const {return m_FieldSeparator;};
  private:
    const std::string build_page();
    column Column(const std::string& name) const;
    void CopySelectedItemsToClipboard();
    void EditDelete();
      
    /// Returns the index of the bitmap in the image list used by this list view.
    /// If the artid is not yet on the image lists, it is added to the image list.
    /// Use only if you setup for IMAGE_ART.
    unsigned int GetArtID(const wxArtID& artid);

    void item_activated(long item_number);

    /// Sets the item file icon image.
    bool set_item_image(long item_number, int iconid) {
      return (m_Data.image() == listview_data::IMAGE_FILE_ICON ?
        SetItemImage(item_number, iconid): false);};

    const wxUniChar m_FieldSeparator = '\t';
    const int m_ImageHeight;
    const int m_ImageWidth;

    listview_data m_Data;

    bool m_ItemUpdated = false;
    long m_ItemNumber = 0;
    
    int m_SortedColumnNo = -1;
    int m_ToBeSortedColumnNo = -1;
    
    std::map<wxArtID, unsigned int> m_ArtIDs;
    std::vector<column> m_Columns;
    
    static item_dialog* m_config_dialog;
  };
};
