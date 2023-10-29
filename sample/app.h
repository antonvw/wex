////////////////////////////////////////////////////////////////////////////////
// Name:      app.h
// Purpose:   Declaration of wex sample class app
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/wex.h>

/// Derive your application from wex::del::app.
class app : public wex::del::app
{
public:
  auto& data() { return m_data; }
  auto& get_files() { return m_files; }

private:
  bool OnInit() final;

  std::vector<wex::path> m_files;
  wex::data::stc         m_data;
};
