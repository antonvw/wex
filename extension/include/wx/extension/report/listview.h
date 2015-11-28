////////////////////////////////////////////////////////////////////////////////
// Name:      listview.h
// Purpose:   Declaration of class wxExListViewWithFrame
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
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
  /// Menu flags, they determine how the context menu will appear.
  enum
  {
    LIST_MENU_REPORT_FIND = 0x0001, ///< for adding find and replace in files
    LIST_MENU_TOOL        = 0x0002, ///< for adding tool menu

    LIST_MENU_DEFAULT = 
      LIST_MENU_REPORT_FIND | 
      LIST_MENU_TOOL
  };

  /// Constructor.
  wxExListViewWithFrame(wxWindow* parent,
    wxExFrameWithHistory* frame,
    wxExListType type,
    wxWindowID id = wxID_ANY,
    long menu_flags = LIST_MENU_DEFAULT,
    const wxExLexer* lexer = nullptr,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxLC_REPORT,
    const wxValidator& validator = wxDefaultValidator,
    const wxString &name = wxListCtrlNameStr);    
    
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
