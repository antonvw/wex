////////////////////////////////////////////////////////////////////////////////
// Name:      printing.h
// Purpose:   Include file for wex::printing class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <memory> 
#include <vector> 
#include <wx/html/htmprint.h>
#include <wx/print.h> 

class wxStyledTextCtrl;

namespace wex
{
  /// Offers printing support.
  class printing
  {
  public:
    /// Returns the printing object.
    static printing* get(bool createOnDemand = true);

#if wxUSE_HTML & wxUSE_PRINTING_ARCHITECTURE
    /// Returns the html printer.
    auto* get_html_printer() {return m_HtmlPrinter.get();};
#endif

    /// Returns the printer.
    auto* get_printer() {return m_Printer.get();};

    /// Sets the object as the current one, returns the pointer 
    /// to the previous current object (both the parameter and returned value may be nullptr). 
    static printing* set(printing* printing);
  private:
    printing();

    std::unique_ptr<wxPrinter> m_Printer;
#if wxUSE_HTML
    std::unique_ptr<wxHtmlEasyPrinting> m_HtmlPrinter;
#endif

    static printing* m_Self;
  };

  // Offers a print out to be used by wxStyledTextCtrl.
  class printout : public wxPrintout
  {
  public:
    /// Constructor.
    printout(wxStyledTextCtrl* owner);

    /// Methods overridden from base class.
    virtual void GetPageInfo(int* minPage, int* maxPage, int* pageFrom, int* pageTo) override;
    virtual bool HasPage(int pageNum) override {
      return (pageNum >= 1 && pageNum <= (int)m_PageBreaks.size());};
    virtual void OnPreparePrinting() override;
    virtual bool OnPrintPage(int pageNum) override;
  private:
    void CountPages();
    void SetScale();
    wxRect m_PageRect, m_PrintRect;
    std::vector<int> m_PageBreaks;
    wxStyledTextCtrl* m_Owner;
  };
};
