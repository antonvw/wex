////////////////////////////////////////////////////////////////////////////////
// Name:      printout.cpp
// Purpose:   Implementation of wex::printout class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2023 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/core.h>
#include <wex/core/path.h>
#include <wex/syntax/printing.h>
#include <wex/syntax/printout.h>
#include <wx/stc/stc.h>

wex::printout::printout(wxStyledTextCtrl* stc)
  : wxPrintout(print_caption(path(stc->GetName().ToStdString())))
  , m_stc(stc)
{
}

void wex::printout::count_pages()
{
  if (GetDC() == nullptr)
    return;

  wxBusyCursor wait;

  m_page_breaks.clear();
  m_page_breaks.emplace_back(0); // a page break at pos 0
  int pos = 0;

  while (pos < m_stc->GetLength())
  {
    set_scale();

    pos = m_stc->FormatRange(
      false,
      pos,
      m_stc->GetLength(),
      GetDC(),
      GetDC(),
      m_print_rect,
      m_page_rect);

    m_page_breaks.emplace_back(pos);
  }
}

void wex::printout::GetPageInfo(
  int* minPage,
  int* maxPage,
  int* pageFrom,
  int* pageTo)
{
  *minPage  = 1;
  *maxPage  = m_page_breaks.size() - 1;
  *pageFrom = 1;
  *pageTo   = m_page_breaks.size() - 1;
}

void wex::printout::OnPreparePrinting()
{
  const double factor = 22.4;
  const auto   ppiScr = [&]
  {
    wxSize s;
    GetPPIScreen(&s.x, &s.y);
    return s;
  }();

  auto*  dlg_data = printing::get()->get_html_printer()->GetPageSetupData();
  wxSize page     = dlg_data->GetPaperSize();

  if (page.x == 0 || page.y == 0)
  {
    dlg_data->SetPaperSize(wxPAPER_A4);
    page = dlg_data->GetPaperSize();
  }

  page.x = static_cast<int>(page.x * ppiScr.x / factor);
  page.y = static_cast<int>(page.y * ppiScr.y / factor);

  m_page_rect = wxRect(0, 0, page.x, page.y);

  const auto margin_left =
    static_cast<int>(dlg_data->GetMarginTopLeft().x * ppiScr.x / factor);
  const auto margin_top =
    static_cast<int>(dlg_data->GetMarginTopLeft().y * ppiScr.y / factor);
  const auto margin_right =
    static_cast<int>(dlg_data->GetMarginBottomRight().x * ppiScr.x / factor);
  const auto margin_bottom =
    static_cast<int>(dlg_data->GetMarginBottomRight().y * ppiScr.y / factor);

  m_print_rect = wxRect(
    margin_left,
    margin_top,
    page.x - (margin_left + margin_right),
    page.y - (margin_top + margin_bottom));

  count_pages();
}

bool wex::printout::OnPrintPage(int pageNum)
{
  if (GetDC() == nullptr)
    return false;

  if (pageNum > static_cast<int>(m_page_breaks.size()))
  {
    assert(0);
    return false;
  }

  set_scale();

  m_stc->FormatRange(
    true,
    m_page_breaks[pageNum - 1],
    m_stc->GetTextLength(),
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

  // Print a header (line and text above).
  if (const auto& header(
        print_header(path_lexer(m_stc->GetName().ToStdString())));
      !header.empty())
  {
    const int line_above_top = 3;
    const int text_above_top = font.GetPixelSize().GetHeight() + line_above_top;

    GetDC()->DrawText(
      translate(header, pageNum, m_page_breaks.size() - 1),
      m_print_rect.GetTopLeft().x,
      m_print_rect.GetTopLeft().y - text_above_top);

    GetDC()->DrawLine(
      m_print_rect.GetTopLeft().x,
      m_print_rect.GetTopLeft().y - line_above_top,
      m_print_rect.GetBottomRight().x,
      m_print_rect.GetTopLeft().y - line_above_top);
  }

  // Print a footer (line and text below)
  if (const auto& footer(print_footer()); !footer.empty())
  {
    GetDC()->DrawLine(
      m_print_rect.GetTopLeft().x,
      m_print_rect.GetBottomRight().y,
      m_print_rect.GetBottomRight().x,
      m_print_rect.GetBottomRight().y);

    GetDC()->DrawText(
      translate(footer, pageNum, m_page_breaks.size() - 1),
      m_print_rect.GetBottomRight().x / 2,
      m_print_rect.GetBottomRight().y);
  }

  return true;
}

void wex::printout::set_scale()
{
  if (GetDC() == nullptr)
    return;

  wxSize ppiScr, ppiPrt;
  GetPPIScreen(&ppiScr.x, &ppiScr.y);

  if (ppiScr.x == 0)
  {
    // Most possible gues 96 dpi.
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

  const auto& dcSize = GetDC()->GetSize();
  wxSize      pageSize;
  GetPageSizePixels(&pageSize.x, &pageSize.y);

  const double factor  = 0.8;
  const auto   scale_x = static_cast<float>(factor * ppiPrt.x * dcSize.x) /
                       static_cast<float>(ppiScr.x * pageSize.x);
  const auto scale_y = static_cast<float>(factor * ppiPrt.y * dcSize.y) /
                       static_cast<float>(ppiScr.y * pageSize.y);

  GetDC()->SetUserScale(scale_x, scale_y);
}
