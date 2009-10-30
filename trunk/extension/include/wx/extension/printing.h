/******************************************************************************\
* File:          printing.h
* Purpose:       Include file for wxExPrinting class
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EXPRINTING_H
#define _EXPRINTING_H

#include <wx/html/htmprint.h>
#include <wx/print.h> 

/// Offers printing support.
class wxExPrinting
{
public:
  /// Constructor.
  wxExPrinting();

  /// Destructor.
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
#if wxUSE_HTML & wxUSE_PRINTING_ARCHITECTURE
  wxHtmlEasyPrinting* m_HtmlPrinter;
#endif

#if wxUSE_PRINTING_ARCHITECTURE
  wxPrinter* m_Printer;
#endif

  static wxExPrinting* m_Self;
};

#if wxUSE_PRINTING_ARCHITECTURE
class wxExSTC;

// Offers a print out to be used by wxExSTC.
class wxExPrintout : public wxPrintout
{
public:
  /// Constructor.
  wxExPrintout(wxExSTC* owner);
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
  vector<int> m_PageBreaks;
  wxExSTC* m_Owner;
};
#endif
#endif
