////////////////////////////////////////////////////////////////////////////////
// Name:      data/item.h
// Purpose:   Declaration of wex::data::item class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <any>
#include <functional>
#include <wex/data/control.h>
#include <wx/imaglist.h>

namespace wex::data
{
  /// Offers user data to be used by item.
  class item
  {
  public:
    /// Label types supported.
    enum label_t
    {
      LABEL_NONE,  ///< no label
      LABEL_LEFT,  ///< label left from window
      LABEL_ABOVE, ///< label above window
    };

    /// A function that you can provide to e.g. specify what
    /// to do when clicking on a button item.
    typedef std::function<
      void(wxWindow* user, const std::any& value, bool save)>
      user_apply_t;

    /// A function that you can provide to specify what needs to
    /// be done for creating a user item.
    typedef std::function<void(wxWindow* user, wxWindow* parent, bool readonly)>
      user_window_create_t;

    /// A function that you can provide to specify what needs to
    /// be done for loading or saving a user item to the config.
    typedef std::function<bool(wxWindow* user, bool save)>
      user_window_to_config_t;

    /// Default constructor from control data.
    item(const data::control& data = data::control());

    /// Copy constructor, with initial value.
    item(const item& data, const std::any& initial = std::any());

    /// Assignment operator.
    item& operator=(const item& rhs);

    /// Returns apply.
    const auto apply() const { return m_apply; };

    /// Sets apply.
    item& apply(const user_apply_t rhs);

    /// Returns the number of columns for the current page.
    auto columns() const { return m_major_dimension; };

    /// Sets columns.
    item& columns(int rhs);

    /// Returns control data.
    const auto& control() const { return m_data; };

    /// Sets control data.
    item& control(const data::control& rhs);

    /// Returns image list.
    const auto image_list() const { return m_image_list; };

    /// Sets image list.
    item& image_list(wxImageList* il);

    /// increment value if appropriate
    const auto& inc() const { return m_inc; };

    /// Sets inc.
    item& inc(const std::any& rhs);

    /// Returns the initial value.
    const auto& initial() const { return m_initial; };

    /// Returns label type.
    const auto label_type() const { return m_label_type; };

    /// Sets label type.
    item& label_type(label_t rhs);

    /// Returns min value.
    const auto& min() const { return m_min; };

    /// Sets min.
    item& min(const std::any& rhs);

    /// Returns max value.
    const auto& max() const { return m_max; };

    /// Sets max.
    item& max(const std::any& rhs);

    /// Return the process callback for window creation
    auto& user_window_create() const { return m_user_window_create_t; };

    /// Sets user window create.
    item& user_window_create(const user_window_create_t rhs);

    /// Returns the process callback for window config
    auto& user_window_to_config() const { return m_user_window_to_config_t; };

    /// Sets user window config.
    item& user_window_to_config(const user_window_to_config_t rhs);

    /// Returns window data.
    const data::window& window() const { return m_data.window(); };

    /// Sets window data.
    item& window(const data::window& rhs);

  private:
    class data::control m_data;

    std::any m_initial, m_min{0}, m_max{1}, m_inc{1};
    int      m_major_dimension{1};
    label_t  m_label_type{LABEL_LEFT};

    wxImageList* m_image_list{nullptr};

    user_apply_t            m_apply{nullptr};
    user_window_create_t    m_user_window_create_t{nullptr};
    user_window_to_config_t m_user_window_to_config_t{nullptr};
  };
} // namespace wex::data
