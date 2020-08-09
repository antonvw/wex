////////////////////////////////////////////////////////////////////////////////
// Name:      app.cpp
// Purpose:   Implementation of class app
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include "app.h"
#include "frame.h"
#include <wex/cmdline.h>
#include <wex/ctags.h>
#include <wex/lexers.h>
#include <wex/stc-data.h>
#include <wex/tostring.h>
#include <wex/util.h>
#include <wex/version.h>

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

  bool               list_lexers = false;
  wex::data::cmdline data(argc, argv);

  if (bool exit = false;
      !wex::cmdline(
         {// --- boolean options ---
          {{"end,+", "start at end any file opened"},
           [&](const std::any& s) {
             m_data.control(wex::data::control().command("G"));
           }},

          {{"debug,d", "use debug mode"},
           [&](bool on) {
             m_is_debug = on;
           }},

          {{"echo,e", "echo commands"},
           [&](bool on) {
             m_is_echo = on;
           }},

          {{"stdin,E", "use stdin"},
           [&](bool on) {
             m_is_stdin = on;
           }},

          {{"hex,H", "hex mode"},
           [&](bool on) {
             if (!on)
               return;
             m_data.flags(
               wex::data::stc::window_t().set(wex::data::stc::WIN_HEX),
               wex::data::control::OR);
           }},

          {{"info,i", "show versions"},
           [&](bool on) {
             if (on)
             {
               std::cout << "syncped-" << wex::get_version_info().get()
                         << " using\n"
                         << wex::get_version_info().external_libraries().str();
               exit = true;
             }
           }},

          {{"keep,k", "keep vi options: +, c, s"},
           [&](bool on) {
             m_keep = on;
           }},

          {{"list-lexers,L", "show list of lexers"},
           [&](bool on) {
             list_lexers = on;
           }},

          {{"locale,l", "show locale"},
           [&](bool on) {
             if (!on)
               return;
             std::cout << "Catalog dir: " << get_catalog_dir()
                       << "\nName: " << get_locale().GetName().c_str()
                       << "\nCanonical name: "
                       << get_locale().GetCanonicalName().c_str()
                       << "\nLanguage: " << get_locale().GetLanguage()
                       << "\nLocale: " << get_locale().GetLocale().c_str()
                       << "\nIsOk: " << get_locale().IsOk();
             if (const auto* info =
                   wxLocale::GetLanguageInfo(get_locale().GetLanguage());
                 info == nullptr)
             {
               std::cout << "\nNo info\n";
             }
             else
             {
               std::cout << "\nIs available: "
                         << get_locale().IsAvailable(get_locale().GetLanguage())
                         << "\n";
             }
             exit = true;
           }},

          {{"no-config,n", "do not save json config on exit"},
           [&](bool on) {
             if (on)
             {
               wex::config::discard();
             }
           }},

          {{"project,p", "open specified files as projects"},
           [&](bool on) {
             m_is_project = on;
           }},

          {{"splithor,o", "split tabs horizontally"},
           [&](bool on) {
             if (on)
               m_split = wxBOTTOM;
           }},

          {{"splitver,O", "split tabs vertically"},
           [&](bool on) {
             if (on)
               m_split = wxRIGHT;
           }},

          {{"readonly,R", "readonly mode"},
           [&](bool on) {
             if (on)
               m_data.flags(
                 wex::data::stc::window_t().set(wex::data::stc::WIN_READ_ONLY),
                 wex::data::control::OR);
           }},

          {{"echo-output,x", "echo output commands"},
           [&](bool on) {
             m_is_output = on;
           }}},

         {// --- options with arguments ---
          {{"command,c", "vi command"},
           {wex::cmdline::STRING,
            [&](const std::any& s) {
              m_data.control(
                wex::data::control().command(std::any_cast<std::string>(s)));
            }}},

          {{"config,j", "json config file"},
           {wex::cmdline::STRING,
            [&](const std::any& s) {
              wex::config::set_file(std::any_cast<std::string>(s));
            }}},

          {{"tag,t", "start at tag"},
           {wex::cmdline::STRING,
            [&](const std::any& s) {
              m_tag = std::any_cast<std::string>(s);
            }}},

          {{"tagfile,u", "use tagfile"},
           {wex::cmdline::STRING,
            [&](const std::any& s) {
              wex::ctags::open(std::any_cast<std::string>(s));
            }}},

          {{"scriptin,s", "script in (:so <arg> applied on any file opened)"},
           {wex::cmdline::STRING,
            [&](const std::any& s) {
              m_data.control(wex::data::control().command(
                ":so " + std::any_cast<std::string>(s)));
            }}},

          {{"scriptout,w", "script out append (echo to file <arg>)"},
           {wex::cmdline::STRING,
            [&](const std::any& s) {
              m_scriptout = std::any_cast<std::string>(s);
            }}},

          {{"output,X", "output (statusbar) append to file"},
           {wex::cmdline::STRING,
            [&](const std::any& s) {
              m_output = std::any_cast<std::string>(s);
            }}}},

         {{"files",
           "input file[:line number][:column number]\n"
           "or project file is -p was specified\n"
           "or executable file if -d was specified"},
          [&](const std::vector<std::string>& v) {
            for (const auto& f : v)
              m_files.emplace_back(f);
          }},
         true,
         "commandline")
         .parse(data) ||
      exit || !wex::app::OnInit())
  {
    return false;
  }

  if (list_lexers)
  {
    // code cannot be part of lambda, as OnInit is required
    for (const auto& l : wex::lexers::get()->get_lexers())
    {
      if (!l.display_lexer().empty())
        std::cout << l.display_lexer() << "\n";
    }

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
  if (!m_keep)
  {
    m_data.control(wex::data::control().command(""));
  }

  m_is_project = false;
  m_split      = -1;
  m_tag.clear();
}
