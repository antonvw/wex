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
#include <wex/ctags.h>
#include <wex/log.h>
#include <wex/stc.h>
#include <wex/tostring.h>
#include <wex/util.h>
#include <wex/version.h>
#include "app.h"
#include "frame.h"

wxIMPLEMENT_APP(app);

#ifdef __WXOSX__  
void app::MacOpenFiles(const wxArrayString& fileNames)
{
  frame* frame = wxDynamicCast(GetTopWindow(), ::frame);
  wex::open_files(frame, wex::to_vector_path(fileNames).get(), m_data);
}
#endif

bool app::OnInit()
{
  SetAppName("syncped");
  reset();
  
  if (bool exit = false;
    !wex::app::OnInit() ||
    !wex::cmdline(
     {{{"debug,d", "use debug mode"}, [&](bool on) {m_debug = on;}},
      {{"hex,H", "hex mode"}, [&](bool on) {
        if (!on) return;
        m_data.flags(wex::stc_data::window_t().set(wex::stc_data::WIN_HEX), wex::control_data::OR);}},
      {{"info,i", "show version"}, [&](bool on) {
        if (!on) return;
        std::cout << 
          "syncped-" << wex::get_version_info().get() << " using\n" << 
            wex::get_version_info().description() << " and:\n" << 
            wxGetLibraryVersionInfo().GetDescription().c_str();
        exit = true;}},
      {{ "locale,l", "show locale"}, [&](bool on) {
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
      {{"splithor,o", "split tabs horizontally"}, [&](bool on) {
        if (on) m_split = wxBOTTOM;}},
      {{"splitver,O", "split tabs vertically"}, [&](bool on) {
        if (on) m_split = wxRIGHT;}},
      {{"readonly,R", "readonly mode"}, [&](bool on) {
        if (on) m_data.flags(wex::stc_data::window_t().set(wex::stc_data::WIN_READ_ONLY), wex::control_data::OR);}}},
     {{{"command,c", "vi command"}, {wex::cmdline::STRING, [&](const std::any& s) {
        m_data.control(wex::control_data().command(std::any_cast<std::string>(s)));}}},
      {{"scriptin,s", "script in"}, {wex::cmdline::STRING, [&](const std::any& s) {
        m_scriptin = std::any_cast<std::string>(s);}}},
      {{"source,S", "source file"}, {wex::cmdline::STRING, [&](const std::any& s) {
        m_data.control(wex::control_data().command(":so " + std::any_cast<std::string>(s)));}}},
      {{"tag,t", "start at tag"}, {wex::cmdline::STRING, [&](const std::any& s) {
        m_tag = std::any_cast<std::string>(s);}}},
      {{"tagfile,u", "use tagfile"}, {wex::cmdline::STRING, [&](const std::any& s) {
        wex::ctags::open(std::any_cast<std::string>(s));}}},
      {{"scriptout,w", "script out"}, {wex::cmdline::STRING, [&](const std::any& s) {
        m_scriptout = std::any_cast<std::string>(s);}}}},
      //{{"append,W", "script out append"}, {wex::cmdline::STRING, [&](const std::any& s) {
      //  m_scriptout.open(std::any_cast<std::string>(s), std::ios_base::out | std::ios_base::app);}}}},
     {{"files", "input file[:line number][:column number]\n"
        "or executable file if -d was specified"}, [&](const std::vector<std::string> & v) {
        for (const auto & f : v) m_files.emplace_back(f);}}).parse(argc, argv) || exit)
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
  m_data.control(wex::control_data().command(""));
  m_tag.clear();
  m_split = -1;
}
