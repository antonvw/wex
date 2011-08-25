////////////////////////////////////////////////////////////////////////////////
// Name:      listview.h
// Purpose:   Declaration of class wxExListViewWithFrame
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EX_REPORT_LISTVIEW_H
#define _EX_REPORT_LISTVIEW_H

#include <wx/extension/listview.h>
#include <wx/extension/tool.h>

class wxExFrameWithHistory;

/// Adds a wxExFrameWithHistory to wxExListViewFileName.
/// It also adds a tool menu if appropriate.
class WXDLLIMPEXP_BASE wxExListViewWithFrame : public wxExListViewFileName
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
    ListType type,
    wxWindowID id = wxID_ANY,
    long menu_flags = LIST_MENU_DEFAULT,
    const wxExLexer* lexer = NULL,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize,
    long style = wxLC_LIST  | wxLC_HRULES | wxLC_VRULES | wxSUNKEN_BORDER,
    const wxValidator& validator = wxDefaultValidator,
    const wxString &name = wxListCtrlNameStr);    
    
  /// Returns list type from tool id.
  static ListType GetTypeTool(const wxExTool& tool);
protected:
  virtual void BuildPopupMenu(wxExMenu& menu);
  wxExFrameWithHistory* GetFrame() {return m_Frame;};
  void OnCommand(wxCommandEvent& event);
private:
  void ItemActivated(long item_number);
  void RunItems(const wxExTool& tool);

  const long m_MenuFlags;
  wxExFrameWithHistory* m_Frame;

  DECLARE_EVENT_TABLE()
};

#endif
