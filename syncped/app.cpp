////////////////////////////////////////////////////////////////////////////////
// Name:      app.cpp
// Purpose:   Implementation of class App
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/msgout.h>
#include <wx/extension/cmdline.h>
#include <wx/extension/stc.h>
#include <wx/extension/util.h>
#include <wx/extension/version.h>
#include "app.h"
#include "frame.h"

wxIMPLEMENT_APP(App);

#ifdef __WXOSX__  
void App::MacOpenFiles(const wxArrayString& fileNames)
{
  Frame* frame = wxDynamicCast(GetTopWindow(), Frame);
  wxExOpenFiles(frame, wxExToVectorString(fileNames).Get(), m_Flags);
}
#endif

bool App::OnInit()
{
  // This must be the first statement, other methods might use the name.
  SetAppName("syncped");
  
  if (!wxExApp::OnInit())
  {
    return false;
  }
  
  m_Flags = 0; // not in reset
  
  Reset();
  
  bool exit = false;

  if (wxExCmdLineParser(wxExCmdLineParser::CmdSwitches {
      {{"l", _("show locale")}, {0, [&](bool on) {
        wxMessageOutput::Get()->Printf("Catalog dir: %s\nName: %s\nCanonical name: %s\nLanguage: %d\nLocale: %s\nIs ok: %d\nIs available: %d",
          GetCatalogDir().c_str(),
          GetLocale().GetName().c_str(),
          GetLocale().GetCanonicalName().c_str(),
          GetLocale().GetLanguage(), 
          GetLocale().GetLocale().c_str(),
          GetLocale().IsOk(),
          GetLocale().IsAvailable(GetLocale().GetLanguage()));
          exit = true;}}},
      {{"o", _("split tabs horizontally")}, {0, [&](bool on) {
        m_Split = wxBOTTOM;}}},
      {{"O", _("split tabs vertically")}, {0, [&](bool on) {
        m_Split = wxRIGHT;}}},
      {{"R", _("readonly mode")}, {0, [&](bool on) {
        m_Flags = wxExSTC::STC_WIN_READ_ONLY;}}},
      {{"v", _("show version")}, {0, [&](bool on) {
        wxMessageOutput::Get()->Printf("%s", wxExGetVersionInfo().GetVersionOnlyString().c_str());
        exit = true;}}}},
    wxExCmdLineParser::CmdOptions {
      {{"c", _("vi command")}, {wxCMD_LINE_VAL_STRING, [&](wxAny any) {
        any.GetAs(&m_Command);}}},
      {{"S", _("source file")}, {wxCMD_LINE_VAL_STRING, [&](wxAny any) {
        any.GetAs(&m_Command);
        m_Command = ":so " + m_Command;}}}},
    wxExCmdLineParser::CmdParams {
      {_("input file:line number:column number"), {wxCMD_LINE_PARAM_OPTIONAL | wxCMD_LINE_PARAM_MULTIPLE, [&](std::vector<wxString> & v) {
        m_Files = v;}}}}).Parse() != 0 || exit)
  {
    return false;
  }

  Frame* frame = new Frame(this);
  
  if (!frame->IsClosing())
  {
    frame->Show();
  }
  
  return true;
}

void App::Reset()
{
  // do not reset flags
  m_Command.clear();
  m_Split = -1;
}
