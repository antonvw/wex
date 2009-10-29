/******************************************************************************\
* File:          app.cpp
* Purpose:       Implementation of wxExApp class
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 2009 Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#include <wx/extension/printing.h>
#include <wx/extension/util.h>

wxExPrinting* wxExPrinting::m_Self = NULL;

wxExPrinting::wxExPrinting()
{
#if wxUSE_HTML & wxUSE_PRINTING_ARCHITECTURE
  m_Printer = new wxHtmlEasyPrinting();

  m_Printer->SetFonts(wxEmptyString, wxEmptyString); // use defaults
  m_Printer->GetPageSetupData()->SetMarginBottomRight(wxPoint(15, 5));
  m_Printer->GetPageSetupData()->SetMarginTopLeft(wxPoint(15, 5));

  m_Printer->SetHeader(wxExPrintHeader(wxFileName()));
  m_Printer->SetFooter(wxExPrintFooter());
#endif
}

wxExPrinting::~wxExPrinting()
{
#if wxUSE_HTML & wxUSE_PRINTING_ARCHITECTURE
  delete m_Printer;
#endif
}

wxExPrinting* wxExPrinting::Get(bool createOnDemand)
{
  if (m_Self == NULL)
  {
    m_Self = new wxExPrinting();
  }

  return m_Self;
}

wxExPrinting* wxExPrinting::Set(wxExPrinting* printing)
{
  wxExPrinting* old = m_Self;
  m_Self = printing;
  return old;
}
