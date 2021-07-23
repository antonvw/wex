////////////////////////////////////////////////////////////////////////////////
// Name:      version-dialog.cpp
// Purpose:   Implementation of wex::version_info_dialog
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/app.h>

#include <wex/frame.h>
#include <wex/version-dialog.h>

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
    m_about.SetDescription(wxString(info.external_libraries().str()));
  }
}

void wex::version_info_dialog::show()
{
  wxAboutBox(m_about, wxTheApp->GetTopWindow());
}
