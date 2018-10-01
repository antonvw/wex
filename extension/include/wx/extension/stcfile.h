////////////////////////////////////////////////////////////////////////////////
// Name:      stcfile.h
// Purpose:   Declaration of class wxExSTCFile
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/extension/file.h>

class wxExSTC;

/// Adds file read and write to wxExSTC.
class wxExSTCFile: public wxExFile
{
public:
  /// Constructor.
  /// Does not open the file.
  wxExSTCFile(
    /// the stc component
    wxExSTC* stc,
    /// the filename to be assigned if not empty
    const std::string& filename = std::string());
  
  virtual bool GetContentsChanged() const override;
  virtual void ResetContentsChanged() override;
private:
  virtual bool DoFileLoad(bool synced = false) override;
  virtual void DoFileNew() override;
  virtual void DoFileSave(bool save_as = false) override;
  void ReadFromFile(bool get_only_new_data);

  wxExSTC* m_STC;
  std::streampos m_PreviousLength {0};
};
