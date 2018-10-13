////////////////////////////////////////////////////////////////////////////////
// Name:      stc_file.h
// Purpose:   Declaration of class wex::stc_file
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/extension/file.h>

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
    
    virtual bool GetContentsChanged() const override;
    virtual void ResetContentsChanged() override;
  private:
    virtual bool DoFileLoad(bool synced = false) override;
    virtual void DoFileNew() override;
    virtual void DoFileSave(bool save_as = false) override;
    void ReadFromFile(bool get_only_new_data);

    stc* m_STC;
    std::streampos m_PreviousLength {0};
  };
};
