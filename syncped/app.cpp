////////////////////////////////////////////////////////////////////////////////
// Name:      app.cpp
// Purpose:   Implementation of class app
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/cmdline.h>
#include <wex/stc.h>
#include <wex/tostring.h>
#include <wex/util.h>
#include <wex/version.h>
#include <easylogging++.h>
#include "app.h"
#include "frame.h"

wxIMPLEMENT_APP(app);

#ifdef __WXOSX__  
void app::MacOpenFiles(const wxArrayString& fileNames)
{
  frame* frame = wxDynamicCast(GetTopWindow(), ::frame);
  wex::open_files(frame, wex::to_vector_path(fileNames).get(), m_Data);
}
#endif

bool app::OnInit()
{
  SetAppName("syncped");
  reset();
  
  if (bool exit = false;
    !wex::app::OnInit() ||
    !wex::cmdline(
     {{{"d", "debug", "use debug mode"}, [&](bool on) {m_Debug = on;}},
      {{"H", "hex", "hex mode"}, [&](bool on) {
        if (!on) return;
        m_Data.flags(wex::stc_data::WIN_HEX, wex::control_data::OR);}},
      {{"i", "info", "show version"}, [&](bool on) {
        if (!on) return;
        std::cout << 
          "syncped-" << wex::get_version_info().get() << " using\n" << 
            wex::get_version_info().description() << " and:\n" << 
            wxGetLibraryVersionInfo().GetDescription().c_str();
        exit = true;}},
      {{"l", "locale", "show locale"}, [&](bool on) {
        if (!on) return;
        std::cout << 
          "Catalog dir: " << get_catalog_dir() <<
          "\nName: " << get_locale().GetName().c_str() <<
          "\nCanonical name: " << get_locale().GetCanonicalName().c_str() <<
          "\nLanguage: " << get_locale().GetLanguage() <<
          "\nLocale: " << get_locale().GetLocale().c_str() <<
          "\nIsOk: " << get_locale().IsOk();
          if (const auto *info = wxLocale::GetLanguageInfo(get_locale().GetLanguage());
            info == nullptr)
          {
            std::cout << "\nNo info\n";
          }
          else
          {
            std::cout << "\nIs available: " << get_locale().IsAvailable(get_locale().GetLanguage()) << "\n";
          }
        exit = true;}},
      {{"o", "splithor", "split tabs horizontally"}, [&](bool on) {
        if (on) m_Split = wxBOTTOM;}},
      {{"v", "verbose", "activates maximum verbosity"}, [&](bool on) {}},
      {{"O", "splitver", "split tabs vertically"}, [&](bool on) {
        if (on) m_Split = wxRIGHT;}},
      {{"R", "readonly", "readonly mode"}, [&](bool on) {
        if (on) m_Data.flags(wex::stc_data::WIN_READ_ONLY, wex::control_data::OR);}}},
     {{{"c", "command", "vi command"}, {wex::cmdline::STRING, [&](const std::any& s) {
        m_Data.control(wex::control_data().command(std::any_cast<std::string>(s)));}}},
      {{"D", "logfile", "sets log file"}, {wex::cmdline::STRING, [&](const std::any& s) {}}},
      {{"L", "logflags", "sets log flags"}, {wex::cmdline::INT, [&](const std::any& s) {
        el::Loggers::addFlag((el::LoggingFlag)std::any_cast<int>(s));}}},
      {{"m", "vmodule", "activates verbosity for files starting with main to level"}, {wex::cmdline::STRING, [&](const std::any& s) {}}},
      {{"s", "scriptin", "script in"}, {wex::cmdline::STRING, [&](const std::any& s) {
        m_Scriptin.open(std::any_cast<std::string>(s));}}},
      {{"S", "source", "source file"}, {wex::cmdline::STRING, [&](const std::any& s) {
        m_Data.control(wex::control_data().command(":so " + std::any_cast<std::string>(s)));}}},
      {{"t", "tag", "start at tag"}, {wex::cmdline::STRING, [&](const std::any& s) {
        m_Tag = std::any_cast<std::string>(s);}}},
      {{"u", "tagfile", "use tagfile"}, {wex::cmdline::STRING, [&](const std::any& s) {
        m_Data.ctags_filename(std::any_cast<std::string>(s));}}},
      {{"V", "v", "activates verbosity upto verbose level (valid range: 0-9)"}, {wex::cmdline::INT, [&](const std::any& s) {}}},
      {{"w", "scriptout", "script out write"}, {wex::cmdline::STRING, [&](const std::any& s) {
        m_Scriptout.open(std::any_cast<std::string>(s), std::ios_base::out);}}},
      {{"W", "append", "script out append"}, {wex::cmdline::STRING, [&](const std::any& s) {
        m_Scriptout.open(std::any_cast<std::string>(s), std::ios_base::app);}}}},
     {{"files", "input file[:line number][:column number]"}, [&](const std::vector<std::string> & v) {
        for (const auto & f : v) m_Files.emplace_back(f);
        return true;}}).parse() || exit)
  {
    return false;
  }

  frame* f = new frame(this);
  
  if (!f->is_closing())
  {
    f->Show();
  }
  
  return !f->is_closing();
}

void app::reset()
{
  // do not reset flags
  m_Data.control(wex::control_data().command(""));
  m_Tag.clear();
  m_Split = -1;
}
