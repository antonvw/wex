////////////////////////////////////////////////////////////////////////////////
// Name:      data/window.h
// Purpose:   Declaration of wex::data::window
// Author:    Anton van Wezenbeek
// Copyright: (c) 2009-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <string>
#include <wex/data.h>
#include <wx/filedlg.h>
#include <wx/window.h>

namespace wex::data
{
const long NUMBER_NOT_SET = wex::NUMBER_NOT_SET;

/// Offers window data to be used by windows or (file) dialogs.
class window
{
public:
  /// Default constructor.
  window();

  /// Sets move path extension.
  /// This is used in file_dialog, to move a matching path extension to front.
  /// Default extensions are not moved.
  window& allow_move_path_extension(const std::string& rhs);

  /// Returns the extension for which it is alloed to
  /// move path extension in a file dialog.
  const auto& allow_move_path_extension() const
  {
    return m_allow_move_path_extension;
  }

  /// Returns button flags.
  const auto& button() const { return m_button; }

  /// Sets buttons flags.
  /// This is a bit list of the following flags:
  /// - wxOK
  /// - wxYES
  /// - wxAPPLY
  /// - wxSAVE
  /// - wxCLOSE
  /// - wxNO
  /// - wxCANCEL
  /// - wxHELP
  /// - wxNO_DEFAULT
  /// The default buttons are wxOK and wxCANCEL
  window& button(long button);

  /// Returns window id.
  const auto& id() const { return m_id; }

  /// Sets window id.
  window& id(wxWindowID id);

  /// Returns name.
  const auto& name() const { return m_name; }

  /// Sets window name.
  window& name(const std::string& name);

  /// Returns parent.
  auto parent() const { return m_parent; }

  /// Sets parent.
  window& parent(wxWindow* parent);

  /// Returns window pos.
  const auto& pos() const { return m_pos; }

  /// Sets window pos.
  window& pos(const wxPoint& point);

  /// Returns window size.
  const auto& size() const { return m_size; }

  /// Sets window size.
  window& size(const wxSize& size);

  /// Returns window style.
  const auto& style() const { return m_style; }

  /// Sets window style.
  /// The style bits available depend on the context.
  /// Therefore default style is NUMBER_NOT_SET,
  window& style(long style);

  /// Returns window title.
  const auto& title() const { return m_title; }

  /// Sets window title.
  window& title(const std::string& title);

  /// Returns wildcard.
  const auto& wildcard() const { return m_wildcard; }

  /// Sets wildcard.
  /// if wildcard is default and file is initialized,
  /// the wildcard is taken from the file
  window& wildcard(const std::string& rhs);

private:
  wxWindowID m_id{wxID_ANY};
  wxPoint    m_pos{wxDefaultPosition};
  wxSize     m_size{wxDefaultSize};
  wxWindow*  m_parent{nullptr};

  std::string m_allow_move_path_extension, m_name, m_title,
    m_wildcard{wxFileSelectorDefaultWildcardStr};

  long m_button{wxOK | wxCANCEL}, m_style{NUMBER_NOT_SET};
};
}; // namespace wex::data
