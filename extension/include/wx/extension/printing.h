////////////////////////////////////////////////////////////////////////////////
// Name:      printing.h
// Purpose:   Include file for wxExPrinting class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2011 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EXPRINTING_H
#define _EXPRINTING_H

#include <vector> 
#include <wx/html/htmprint.h>
#include <wx/print.h> 

/// Offers printing support.
class WXDLLIMPEXP_BASE wxExPrinting
{
public:
  // Destructor (not for Doxy).
 ~wxExPrinting();
 
  /// Returns the printing object.
  static wxExPrinting* Get(bool createOnDemand = true);

#if wxUSE_HTML & wxUSE_PRINTING_ARCHITECTURE
  /// Gets the html printer.
  wxHtmlEasyPrinting* GetHtmlPrinter() {return m_HtmlPrinter;};
#endif

#if wxUSE_PRINTING_ARCHITECTURE
  /// Gets the printer.
  wxPrinter* GetPrinter() {return m_Printer;};
#endif

  /// Sets the object as the current one, returns the pointer 
  /// to the previous current object (both the parameter and returned value may be NULL). 
  static wxExPrinting* Set(wxExPrinting* printing);
private:
  wxExPrinting();

#if wxUSE_HTML & wxUSE_PRINTING_ARCHITECTURE
  wxHtmlEasyPrinting* m_HtmlPrinter;
#endif

#if wxUSE_PRINTING_ARCHITECTURE
  wxPrinter* m_Printer;
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
  int m_CurrentPage;
  std::vector<int> m_PageBreaks;
  wxStyledTextCtrl* m_Owner;
};
#endif
#endif
