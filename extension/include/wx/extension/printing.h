////////////////////////////////////////////////////////////////////////////////
// Name:      printing.h
// Purpose:   Include file for wxExPrinting class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <memory> 
#include <vector> 
#include <wx/html/htmprint.h>
#include <wx/print.h> 

/// Offers printing support.
class WXDLLIMPEXP_BASE wxExPrinting
{
public:
  /// Returns the printing object.
  static wxExPrinting* Get(bool createOnDemand = true);

#if wxUSE_HTML & wxUSE_PRINTING_ARCHITECTURE
  /// Returns the html printer.
  auto* GetHtmlPrinter() {return m_HtmlPrinter.get();};
#endif

#if wxUSE_PRINTING_ARCHITECTURE
  /// Returns the printer.
  auto* GetPrinter() {return m_Printer.get();};
#endif

  /// Sets the object as the current one, returns the pointer 
  /// to the previous current object (both the parameter and returned value may be nullptr). 
  static wxExPrinting* Set(wxExPrinting* printing);
private:
  wxExPrinting();

#if wxUSE_PRINTING_ARCHITECTURE
  std::unique_ptr<wxPrinter> m_Printer;
#if wxUSE_HTML
  std::unique_ptr<wxHtmlEasyPrinting> m_HtmlPrinter;
#endif
#endif

  static wxExPrinting* m_Self;
};

#if wxUSE_PRINTING_ARCHITECTURE
class wxStyledTextCtrl;

// Offers a print out to be used by wxStyledTextCtrl.
class WXDLLIMPEXP_BASE wxExPrintout : public wxPrintout
{
public:
  /// Constructor.
  wxExPrintout(wxStyledTextCtrl* owner);
private:
  void CountPages();
  void GetPageInfo(int* minPage, int* maxPage, int* pageFrom, int* pageTo);
  bool HasPage(int pageNum) {
    return (pageNum >= 1 && pageNum <= (int)m_PageBreaks.size());};
  void OnPreparePrinting();
  bool OnPrintPage(int pageNum);
  void SetScale(wxDC *dc);
  wxRect m_PageRect, m_PrintRect;
  std::vector<int> m_PageBreaks;
  wxStyledTextCtrl* m_Owner;
};
#endif
