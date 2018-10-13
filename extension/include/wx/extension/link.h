////////////////////////////////////////////////////////////////////////////////
// Name:      link.h
// Purpose:   Declaration of class wex::link
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <memory>
#include <string>
#include <wx/extension/path.h>

#define LINK_LINE_OPEN_URL          -1
#define LINK_LINE_OPEN_MIME         -2
#define LINK_LINE_OPEN_URL_AND_MIME -3

namespace wex
{
  class control_data;
  class paths;
  class stc;

  /// Offers a class holding info about a link.
  class link
  {
  public:
    /// Default constructor.
    link(stc* stc = nullptr);

    /// Destructor.
   ~link();
    
    /// Returns a path from text, using paths if necessary,
    /// returns empty path if no path could be found.
    const path GetPath(
      /// text containing a path somewhere
      const std::string& text,
      /// control data to be filled in Line from data, 
      /// you can use:
      /// - LINK_LINE_OPEN_URL 
      /// - LINK_LINE_OPEN_MIME 
      /// - LINK_LINE_OPEN_URL_AND_MIMTE 
      /// as line number. 
      /// Afterwards Line and Col from data are filled in if possible.
      control_data& data) const;
    
    /// Sets paths with info from config.
    /// If there is no config, paths will be empty.
    void SetFromConfig();
  private:
    const path FindPath(const std::string& text, const control_data& data) const;
    bool SetLink(path& text, control_data& data) const;
    
    std::unique_ptr<paths> m_Paths;
    stc* m_STC;
  };
};
