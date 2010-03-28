////////////////////////////////////////////////////////////////////////////////
// Name:      bar.cpp
// Purpose:   Implementation of several bar classes
// Author:    Anton van Wezenbeek
// Created:   2010-03-26
// RCS-ID:    $Id$
// Copyright: (c) 2010 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/config.h>
#if wxUSE_TOOLTIPS
#include <wx/tooltip.h> // for GetTip
#endif
#include <wx/extension/bar.h>
#include <wx/extension/art.h>
#include <wx/extension/frame.h>
#include <wx/extension/frd.h>
#include <wx/extension/stcfile.h>
#include <wx/extension/util.h>

#if wxUSE_GUI

int wxExPane::m_Total = 0;

#if wxUSE_STATUSBAR
BEGIN_EVENT_TABLE(wxExStatusBar, wxStatusBar)
  EVT_LEFT_DOWN(wxExStatusBar::OnMouse)
  EVT_LEFT_DCLICK(wxExStatusBar::OnMouse)
  EVT_MOTION(wxExStatusBar::OnMouse)
END_EVENT_TABLE()

wxExStatusBar::wxExStatusBar(
  wxExFrame* parent,
  wxWindowID id,
  long style,
  const wxString& name)
  : wxStatusBar(parent, id, style, name)
  , m_Frame(parent)
{
}

const wxExPane wxExStatusBar::GetPane(int pane) const
{
  for (
    std::map<wxString, wxExPane>::const_iterator it = m_Panes.begin();
    it != m_Panes.end();
    ++it)
  {
    if (it->second.m_No == pane)
    {
      return it->second;
    }
  }

  return wxExPane();
}

void wxExStatusBar::OnMouse(wxMouseEvent& event)
{
  bool found = false;

  for (int i = 0; i < GetFieldsCount() && !found; i++)
  {
    wxRect rect;

    if (GetFieldRect(i, rect))
    {
      if (rect.Contains(event.GetPosition()))
      {
        found = true;

        // Handle the event, don't fail if none is true here,
        // it seems that moving and clicking almost at the same time
        // could cause assertions.
        if (event.ButtonDClick())
        {
          m_Frame->StatusBarDoubleClicked(GetPane(i).m_Name);
        }
        else if (event.ButtonDown())
        {
          m_Frame->StatusBarClicked(GetPane(i).m_Name);
        }
#if wxUSE_TOOLTIPS
        // Show tooltip if tooltip is available, and not yet tooltip presented.
        else if (event.Moving())
        {
          if (!m_Panes.empty())
          {
            const wxString tooltip =
              (GetToolTip() != NULL ? GetToolTip()->GetTip(): wxString(wxEmptyString));

            if (tooltip != GetPane(i).m_Helptext)
            {
              SetToolTip(GetPane(i).m_Helptext);
            }
          }
        }
#endif
      }
    }
  }

  event.Skip();
}

void wxExStatusBar::SetPanes(const std::vector<wxExPane>& panes)
{
  int* styles = new int[panes.size()];
  int* widths = new int[panes.size()];

  for (
    std::vector<wxExPane>::const_iterator it = panes.begin();
    it != panes.end();
    ++it)
  {
    m_Panes[it->m_Name] = *it;
    styles[it->m_No] = it->GetStyle();
    widths[it->m_No] = it->GetWidth();
  }

  SetStatusStyles(panes.size(), styles);
  SetStatusWidths(panes.size(), widths);

  delete[] styles;
  delete[] widths;
}

void wxExStatusBar::SetStatusText(const wxString& text, const wxString& pane)
{
  std::map<wxString, wxExPane>::const_iterator it = m_Panes.find(pane);

  if (it != m_Panes.end())
  {
    // wxStatusBar checks whether new text differs from current,
    // and does nothing if the same to avoid flicker.
    wxStatusBar::SetStatusText(text, it->second.m_No);
  }
}

#endif //wxUSE_STATUSBAR

#if wxUSE_TOOLBAR
wxExToolBar::wxExToolBar(wxWindow* parent,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size,
  long style,
  const wxString& name)
  : wxToolBar(parent, id, pos, size, style, name)
{
}

wxToolBarToolBase* wxExToolBar::AddTool(
  int toolId,
  const wxString& label,
  const wxBitmap& bitmap,
  const wxString& shortHelp,
  wxItemKind kind)
{
  const wxExStockArt art(toolId);

  if (art.GetBitmap().IsOk())
  {
    return wxToolBar::AddTool(
      toolId, 
      wxEmptyString,
      art.GetBitmap(wxART_TOOLBAR, GetToolBitmapSize()),
      wxGetStockLabel(toolId, wxSTOCK_NOFLAGS),
      kind);
  }
  else
  {
    return wxToolBar::AddTool(
      toolId, 
      label,
      bitmap,
      shortHelp,
      kind);
  }
}
#endif // wxUSE_TOOLBAR

/// Offers a find combobox that allows you to find text
/// on a current STC on an wxExFrame.
class ComboBox : public wxComboBox
{
public:
  /// Constructor. Fills the combobox box with values from FindReplace from config.
  ComboBox(
    wxWindow* parent,
    wxExFrame* frame,
    wxWindowID id = wxID_ANY,
    const wxPoint& pos = wxDefaultPosition,
    const wxSize& size = wxDefaultSize);
private:
  void OnCommand(wxCommandEvent& event);
  void OnKey(wxKeyEvent& event);
  wxExFrame* m_Frame;

  DECLARE_EVENT_TABLE()
};

BEGIN_EVENT_TABLE(ComboBox, wxComboBox)
  EVT_CHAR(ComboBox::OnKey)
  EVT_MENU(wxID_DELETE, ComboBox::OnCommand)
END_EVENT_TABLE()

ComboBox::ComboBox(
  wxWindow* parent,
  wxExFrame* frame,
  wxWindowID id,
  const wxPoint& pos,
  const wxSize& size)
  : wxComboBox(parent, id, wxEmptyString, pos, size)
  , m_Frame(frame)
{
  const int accels = 1;
  wxAcceleratorEntry entries[accels];
  entries[0].Set(wxACCEL_NORMAL, WXK_DELETE, wxID_DELETE);
  wxAcceleratorTable accel(accels, entries);
  SetAcceleratorTable(accel);

  SetFont(wxConfigBase::Get()->ReadObject("FindFont", 
    wxSystemSettings::GetFont(wxSYS_OEM_FIXED_FONT)));

  wxExComboBoxFromList(
    this, 
    wxExFindReplaceData::Get()->GetFindStrings());

  // And override the value set by previous, as we want text to be same as in Find.
  SetValue(wxExFindReplaceData::Get()->GetFindString());
}

void ComboBox::OnCommand(wxCommandEvent& event)
{
  // README: The delete key default behaviour does not delete the char right from insertion point.
  // Instead, the event is sent to the editor and a char is deleted from the editor.
  // Therefore implement the delete here.
  switch (event.GetId())
  {
  case wxID_DELETE:
    Remove(GetInsertionPoint(), GetInsertionPoint() + 1);
    break;
  default:
    wxFAIL;
    break;
  }
}

void ComboBox::OnKey(wxKeyEvent& event)
{
  const int key = event.GetKeyCode();

  if (key == WXK_RETURN)
  {
    wxExSTC* stc = m_Frame->GetSTC();

    if (stc != NULL)
    {
      stc->FindNext(GetValue());

      wxExFindReplaceData::Get()->SetFindString(GetValue());

      Clear(); // so we can append again
      wxExComboBoxFromList(this, wxExFindReplaceData::Get()->GetFindStrings());
    }
  }
  else
  {
    event.Skip();
  }
}

#if wxUSE_TOOLBAR
// Cannot use wxNewId here, as these are used in a switch statement.
enum
{
  ID_MATCH_WHOLE_WORD = 100,
  ID_MATCH_CASE,
  ID_REGULAR_EXPRESSION,
};

BEGIN_EVENT_TABLE(wxExFindToolBar, wxExToolBar)
  EVT_CHECKBOX(ID_MATCH_WHOLE_WORD, wxExFindToolBar::OnCommand)
  EVT_CHECKBOX(ID_MATCH_CASE, wxExFindToolBar::OnCommand)
  EVT_CHECKBOX(ID_REGULAR_EXPRESSION, wxExFindToolBar::OnCommand)
  EVT_MENU(wxID_DOWN, wxExFindToolBar::OnCommand)
  EVT_MENU(wxID_UP, wxExFindToolBar::OnCommand)
  EVT_UPDATE_UI(wxID_DOWN, wxExFindToolBar::OnUpdateUI)
  EVT_UPDATE_UI(wxID_UP, wxExFindToolBar::OnUpdateUI)
END_EVENT_TABLE()

wxExFindToolBar::wxExFindToolBar(
  wxWindow* parent,
  wxExFrame* frame,
  wxWindowID id)
  : wxExToolBar(parent, id)
  , m_Frame(frame)
{
  Initialize();

  // And place the controls on the toolbar.
  AddControl(m_ComboBox);
  AddSeparator();

  AddTool(
    wxID_DOWN, 
    wxEmptyString, 
    wxArtProvider::GetBitmap(wxART_GO_DOWN, wxART_TOOLBAR, GetToolBitmapSize()),
    _("Find next"));
  AddTool(
    wxID_UP, 
    wxEmptyString, 
    wxArtProvider::GetBitmap(wxART_GO_UP, wxART_TOOLBAR, GetToolBitmapSize()),
    _("Find previous"));
  AddSeparator();

  AddControl(m_MatchWholeWord);
  AddControl(m_MatchCase);
  AddControl(m_RegularExpression);

  Realize();
}

void wxExFindToolBar::Initialize()
{
#ifdef __WXMSW__
  const wxSize size(150, 20);
#else
  const wxSize size(150, -1);
#endif
  ComboBox* cb = new ComboBox(this, m_Frame, wxID_ANY, wxDefaultPosition, size);
  m_ComboBox = cb;

  m_MatchCase = new wxCheckBox(this, 
    ID_MATCH_CASE, wxExFindReplaceData::Get()->GetTextMatchCase());

  m_MatchWholeWord = new wxCheckBox(this, 
    ID_MATCH_WHOLE_WORD, wxExFindReplaceData::Get()->GetTextMatchWholeWord());

  m_RegularExpression= new wxCheckBox(this, 
    ID_REGULAR_EXPRESSION, wxExFindReplaceData::Get()->GetTextRegEx());

  m_MatchCase->SetValue(wxExFindReplaceData::Get()->MatchCase());
  m_MatchWholeWord->SetValue(wxExFindReplaceData::Get()->MatchWord());
  m_RegularExpression->SetValue(wxExFindReplaceData::Get()->UseRegularExpression());
}

void wxExFindToolBar::OnCommand(wxCommandEvent& event)
{
  switch (event.GetId())
  {
  case wxID_DOWN:
  case wxID_UP:
    {
      wxExSTC* stc = m_Frame->GetSTC();

      if (stc != NULL)
      {
        stc->FindNext(m_ComboBox->GetValue(), (event.GetId() == wxID_DOWN));
      }
    }
    break;

  case ID_MATCH_WHOLE_WORD:
    wxExFindReplaceData::Get()->SetMatchWord(
      m_MatchWholeWord->GetValue());
    break;
  case ID_MATCH_CASE:
    wxExFindReplaceData::Get()->SetMatchCase(
      m_MatchCase->GetValue());
    break;
  case ID_REGULAR_EXPRESSION:
    wxExFindReplaceData::Get()->SetUseRegularExpression(
      m_RegularExpression->GetValue());
    break;

  default:
    wxFAIL;
    break;
  }
}

void wxExFindToolBar::OnUpdateUI(wxUpdateUIEvent& event)
{
  event.Enable(!m_ComboBox->GetValue().empty());
}
#endif // wxUSE_TOOLBAR
#endif // wxUSE_GUI
