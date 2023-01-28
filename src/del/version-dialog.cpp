////////////////////////////////////////////////////////////////////////////////
// Name:      version-dialog.cpp
// Purpose:   Implementation of wex::version_info_dialog
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <sstream>

#include <wx/app.h>
#include <wx/utils.h>

#include <boost/version.hpp>
#include <ctags/main/ctags.h>
#include <pugixml.hpp>

#include <wex/del/version-dialog.h>

const std::stringstream wex::external_libraries()
{
  std::stringstream ss;

  ss << wex::get_version_info().description() << ": "
     << wex::get_version_info().get()
     << "\n"

     // boost
     << "Boost library: " << BOOST_VERSION / 100000 << "." // major version
     << BOOST_VERSION / 100 % 1000 << "."                  // minor version
     << BOOST_VERSION % 100                                // patch level
     << "\n"

     // pugi
     << "pugixml library: " << PUGIXML_VERSION / 1000 << "." // major version
     << PUGIXML_VERSION % 1000 / 10 << "."                   // minor version
     << PUGIXML_VERSION % 10                                 // patch level
     << "\n"

     // ctags
     << PROGRAM_NAME << ": " << PROGRAM_VERSION
     << "\n"

     // wxWidgets
     << wxGetLibraryVersionInfo().GetDescription().c_str();

  return ss;
}

wex::about_info& wex::about_info::icon(const wxIcon& rhs)
{
  SetIcon(rhs);
  return *this;
}

wex::about_info& wex::about_info::description(const std::string& rhs)
{
  SetDescription(rhs);
  return *this;
}

wex::about_info& wex::about_info::developer(const std::string& rhs)
{
  AddDeveloper(rhs);
  return *this;
}

wex::about_info& wex::about_info::licence(const std::string& rhs)
{
  SetLicence(rhs);
  return *this;
}

wex::about_info& wex::about_info::website(const std::string& rhs)
{
  SetWebSite(rhs);
  return *this;
}

wex::version_info_dialog::version_info_dialog(const about_info& about)
  : version_info_dialog(get_version_info(), about)
{
}

wex::version_info_dialog::version_info_dialog(
  const version_info& info,
  const about_info&   about)
  : m_about(about)
{
  m_about.SetVersion(info.get());

  if (!m_about.HasCopyright())
  {
    m_about.SetCopyright(info.copyright());
  }

  if (!m_about.HasDescription())
  {
    if (!info.description().empty())
    {
      m_about.SetDescription(info.description());
    }
    else
    {
      m_about.SetDescription(wxString(external_libraries().str()));
    }
  }
}

void wex::version_info_dialog::show()
{
  wxAboutBox(m_about, wxTheApp->GetTopWindow());
}
