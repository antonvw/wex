////////////////////////////////////////////////////////////////////////////////
// Name:      stcfile.h
// Purpose:   Declaration of class wxExSTCFile
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/extension/file.h> // for wxExFile

class wxExSTC;

#if wxUSE_GUI
/// Adds file read and write to wxExSTC.
class WXDLLIMPEXP_BASE wxExSTCFile: public wxExFile
{
public:
  /// Constructor.
  /// Does not open the file.
  wxExSTCFile(
    /// the stc component
    wxExSTC* stc,
    /// the filename to be assigned if not empty
    const std::string& filename = std::string());
  
  /// Override virtual methods.
  virtual bool GetContentsChanged() const override;
  virtual void ResetContentsChanged() override;
protected:
  virtual bool DoFileLoad(bool synced = false) override;
  virtual void DoFileNew() override;
  virtual void DoFileSave(bool save_as = false) override;
private:
  void ReadFromFile(bool get_only_new_data);

  wxExSTC* m_STC;
  wxFileOffset m_PreviousLength;
};
#endif // wxUSE_GUI
