////////////////////////////////////////////////////////////////////////////////
// Name:      notebook.h
// Purpose:   Declaration of class wex::notebook
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <map>
#include <wex/config.h>
#include <wex/defs.h>
#include <wex/file-dialog.h>
#include <wex/managed-frame.h>
#include <wex/notebook-data.h>
#include <wex/stc.h>
#include <wex/window-data.h>
#include <wx/aui/auibook.h>
#include <wx/wupdlock.h>

namespace wex
{
  /// Offers a notebook with page access using keys,
  /// and that interfaces with wex::managed_frame.
  class notebook : public wxAuiNotebook
  {
  public:
    /// Static interface.

    /// Shows a dialog with options, returns dialog return code.
    /// If used modeless, it uses the dialog id as specified,
    /// so you can use that id in frame::on_command_item_dialog.
    static int config_dialog(const data::window& data = data::window());

    /// Other methods.

    /// Default constructor.
    notebook(
      const data::window& data = data::window().style(wxAUI_NB_DEFAULT_STYLE));

    /// Adds the page with given key and fills the keys.
    wxWindow* add_page(const data::notebook& data);

    /// Changes the selection for the given page, returning the previous
    /// selection. If the key does not exist an empty string is returned.
    const std::string change_selection(const std::string& key);

    /// Sets the configurable parameters to values currently in config.
    void config_get();

    /// Returns key for the current page.
    const std::string current_page_key();

    /// Deletes the page with the given key.
    /// Returns true if page could be deleted.
    bool delete_page(const std::string& key);

    /// Do something for each page in the notebook.
    /// The id should be in between ID_ALL_LOWEST and ID_ALL_HIGHEST.
    /// Cannot be const as it can call delete_page.
    template <class T> bool for_each(int id)
    {
      m_frame->set_find_focus(nullptr);

      wxWindowUpdateLocker locker(
        m_frame != nullptr ? (wxWindow*)m_frame : (wxWindow*)this);

      // The page should be an int (no), otherwise page >= 0 never fails!
      for (int page = GetPageCount() - 1; page >= 0; page--)
      {
        switch (T* win = (T*)GetPage(page); id)
        {
          case ID_ALL_CLOSE:
          case ID_ALL_CLOSE_OTHERS:
            if (
              (id == ID_ALL_CLOSE_OTHERS && GetSelection() != page) ||
              id == ID_ALL_CLOSE)
            {
              if (
                file_dialog(&win->get_file()).show_modal_if_changed() ==
                wxID_CANCEL)
              {
                return false;
              }
              const std::string key = m_windows[win];
              m_windows.erase(win);
              m_keys.erase(key);

              if (!wxAuiNotebook::DeletePage(page))
              {
                return false;
              }
            }
            break;

          case ID_ALL_CONFIG_GET:
            win->config_get();
            break;

          case ID_ALL_SAVE:
            if (win->get_file().get_contents_changed())
            {
              win->get_file().file_save();
            }
            break;

          // STC only!!!
          case ID_ALL_STC_SET_LEXER:
            // At this moment same as themed change,
            // as we want default colour updates as well.
            ((stc*)win)->get_lexer().set(
              ((stc*)win)->get_lexer().display_lexer());
            break;

          case ID_ALL_STC_SET_LEXER_THEME:
            ((stc*)win)->get_lexer().set(
              ((stc*)win)->get_lexer().display_lexer());
            break;

          case ID_ALL_STC_SYNC:
            ((stc*)win)->sync(config("AllowSync").get(true));
            break;

          default:
            assert(0);
            break;
        }
      }

      if (m_frame != nullptr && m_keys.empty())
      {
        m_frame->sync_close_all(GetId());
      }
      return true;
    };

    /// Inserts the page with given key and fills the keys.
    wxWindow* insert_page(const data::notebook& data);

    /// Returns the key specified by the given page.
    /// If the page does not exist or is nullptr an empty string is returned.
    const std::string key_by_page(wxWindow* page) const
    {
      if (page == nullptr)
        return std::string();
      const auto& it = m_windows.find(page);
      return (it != m_windows.end() ? it->second : std::string());
    };

    /// Returns the page specified by the given key.
    /// If the key does not exist nullptr is returned.
    wxWindow* page_by_key(const std::string& key) const
    {
      const auto& it = m_keys.find(key);
      return (it != m_keys.end() ? it->second : nullptr);
    };

    /// Returns the page index specified by the given key.
    /// If the key does not exist wxNOT_FOUND is returned.
    int page_index_by_key(const std::string& key) const
    {
      auto* page = page_by_key(key);
      return (page != nullptr ? GetPageIndex(page) : wxNOT_FOUND);
    };

    /// Rearranges all pages.
    void rearrange(
      /// Specify where the pane should go.
      /// It should be one of the following:
      /// - wxTOP
      /// - wxBOTTOM
      /// - wxLEFT
      /// - wxRIGHT
      int direction);

    /// Sets the pagetext for the given new key,
    /// on the page for the given key.
    /// If the key does not exist false is returned.
    bool set_page_text(
      const std::string& key,
      const std::string& new_key,
      const std::string& caption,
      const wxBitmap&    bitmap = wxNullBitmap);

    /// Selects (and returns) the page specified by the given key.
    /// If the key does not exist nullptr is returned.
    wxWindow* set_selection(const std::string& key);

    /// Split performs a split operation programmatically.
    /// If the key does not exist false is returned.
    bool split(
      /// The page that will be split off.
      /// This page will also become the active page after the split.
      const std::string& key,
      /// Specify where the pane should go.
      /// It should be one of the following:
      /// - wxTOP
      /// - wxBOTTOM
      /// - wxLEFT
      /// - wxRIGHT
      int direction);

  private:
    managed_frame* m_frame;
    // In bookctrl.h: m_pages
    std::map<std::string, wxWindow*> m_keys;
    std::map<wxWindow*, std::string> m_windows;

    static inline item_dialog* m_config_dialog = nullptr;
  };
}; // namespace wex
