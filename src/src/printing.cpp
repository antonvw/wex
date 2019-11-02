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

wex::printing* wex::printing::m_self = nullptr;

wex::printing::printing()
#if wxUSE_PRINTING_ARCHITECTURE
  : m_printer(std::make_unique<wxPrinter>())
#if wxUSE_HTML
  , m_html_printer(std::make_unique<wxHtmlEasyPrinting>())
#endif
#endif
{
#if wxUSE_HTML & wxUSE_PRINTING_ARCHITECTURE
  m_html_printer->SetFonts(wxEmptyString, wxEmptyString); // use defaults
  m_html_printer->GetPageSetupData()->SetMarginBottomRight(wxPoint(15, 5));
  m_html_printer->GetPageSetupData()->SetMarginTopLeft(wxPoint(15, 5));

  m_html_printer->SetHeader(print_header(path()));
  m_html_printer->SetFooter(print_footer());
#endif
}

wex::printing* wex::printing::get(bool createOnDemand)
{
  if (m_self == nullptr && createOnDemand)
  {
    m_self = new printing();
  }

  return m_self;
}

wex::printing* wex::printing::set(printing* printing)
{
  wex::printing* old = m_self;
  m_self = printing;
  return old;
}

#if wxUSE_PRINTING_ARCHITECTURE
wex::printout::printout(wxStyledTextCtrl* owner)
  : wxPrintout(print_caption(owner->GetName().ToStdString()))
  , m_page_rect()
  , m_print_rect()
  , m_page_breaks()
  , m_owner(owner)
{
}

void wex::printout::count_pages()
{
  if (GetDC() == nullptr) return;

  wxBusyCursor wait;

  m_page_breaks.clear();
  m_page_breaks.emplace_back(0); // a page break at pos 0
  int pos = 0;

  while (pos < m_owner->GetLength())
  {
    set_scale();

    pos = m_owner->FormatRange(
      false,
      pos,
      m_owner->GetLength(),
      GetDC(),
      GetDC(),
      m_print_rect,
      m_page_rect);

    m_page_breaks.emplace_back(pos);
  }
}

void wex::printout::GetPageInfo(
  int* minPage, int* maxPage, int* pageFrom, int* pageTo)
{
  *minPage = 1;
  *maxPage = m_page_breaks.size() - 1;
  *pageFrom = 1;
  *pageTo = m_page_breaks.size() - 1;
}

void wex::printout::OnPreparePrinting()
{
  const double factor = 22.4;
  wxSize ppiScr;
  GetPPIScreen(&ppiScr.x, &ppiScr.y);

  wxPageSetupDialogData* dlg_data = printing::get()->get_html_printer()->GetPageSetupData();
  wxSize page = dlg_data->GetPaperSize();

  if (page.x == 0 || page.y == 0)
  {
    dlg_data->SetPaperSize(wxPAPER_A4);
    page = dlg_data->GetPaperSize();
  }

  page.x = (int)(page.x * ppiScr.x / factor);
  page.y = (int)(page.y * ppiScr.y / factor);

  m_page_rect = wxRect(0, 0, page.x, page.y);

  int left = (int)(dlg_data->GetMarginTopLeft().x * ppiScr.x / factor);
  int top = (int)(dlg_data->GetMarginTopLeft().y * ppiScr.y / factor);
  int right = (int)(dlg_data->GetMarginBottomRight().x * ppiScr.x / factor);
  int bottom = (int)(dlg_data->GetMarginBottomRight().y * ppiScr.y / factor);

  m_print_rect = wxRect(
    left,
    top,
    page.x - (left + right),
    page.y - (top + bottom));

  count_pages();
}

bool wex::printout::OnPrintPage(int pageNum)
{
  if (GetDC() == nullptr) return false;

  if (pageNum > (int)m_page_breaks.size())
  {
    assert(0);
    return false;
  }

  set_scale();

  m_owner->FormatRange(
    true,
    m_page_breaks[pageNum - 1],
    m_owner->GetTextLength(),
    GetDC(),
    GetDC(),
    m_print_rect,
    m_page_rect);

  wxFont font = *wxNORMAL_FONT;
  font.SetWeight(wxFONTWEIGHT_BOLD);
  GetDC()->SetFont(font);
  GetDC()->SetTextForeground(*wxBLACK);
  GetDC()->SetTextBackground(*wxWHITE);
  GetDC()->SetPen(*wxBLACK_PEN);

  // Print a header.
  const std::string header = print_header(m_owner->GetName().ToStdString());
  if (!header.empty())
  {
    const int text_from_top = 23;
    const int line_from_top = 8;
    
    GetDC()->DrawText(
      translate(header, pageNum, m_page_breaks.size() - 1),
      m_print_rect.GetTopLeft().x,
      m_print_rect.GetTopLeft().y - text_from_top);

    GetDC()->DrawLine(
      m_print_rect.GetTopLeft().x,
      m_print_rect.GetTopLeft().y - line_from_top,
      m_print_rect.GetBottomRight().x,
      m_print_rect.GetTopLeft().y - line_from_top);
  }

  // Print a footer
  const std::string footer = print_footer();
  if (!footer.empty())
  {
    GetDC()->DrawText(
      translate(footer, pageNum, m_page_breaks.size() - 1),
      m_print_rect.GetBottomRight().x / 2,
      m_print_rect.GetBottomRight().y);

    GetDC()->DrawLine(
      m_print_rect.GetTopLeft().x,
      m_print_rect.GetBottomRight().y,
      m_print_rect.GetBottomRight().x,
      m_print_rect.GetBottomRight().y);
  }

  return true;
}

void wex::printout::set_scale()
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
