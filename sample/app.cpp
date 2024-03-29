////////////////////////////////////////////////////////////////////////////////
// Name:      app.cpp
// Purpose:   Implementation of wex sample app class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include "app.h"
#include "frame.h"

wxIMPLEMENT_APP(app);

bool app::OnInit()
{
  SetAppName("wex-sample");

  if (wex::data::cmdline c(argc, argv);
      !wex::cmdline(
         {// --- boolean options ---
          {{"ex", "ex mode"},
           [&](bool on)
           {
             if (!on)
               return;
             m_data.flags(
               wex::data::stc::window_t().set(wex::data::stc::WIN_EX),
               wex::data::control::OR);
           }},

          {{"project,p", "open specified files as projects"},
           [&](bool on)
           {
             m_data.flags(
               wex::data::stc::window_t().set(wex::data::stc::WIN_IS_PROJECT),
               wex::data::control::OR);
           }}},

         {// --- options with arguments ---
          {{"scriptin,s", "script in (:so <arg> applied on any file opened)"},
           {wex::cmdline::STRING,
            [&](const std::any& s)
            {
              m_data.control(wex::data::control().command(
                ":so " + std::any_cast<std::string>(s)));
            }}}},
         {{"files", "input file[:line number][:column number]"},
          [&](const std::vector<std::string>& v)
          {
            std::transform(
              v.begin(),
              v.end(),
              std::back_inserter(m_files),
              [](const auto& v)
              {
                return wex::path(v);
              });
          }})
         .parse(c) ||
      !wex::del::app::OnInit())
  {
    return false;
  }

  auto* f = new frame(this);
  f->Show(true);
  f->update();

  wex::log::status("Locale")
    << get_locale().GetName().ToStdString() << "dir" << get_catalog_dir();

  return true;
}
