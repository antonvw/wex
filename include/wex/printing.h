////////////////////////////////////////////////////////////////////////////////
// Name:      printing.h
// Purpose:   Include file for wex::printing class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <memory>
#include <vector>
#include <wex/path-lexer.h>
#include <wx/html/htmprint.h>
#include <wx/print.h>

class wxStyledTextCtrl;

namespace wex
{
  /// Adds a caption.
  const std::string print_caption(const path& filename);

  /// You can use macros in PrintFooter and in PrintHeader:
  ///   \@PAGENUM\@ is replaced by page number
  ///   \@PAGESCNT\@ is replaced by total number of pages
  const std::string print_footer();

  /// Adds a header.
  const std::string print_header(const path_lexer& filename);

  /// Offers printing support.
  class printing
  {
  public:
    /// Static interface.

    /// Returns the printing object.
    static printing* get(bool createOnDemand = true);

    /// Sets the object as the current one, returns the pointer
    /// to the previous current object (both the parameter and
    /// returned value may be nullptr).
    static printing* set(printing* printing);

    /// Other methods.

    /// Returns the html printer.
    auto* get_html_printer() { return m_html_printer.get(); };

    /// Returns the printer.
    auto* get_printer() { return m_printer.get(); };

  private:
    printing();

    std::unique_ptr<wxPrinter>          m_printer;
    std::unique_ptr<wxHtmlEasyPrinting> m_html_printer;

    static printing* m_self;
  };

  // Offers a print out to be used by wxStyledTextCtrl.
  class printout : public wxPrintout
  {
  public:
    /// Constructor.
    printout(wxStyledTextCtrl* owner);

    /// Methods overridden from base class.
    void GetPageInfo(int* minPage, int* maxPage, int* pageFrom, int* pageTo)
      override;
    bool HasPage(int pageNum) override
    {
      return (pageNum >= 1 && pageNum <= (int)m_page_breaks.size());
    };
    void OnPreparePrinting() override;
    bool OnPrintPage(int pageNum) override;

  private:
    void count_pages();
    void set_scale();

    wxRect m_page_rect, m_print_rect;

    std::vector<int>  m_page_breaks;
    wxStyledTextCtrl* m_owner;
  };
}; // namespace wex
