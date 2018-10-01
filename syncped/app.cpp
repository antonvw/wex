////////////////////////////////////////////////////////////////////////////////
// Name:      app.cpp
// Purpose:   Implementation of class App
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/cmdline.h>
#include <wx/extension/stc.h>
#include <wx/extension/tostring.h>
#include <wx/extension/util.h>
#include <wx/extension/version.h>
#include <easylogging++.h>
#include "app.h"
#include "frame.h"

wxIMPLEMENT_APP(App);

#ifdef __WXOSX__  
void App::MacOpenFiles(const wxArrayString& fileNames)
{
  Frame* frame = wxDynamicCast(GetTopWindow(), Frame);
  wxExOpenFiles(frame, wxExToVectorPath(fileNames).Get(), m_Data);
}
#endif

bool App::OnInit()
{
  SetAppName("syncped");
  Reset();
  
  if (bool exit = false;
    !wxExApp::OnInit() ||
    !wxExCmdLine(
     {{{"d", "debug", "use debug mode"}, [&](bool on) {m_Debug = on;}},
      {{"H", "hex", "hex mode"}, [&](bool on) {
        if (!on) return;
        m_Data.Flags(STC_WIN_HEX, DATA_OR);}},
      {{"i", "info", "show version"}, [&](bool on) {
        if (!on) return;
        std::cout << 
          "syncped-" << wxExGetVersionInfo().Get().c_str() << " using\n" << 
            wxExGetVersionInfo().Description().c_str() << " and:\n" << 
            wxGetLibraryVersionInfo().GetDescription().c_str();
        exit = true;}},
      {{"l", "locale", "show locale"}, [&](bool on) {
        if (!on) return;
        std::cout << 
          "Catalog dir: " << GetCatalogDir() <<
          "\nName: " << GetLocale().GetName().c_str() <<
          "\nCanonical name: " << GetLocale().GetCanonicalName().c_str() <<
          "\nLanguage: " << GetLocale().GetLanguage() <<
          "\nLocale: " << GetLocale().GetLocale().c_str() <<
          "\nIs ok: " << GetLocale().IsOk();
          if (const auto *info = wxLocale::GetLanguageInfo(GetLocale().GetLanguage());
            info == nullptr)
          {
            std::cout << "\nNo info\n";
          }
          else
          {
            std::cout << "\nIs available: " << GetLocale().IsAvailable(GetLocale().GetLanguage()) << "\n";
          }
        exit = true;}},
      {{"o", "splithor", "split tabs horizontally"}, [&](bool on) {
        if (on) m_Split = wxBOTTOM;}},
      {{"v", "verbose", "activates maximum verbosity"}, [&](bool on) {}},
      {{"O", "splitver", "split tabs vertically"}, [&](bool on) {
        if (on) m_Split = wxRIGHT;}},
      {{"R", "readonly", "readonly mode"}, [&](bool on) {
        if (on) m_Data.Flags(STC_WIN_READ_ONLY, DATA_OR);}}},
     {{{"c", "command", "vi command"}, {CMD_LINE_STRING, [&](const std::any& s) {
        m_Data.Control(wxExControlData().Command(std::any_cast<std::string>(s)));}}},
      {{"D", "logfile", "sets log file"}, {CMD_LINE_STRING, [&](const std::any& s) {}}},
      {{"L", "logflags", "sets log flags"}, {CMD_LINE_INT, [&](const std::any& s) {
        el::Loggers::addFlag((el::LoggingFlag)std::any_cast<int>(s));}}},
      {{"m", "vmodule", "activates verbosity for files starting with main to level"}, {CMD_LINE_STRING, [&](const std::any& s) {}}},
      {{"s", "scriptin", "script in"}, {CMD_LINE_STRING, [&](const std::any& s) {
        m_Scriptin.Open(std::any_cast<std::string>(s));}}},
      {{"S", "source", "source file"}, {CMD_LINE_STRING, [&](const std::any& s) {
        m_Data.Control(wxExControlData().Command(":so " + std::any_cast<std::string>(s)));}}},
      {{"t", "tag", "start at tag"}, {CMD_LINE_STRING, [&](const std::any& s) {
        m_Tag = std::any_cast<std::string>(s);}}},
      {{"u", "tagfile", "use tagfile"}, {CMD_LINE_STRING, [&](const std::any& s) {
        m_Data.CTagsFileName(std::any_cast<std::string>(s));}}},
      {{"V", "v", "activates verbosity upto verbose level (valid range: 0-9)"}, {CMD_LINE_INT, [&](const std::any& s) {}}},
      {{"w", "scriptout", "script out write"}, {CMD_LINE_STRING, [&](const std::any& s) {
        m_Scriptout.Open(std::any_cast<std::string>(s), std::ios_base::out);}}},
      {{"W", "append", "script out append"}, {CMD_LINE_STRING, [&](const std::any& s) {
        m_Scriptout.Open(std::any_cast<std::string>(s), std::ios_base::app);}}}},
     {{"files", "input file[:line number][:column number]"}, [&](const std::vector<std::string> & v) {
        for (const auto & f : v) m_Files.emplace_back(f);
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
