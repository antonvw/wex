////////////////////////////////////////////////////////////////////////////////
// Name:      link.h
// Purpose:   Declaration of class wex::link
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <memory>
#include <string>
#include <wex/path.h>

namespace wex
{
  namespace data
  {
    class control;
  };

  class paths;
  class stc;

  /// Offers a class holding info about a link.
  class link
  {
  public:
    /// line number to be used for data::control
    /// Afterwards line and col from data are filled in if possible.
    enum
    {
      LINE_OPEN_URL          = -2,
      LINE_OPEN_MIME         = -3,
      LINE_OPEN_URL_AND_MIME = -4,
    };

    /// Default constructor.
    link();

    /// Destructor.
    ~link();

    /// Sets paths with info from config.
    /// If there is no config, paths will be empty.
    void config_get();

    /// Returns a path from text, using paths if necessary,
    /// returns empty path if no path could be found.
    const path get_path(
      /// text containing a path somewhere
      const std::string& text,
      /// control data to be filled in Line from data
      data::control& data,
      /// stc component
      stc* stc = nullptr) const;

  private:
    const path find_between(const std::string& text, stc* stc) const;
    const path
               find_filename(const std::string& text, data::control& data) const;
    const path find_url_or_mime(
      const std::string&   text,
      const data::control& data,
      stc*                 stc) const;

    std::unique_ptr<paths> m_paths;
  };
}; // namespace wex
