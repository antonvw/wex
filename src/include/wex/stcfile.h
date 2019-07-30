////////////////////////////////////////////////////////////////////////////////
// Name:      stc_file.h
// Purpose:   Declaration of class wex::stc_file
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/file.h>

namespace wex
{
  class stc;

  /// Adds file read and write to stc.
  class stc_file: public file
  {
  public:
    /// Constructor.
    /// Does not open the file.
    stc_file(
      /// the stc component
      stc* stc,
      /// the filename to be assigned if not empty
      const std::string& filename = std::string());
    
    virtual bool get_contents_changed() const override;
    virtual void reset_contents_changed() override;
  private:
    virtual bool do_file_load(bool synced = false) override;
    virtual void do_file_new() override;
    virtual void do_file_save(bool save_as = false) override;
    void read_from_file(bool get_only_new_data, bool hexmode);

    stc* m_STC;
    std::streampos m_PreviousLength {0};
  };
};
