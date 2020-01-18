////////////////////////////////////////////////////////////////////////////////
// Name:      app.cpp
// Purpose:   Implementation of class app
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/version.hpp>
#ifndef __WXMSW__  
#include <easylogging++.h>
#endif
#include <nlohmann/json.hpp>
#include <pugixml.hpp>
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
  auto* frame = dynamic_cast<::frame*>(GetTopWindow());
  wex::open_files(frame, wex::to_vector_path(fileNames).get(), m_data);
}
#endif

bool app::OnInit()
{
  SetAppName("syncped");
  reset();
  
  if (bool exit = false;
    !wex::cmdline(
     {{{"debug,d", "use debug mode"}, 
        [&](bool on) {m_debug = on;}},

      {{"hex,H", "hex mode"}, 
        [&](bool on) {
        if (!on) return;
        m_data.flags(
          wex::stc_data::window_t().set(wex::stc_data::WIN_HEX), 
          wex::control_data::OR);}},

      {{"info,i", "show version"}, 
        [&](bool on) {
        if (!on) return;
        auto json(nlohmann::json::meta());
        std::cout 
          << "syncped-" << wex::get_version_info().get() << " using\n" 
          << wex::get_version_info().description() << "\n"
          << "Boost Libraries version: " 
          << BOOST_VERSION / 100000     << "."  // major version
          << BOOST_VERSION / 100 % 1000 << "."  // minor version
          << BOOST_VERSION % 100                // patch level
          << "\n"
          << json.meta()["name"] << ": " << 
             json.meta()["version"]["string"] << "\n"
          << "pugixml version: " 
          << PUGIXML_VERSION / 1000      << "."  // major version
          << PUGIXML_VERSION % 1000 / 10 << "."  // minor version
          << PUGIXML_VERSION % 10                // patch level
          << "\n"
#ifndef __WXMSW__  
          << "easylogging++ version: " << el::VersionInfo::version() << "\n"
#endif
          << wxGetLibraryVersionInfo().GetDescription().c_str();
        exit = true;}},

      {{ "locale,l", "show locale"}, 
        [&](bool on) {
        if (!on) return;
        std::cout << 
          "Catalog dir: " << get_catalog_dir() <<
          "\nName: " << get_locale().GetName().c_str() <<
          "\nCanonical name: " << get_locale().GetCanonicalName().c_str() <<
          "\nLanguage: " << get_locale().GetLanguage() <<
          "\nLocale: " << get_locale().GetLocale().c_str() <<
          "\nIsOk: " << get_locale().IsOk();
          if (const auto *info = 
              wxLocale::GetLanguageInfo(get_locale().GetLanguage());
            info == nullptr)
          {
            std::cout << "\nNo info\n";
          }
          else
          {
            std::cout << "\nIs available: " << 
              get_locale().IsAvailable(get_locale().GetLanguage()) << "\n";
          }
        exit = true;}},

      {{"splithor,o", "split tabs horizontally"}, 
        [&](bool on) {
        if (on) m_split = wxBOTTOM;}},

      {{"splitver,O", "split tabs vertically"}, 
        [&](bool on) {
        if (on) m_split = wxRIGHT;}},

      {{"readonly,R", "readonly mode"}, 
        [&](bool on) {
        if (on) m_data.flags(
          wex::stc_data::window_t().set(wex::stc_data::WIN_READ_ONLY), 
          wex::control_data::OR);}}},

     {{{"command,c", "vi command"}, 
       {wex::cmdline::STRING, [&](const std::any& s) {
        m_data.control(
          wex::control_data().command(std::any_cast<std::string>(s)));}}},

      {{"config,j", "json config file"}, 
       {wex::cmdline::STRING, [&](const std::any& s) {
        wex::config::set_file(
          std::any_cast<std::string>(s));}}},

      {{"source,S", "source file"}, 
       {wex::cmdline::STRING, [&](const std::any& s) {
        m_data.control(
          wex::control_data().command(":so " + std::any_cast<std::string>(s)));}}},

      {{"tag,t", "start at tag"}, 
       {wex::cmdline::STRING, [&](const std::any& s) {
        m_tag = std::any_cast<std::string>(s);}}},

      {{"tagfile,u", "use tagfile"}, 
       {wex::cmdline::STRING, [&](const std::any& s) {
        wex::ctags::open(std::any_cast<std::string>(s));}}},
      
      {{"scriptin,s", "script in"}, 
       {wex::cmdline::STRING, [&](const std::any& s) {
         m_scriptin = std::any_cast<std::string>(s);}}},
              
      {{"scriptout,w", "script out append"}, 
       {wex::cmdline::STRING, [&](const std::any& s) {
        m_scriptout = std::any_cast<std::string>(s);}}}},

     {{"files", 
       "input file[:line number][:column number]\n"
        "or executable file if -d was specified"}, 
      [&](const std::vector<std::string> & v) {
        for (const auto & f : v) 
          m_files.emplace_back(f);}},
     true,
     "commandline").parse(argc, argv) || 
      exit ||
     !wex::app::OnInit())
  {
    return false;
  }

  auto* f = new frame(this);
  
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
