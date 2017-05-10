////////////////////////////////////////////////////////////////////////////////
// Name:      app.cpp
// Purpose:   Implementation of class App
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/msgout.h>
#include <wx/extension/cmdline.h>
#include <wx/extension/stc.h>
#include <wx/extension/tostring.h>
#include <wx/extension/util.h>
#include <wx/extension/version.h>
#include "app.h"
#include "frame.h"

wxIMPLEMENT_APP(App);

#ifdef __WXOSX__  
void App::MacOpenFiles(const wxArrayString& fileNames)
{
  Frame* frame = wxDynamicCast(GetTopWindow(), Frame);
  wxExOpenFiles(frame, wxExToVectorString(fileNames).Get(), m_Data);
}
#endif

bool App::OnInit()
{
  SetAppName("syncped");
  Reset();
  
  bool exit = false;

  if (
    !wxExApp::OnInit() ||
    !wxExCmdLine(
     {{std::make_tuple("d", "debug", "use debug mode"), [&](bool on) {m_Debug = on && true;}},
      {std::make_tuple("H", "hex", "hex mode"), [&](bool on) {
        if (!on) return;
        m_Data.Flags(STC_WIN_HEX, DATA_OR);}},
      {std::make_tuple("i", "info", "show version"), [&](bool on) {
        if (!on) return;
        wxMessageOutput::Get()->Printf("syncped-%s using:\n%s\nand:\n%s", 
          wxExGetVersionInfo().GetVersionOnlyString().c_str(),
          wxExGetVersionInfo().GetDescription().c_str(),
          wxGetLibraryVersionInfo().GetDescription().c_str());
        exit = true;}},
      {std::make_tuple("l", "locale", "show locale"), [&](bool on) {
        if (!on) return;
        wxMessageOutput::Get()->Printf("Catalog dir: %s\nName: %s\nCanonical name: %s\nLanguage: %d\nLocale: %s\nIs ok: %d\nIs available: %d",
          GetCatalogDir().c_str(),
          GetLocale().GetName().c_str(),
          GetLocale().GetCanonicalName().c_str(),
          GetLocale().GetLanguage(), 
          GetLocale().GetLocale().c_str(),
          GetLocale().IsOk(),
          GetLocale().IsAvailable(GetLocale().GetLanguage()));
          exit = true;}},
      {std::make_tuple("o", "splithor", "split tabs horizontally"), [&](bool on) {
        if (on) m_Split = wxBOTTOM;}},
      {std::make_tuple("O", "splitver", "split tabs vertically"), [&](bool on) {
        if (on) m_Split = wxRIGHT;}},
      {std::make_tuple("R", "readonly", "readonly mode"), [&](bool on) {
        if (on) m_Data.Flags(STC_WIN_READ_ONLY, DATA_OR);}}},
     {{std::make_tuple("c", "command", "vi command"), {CMD_LINE_STRING, [&](const wxAny& s) {
        m_Data.Control(wxExControlData().Command(s.As<std::string>()));}}},
      {std::make_tuple("s", "scriptin", "script in"), {CMD_LINE_STRING, [&](const wxAny& s) {
        m_Scriptin.Open(s.As<std::string>());}}},
      {std::make_tuple("S", "source", "source file"), {CMD_LINE_STRING, [&](const wxAny& s) {
        m_Data.Control(wxExControlData().Command(":so " + s.As<std::string>()));}}},
      {std::make_tuple("t", "tag", "start at tag"), {CMD_LINE_STRING, [&](const wxAny& s) {
        m_Tag = s.As<std::string>();}}},
      {std::make_tuple("w", "scriptout", "script out write"), {CMD_LINE_STRING, [&](const wxAny& s) {
        m_Scriptout.Open(s.As<std::string>(), wxFile::write);}}},
      {std::make_tuple("W", "append", "script out append"), {CMD_LINE_STRING, [&](const wxAny& s) {
        m_Scriptout.Open(s.As<std::string>(), wxFile::write_append);}}}},
     {std::make_pair("files", "input file[:line number][:column number]"), [&](const std::vector<std::string> & v) {
        m_Files = v;
        return true;}}).Parse() || exit)
  {
    return false;
  }

  Frame* frame = new Frame(this);
  
  if (!frame->IsClosing())
  {
    frame->Show();
  }
  
  return !frame->IsClosing();
}

void App::Reset()
{
  // do not reset flags
  m_Data.Control(wxExControlData().Command(""));
  m_Tag.clear();
  m_Split = -1;
}
