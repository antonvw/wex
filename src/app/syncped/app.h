////////////////////////////////////////////////////////////////////////////////
// Name:      app.h
// Purpose:   Declaration of class 'app'
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/app.h>
#include <wex/path.h>
#include <wex/stc-data.h>

class app : public wex::app
{
public:
  auto& data() { return m_data; };
  auto& get_files() const { return m_files; };
  auto  get_is_debug() const { return m_is_debug; };
  auto  get_is_echo() const { return m_is_echo; };
  auto  get_is_project() const { return m_is_project; };
  auto  get_is_stdin() const { return m_is_stdin; };
  auto& get_scriptin() { return m_scriptin; };
  auto& get_scriptout() { return m_scriptout; };
  auto  get_split() const { return m_split; };
  auto& get_tag() const { return m_tag; };

  void reset();

private:
#ifdef __WXOSX__
  void MacOpenFiles(const wxArrayString& fileNames) override;
#endif

  bool OnInit() override;

  std::string m_scriptin, m_scriptout, m_tag;

  std::vector<wex::path> m_files;

  bool m_is_echo{false}, m_is_debug{false}, m_is_project{false},
    m_is_stdin{false};

  int           m_split;
  wex::data::stc m_data;
};
