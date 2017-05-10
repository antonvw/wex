////////////////////////////////////////////////////////////////////////////////
// Name:      listview.h
// Purpose:   Declaration of class wxExListViewWithFrame
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/extension/listview.h>
#include <wx/extension/tool.h>

class wxExFrameWithHistory;

/// Adds a wxExFrameWithHistory to wxExListView.
/// It also adds a tool menu if appropriate.
class WXDLLIMPEXP_BASE wxExListViewWithFrame : public wxExListView
{
public:
  /// Default constructor.
  wxExListViewWithFrame(const wxExListViewData& data = wxExListViewData());
    
  /// Destroys the window safely.
  virtual bool Destroy() override;

  /// Returns list type from tool id.
  static wxExListType GetTypeTool(const wxExTool& tool);
protected:
  virtual void BuildPopupMenu(wxExMenu& menu) override;
  wxExFrameWithHistory* GetFrame() {return m_Frame;};
private:
  const long m_MenuFlags;
  wxExFrameWithHistory* m_Frame;
};
