////////////////////////////////////////////////////////////////////////////////
// Name:      printout.h
// Purpose:   Include file for wex::printout class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/core/types.h>
#include <wx/print.h>

class wxStyledTextCtrl;

namespace wex
{
// Offers a printout to be used by wxStyledTextCtrl.
class printout : public wxPrintout
{
public:
  /// Constructor.
  explicit printout(wxStyledTextCtrl* stc);

  /// Methods overridden from base class.
  void
  GetPageInfo(int* minPage, int* maxPage, int* pageFrom, int* pageTo) override;
  bool HasPage(int pageNum) override
  {
    return (pageNum >= 1 && pageNum <= static_cast<int>(m_page_breaks.size()));
  };
  void OnPreparePrinting() override;
  bool OnPrintPage(int pageNum) override;

private:
  void count_pages();
  void set_scale();

  wxRect m_page_rect, m_print_rect;

  ints_t            m_page_breaks;
  wxStyledTextCtrl* m_stc;
};
}; // namespace wex
