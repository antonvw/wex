////////////////////////////////////////////////////////////////////////////////
// Name:      printing.cpp
// Purpose:   Implementation of wex::printing class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/stc/stc.h>
#include <wex/printing.h>
#include <wex/path.h>
#include <wex/util.h>

wex::printing* wex::printing::m_Self = nullptr;

wex::printing::printing()
#if wxUSE_PRINTING_ARCHITECTURE
  : m_Printer(std::make_unique<wxPrinter>())
#if wxUSE_HTML
  , m_HtmlPrinter(std::make_unique<wxHtmlEasyPrinting>())
#endif
#endif
{
#if wxUSE_HTML & wxUSE_PRINTING_ARCHITECTURE
  m_HtmlPrinter->SetFonts(wxEmptyString, wxEmptyString); // use defaults
  m_HtmlPrinter->GetPageSetupData()->SetMarginBottomRight(wxPoint(15, 5));
  m_HtmlPrinter->GetPageSetupData()->SetMarginTopLeft(wxPoint(15, 5));

  m_HtmlPrinter->SetHeader(print_header(path()));
  m_HtmlPrinter->SetFooter(print_footer());
#endif
}

wex::printing* wex::printing::Get(bool createOnDemand)
{
  if (m_Self == nullptr && createOnDemand)
  {
    m_Self = new printing();
  }

  return m_Self;
}

wex::printing* wex::printing::Set(printing* printing)
{
  wex::printing* old = m_Self;
  m_Self = printing;
  return old;
}

#if wxUSE_PRINTING_ARCHITECTURE
wex::printout::printout(wxStyledTextCtrl* owner)
  : wxPrintout(print_caption(owner->GetName().ToStdString()))
  , m_PageRect()
  , m_PrintRect()
  , m_PageBreaks()
  , m_Owner(owner)
{
}

void wex::printout::CountPages()
{
  if (GetDC() == nullptr) return;

  wxBusyCursor wait;

  m_PageBreaks.clear();
  m_PageBreaks.emplace_back(0); // a page break at pos 0
  int pos = 0;

  while (pos < m_Owner->GetLength())
  {
    SetScale();

    pos = m_Owner->FormatRange(
      false,
      pos,
      m_Owner->GetLength(),
      GetDC(),
      GetDC(),
      m_PrintRect,
      m_PageRect);

    m_PageBreaks.emplace_back(pos);
  }
}

void wex::printout::GetPageInfo(
  int* minPage, int* maxPage, int* pageFrom, int* pageTo)
{
  *minPage = 1;
  *maxPage = m_PageBreaks.size() - 1;
  *pageFrom = 1;
  *pageTo = m_PageBreaks.size() - 1;
}

void wex::printout::OnPreparePrinting()
{
  const double factor = 22.4;
  wxSize ppiScr;
  GetPPIScreen(&ppiScr.x, &ppiScr.y);

  wxPageSetupDialogData* dlg_data = printing::Get()->GetHtmlPrinter()->GetPageSetupData();
  wxSize page = dlg_data->GetPaperSize();

  if (page.x == 0 || page.y == 0)
  {
    dlg_data->SetPaperSize(wxPAPER_A4);
    page = dlg_data->GetPaperSize();
  }

  page.x = (int)(page.x * ppiScr.x / factor);
  page.y = (int)(page.y * ppiScr.y / factor);

  m_PageRect = wxRect(0, 0, page.x, page.y);

  int left = (int)(dlg_data->GetMarginTopLeft().x * ppiScr.x / factor);
  int top = (int)(dlg_data->GetMarginTopLeft().y * ppiScr.y / factor);
  int right = (int)(dlg_data->GetMarginBottomRight().x * ppiScr.x / factor);
  int bottom = (int)(dlg_data->GetMarginBottomRight().y * ppiScr.y / factor);

  m_PrintRect = wxRect(
    left,
    top,
    page.x - (left + right),
    page.y - (top + bottom));

  CountPages();
}

bool wex::printout::OnPrintPage(int pageNum)
{
  if (GetDC() == nullptr) return false;

  if (pageNum > (int)m_PageBreaks.size())
  {
    wxFAIL;
    return false;
  }

  SetScale();

  m_Owner->FormatRange(
    true,
    m_PageBreaks[pageNum - 1],
    m_Owner->GetTextLength(),
    GetDC(),
    GetDC(),
    m_PrintRect,
    m_PageRect);

  wxFont font = *wxNORMAL_FONT;
  font.SetWeight(wxFONTWEIGHT_BOLD);
  GetDC()->SetFont(font);
  GetDC()->SetTextForeground(*wxBLACK);
  GetDC()->SetTextBackground(*wxWHITE);
  GetDC()->SetPen(*wxBLACK_PEN);

  // Print a header.
  const std::string header = print_header(m_Owner->GetName().ToStdString());
  if (!header.empty())
  {
    const int text_from_top = 23;
    const int line_from_top = 8;
    
    GetDC()->DrawText(
      translate(header, pageNum, m_PageBreaks.size() - 1),
      m_PrintRect.GetTopLeft().x,
      m_PrintRect.GetTopLeft().y - text_from_top);

    GetDC()->DrawLine(
      m_PrintRect.GetTopLeft().x,
      m_PrintRect.GetTopLeft().y - line_from_top,
      m_PrintRect.GetBottomRight().x,
      m_PrintRect.GetTopLeft().y - line_from_top);
  }

  // Print a footer
  const std::string footer = print_footer();
  if (!footer.empty())
  {
    GetDC()->DrawText(
      translate(footer, pageNum, m_PageBreaks.size() - 1),
      m_PrintRect.GetBottomRight().x / 2,
      m_PrintRect.GetBottomRight().y);

    GetDC()->DrawLine(
      m_PrintRect.GetTopLeft().x,
      m_PrintRect.GetBottomRight().y,
      m_PrintRect.GetBottomRight().x,
      m_PrintRect.GetBottomRight().y);
  }

  return true;
}

void wex::printout::SetScale()
{
  if (GetDC() == nullptr) return;

  wxSize ppiScr, ppiPrt;
  GetPPIScreen(&ppiScr.x, &ppiScr.y);

  if (ppiScr.x == 0)
  {
    // Most possible gues  96 dpi.
    ppiScr.x = 96;
    ppiScr.y = 96;
  }

  GetPPIPrinter(&ppiPrt.x, &ppiPrt.y);

  if (ppiPrt.x == 0)
  {
    // Scaling factor to 1.
    ppiPrt.x = ppiScr.x;
    ppiPrt.y = ppiScr.y;
  }

  const wxSize dcSize = GetDC()->GetSize();
  wxSize pageSize;
  GetPageSizePixels(&pageSize.x, &pageSize.y);

  const double factor = 0.8;
  const float scale_x = (float)(factor * ppiPrt.x * dcSize.x) / (float)(ppiScr.x * pageSize.x);
  const float scale_y = (float)(factor * ppiPrt.y * dcSize.y) / (float)(ppiScr.y * pageSize.y);

  GetDC()->SetUserScale(scale_x, scale_y);
}
#endif // wxUSE_PRINTING_ARCHITECTURE
