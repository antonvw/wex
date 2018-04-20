////////////////////////////////////////////////////////////////////////////////
// Name:      debug.cpp
// Purpose:   Implementation of class wxExDebug
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <fstream>
#include <iostream>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/config.h>
#include <wx/numdlg.h>
#include <wx/extension/debug.h>
#include <wx/extension/defs.h>
#include <wx/extension/itemdlg.h>
#include <wx/extension/listview.h>
#include <wx/extension/managedframe.h>
#include <wx/extension/menu.h>
#include <wx/extension/menus.h>
#include <wx/extension/process.h>
#include <wx/extension/shell.h>
#include <wx/extension/stc.h>
#include <wx/extension/tokenizer.h>
#include <wx/extension/util.h>

#ifdef __WXGTK__
// This class adds name and pid of running processes to
// a listview. Each process has an entry in /proc,
// with a subdir of the pid as name. In this dir there is 
// a file comm that contains the name of that process.
class wxExDirProcesses : public wxExDir
{
public:
  wxExDirProcesses(wxExListView* lv, bool init)
    : wxExDir("/proc", "[0-9]+", DIR_DIRS)
    , m_ListView(lv) {
    if (init)
    {
      m_ListView->AppendColumns({{"Name", wxExColumn::COL_STRING, 200}, {"Pid"}});
    }
    else
    {
      lv->DeleteAllItems();
    }};
 ~wxExDirProcesses() {
    m_ListView->SortColumn("Name", SORT_ASCENDING);};
private:
  virtual bool OnDir(const wxExPath& p) override {
    std::ifstream ifs(wxExPath(p.Path(), "comm").Path());
    std::string line;
    if (ifs.is_open() && std::getline(ifs, line))
    {
      m_ListView->InsertItem({line, p.GetName()});
    }
    return true;};

  wxExListView* m_ListView;
};
#endif

wxExItemDialog* wxExDebug::m_Dialog = nullptr;

wxExDebug::wxExDebug(wxExManagedFrame* frame, wxExProcess* debug)
  : m_Frame(frame)
  , m_Process(debug)
{
  std::vector< wxExMenuCommands<wxExMenuCommand>> entries;
  
  if (wxExMenus::Load("debug", entries))
  {
    const size_t use = wxConfigBase::Get()->ReadLong("DEBUG", 0);
    m_Entry = entries[use < entries.size() ? use: 0];
  }
}
  
int wxExDebug::AddMenu(wxExMenu* menu, bool popup) const
{
  wxExMenu* sub = (popup ? new wxExMenu: nullptr);
  wxExMenu* use = (popup ? sub: menu);
  
  const auto ret = wxExMenus::BuildMenu(
    m_Entry.GetCommands(), ID_EDIT_DEBUG_FIRST, use, popup);
  
  if (ret > 0 && popup)
  {
    menu->AppendSeparator();
    menu->AppendSubMenu(sub, "debug");
  }

  return ret;
}
  
bool wxExDebug::DeleteAllBreakpoints(const std::string& text)
{
  std::vector<std::string> v;

  if (wxExMatch("(d|del|delete|Delete) (all )?breakpoints", text, v) >= 1)
  {
    for (const auto& it: m_Breakpoints)
    {
      if (m_Frame->IsOpen(std::get<0>(it.second)))
      {
        auto* stc = m_Frame->OpenFile(std::get<0>(it.second));
        stc->MarkerDeleteAll(m_MarkerBreakpoint.GetNo());
      }
    }

    m_Breakpoints.clear();
    
    return true;
  }
  
  return false;
}
  
bool wxExDebug::Execute(const std::string& action, wxExSTC* stc)
{
  std::string args;

  if (!GetArgs(action, args, stc) ||
     (m_Process == nullptr &&
     (m_Process = m_Frame->Process(m_Entry.GetName())) == nullptr))
     return false;

  m_Frame->ShowPane("PROCESS"); 

  if (!m_Process->IsRunning())
  {
    m_Process->Execute(m_Entry.GetName());
  }

  return m_Process->Write(action == "interrupt" ?
    std::string(1, 3) : action + args);
}

bool wxExDebug::GetArgs(
  const std::string& command, std::string& args, wxExSTC* stc)
{
  std::vector<std::string> v;

  if (wxExMatch("^(at|attach)", command, v) == 1)
  {
    static wxExListView* lv = nullptr;
    bool init = false;

    if (m_Dialog == nullptr)
    {
      init = true;
      m_Dialog = new wxExItemDialog({
#ifdef __WXGTK__
      {"processes", wxExListViewData(), std::any(), LABEL_NONE,
        [&](wxWindow* user, const std::any& value, bool save) {
        lv = ((wxExListView *)user);
        if (save && lv->GetFirstSelected() != -1)
        {
          args = " " + lv->GetItemText(lv->GetFirstSelected(), "Pid");
        }
        }}},
#else
      {"pid", ITEM_TEXTCTRL_INT, std::any(), wxExControlData().Required(true),
        LABEL_LEFT,
        [&](wxWindow* user, const std::any& value, bool save) {
           if (save) args += " " + std::to_string(std::any_cast<long>(value));}}},
#endif
      wxExWindowData().Title("Attach").Size({400, 400}).Parent(m_Frame));
    }

    if (lv != nullptr)
    {
#ifdef __WXGTK__
      wxExDirProcesses(lv, init).FindFiles();
#endif
    }

    return m_Dialog->ShowModal() != wxID_CANCEL;
  }
  else if ((wxExMatch("^(br|break)", command, v) == 1) && stc != nullptr)
  {
    args += " " +
      stc->GetFileName().Path().string() + ":" + 
      std::to_string(stc->GetCurrentLine() + 1); 
  }
  else if ((wxExMatch("^(d|del|delete) (br|breakpoint)", command, v) > 0) && stc != nullptr)
  {
    for (const auto& it: m_Breakpoints)
    {
      if (
        stc->GetFileName() == std::get<0>(it.second) &&
        stc->GetCurrentLine() == std::get<2>(it.second))
      {
        args += " " + it.first;
        break;
      }
    }
  }
  else if (DeleteAllBreakpoints(command)) {}
  else if (command == "file")
  {
    return wxExItemDialog(
      {{"File", ITEM_COMBOBOX_FILE, std::any(), wxExControlData().Required(true),
          LABEL_LEFT,
          [&](wxWindow* user, const std::any& value, bool save) {
             if (save) args += " " + std::any_cast<wxArrayString>(value)[0];}},
       {m_Entry.GetName(), ITEM_FILEPICKERCTRL}},
      wxExWindowData().Title("Debug").Parent(m_Frame)).ShowModal() != wxID_CANCEL;
  }
  else if ((wxExMatch("^(p|print)", command, v) == 1) && stc != nullptr)
  {
    args += " " + stc->GetSelectedText(); 
  }
  else if ((wxExMatch("(u|until)", command, v) == 1) && stc != nullptr)
  {
    args += " " + std::to_string(stc->GetCurrentLine());
  }
  
  return true;
}

void wxExDebug::ProcessStdIn(const std::string& text)
{
  std::vector<std::string> v;

  if (wxExMatch("(d|del|delete) +([0-9 ]*)", text, v) > 0)
  {
    switch (v.size())
    {
      case 1:
        DeleteAllBreakpoints(text);
      break;

      case 2:
      {
        wxExTokenizer tkz(v[1], " ");

        while (tkz.HasMoreTokens())
        {
          const auto& it = m_Breakpoints.find(tkz.GetNextToken());

          if (it != m_Breakpoints.end() && m_Frame->IsOpen(std::get<0>(it->second)))
          {
            wxExSTC* stc = m_Frame->OpenFile(std::get<0>(it->second));
            stc->MarkerDeleteHandle(std::get<1>(it->second));
          }
        }
      }
      break;
    }
  }
}

void wxExDebug::ProcessStdOut(const std::string& text)
{
  std::vector<std::string> v;
  wxExControlData data;

  if (
    wxExMatch("Breakpoint ([0-9]+) at 0x[0-9a-f]+: file (.*), line ([0-9]+)", text, v) == 3 || 
    wxExMatch("Breakpoint ([0-9]+) at 0x[0-9a-f]+: (.*):([0-9]+)", text, v) == 3)
  {
    wxExPath filename(v[1]);
    filename.MakeAbsolute();
    if (filename.FileExists())
    {
      auto* stc = m_Frame->OpenFile(filename);
      if (stc != nullptr)
      {
        const int line = std::stoi(v[2]) - 1;
        const auto id = stc->MarkerAdd(line, m_MarkerBreakpoint.GetNo());
        m_Breakpoints[v[0]] = std::make_tuple(filename, id, line);
      }
    }
  }
  else if (DeleteAllBreakpoints(text)) {}
  else if (wxExMatch("at (.*):([0-9]+)", text, v) > 1)
  {
    m_Path = wxExPath(v[0]).MakeAbsolute();
    data.Line(std::stoi(v[1]));
  }
  else if (wxExMatch("^([0-9]+)", text, v) > 0)
  {
    data.Line(std::stoi(v[0]));
  }

  if (data.Line() > 0 && m_Path.FileExists())
  {
    m_Frame->OpenFile(m_Path, data);
    m_Process->GetShell()->SetFocus();
  }
}
