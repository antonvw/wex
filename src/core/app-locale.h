////////////////////////////////////////////////////////////////////////////////
// Name:      app-locale.h
// Purpose:   Declaration of wex::translations class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wx/translation.h>

namespace wex
{
/// This class offers translations.
class translations : public wxTranslations
{
public:
  /// Adds catalogs.
  void add_catalogs(wxLanguage language);
};
}; // namespace wex
