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

/// Offers a printing support.
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
  /// Gets the printer.
  wxHtmlEasyPrinting* GetPrinter() {return m_Printer;};
#endif

  /// Sets the object as the current one, returns the pointer 
  /// to the previous current object (both the parameter and returned value may be NULL). 
  static wxExPrinting* Set(wxExPrinting* printing);
private:
#if wxUSE_HTML & wxUSE_PRINTING_ARCHITECTURE
  wxHtmlEasyPrinting* m_Printer;
#endif

  static wxExPrinting* m_Self;
};
#endif
