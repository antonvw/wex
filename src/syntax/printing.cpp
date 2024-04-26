////////////////////////////////////////////////////////////////////////////////
// Name:      printing.cpp
// Purpose:   Implementation of wex::printing class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2024 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/chrono.h>
#include <wex/core/core.h>
#include <wex/syntax/printing.h>

namespace wex
{
const std::string print_caption(const path& filename)
{
  return filename.string();
}

const std::string print_footer()
{
  return _("Page @PAGENUM@ of @PAGESCNT@").ToStdString();
}

const std::string print_header(const path_lexer& filename)
{
  if (filename.file_exists())
  {
    return find_tail(
      filename.string() + " " + filename.stat().get_modification_time_str(),
      filename.lexer().line_size());
  }

  return _("Printed").ToStdString() + ": " + now();
}
} // namespace wex

wex::printing::printing()
  : m_printer(std::make_unique<wxPrinter>())
  , m_html_printer(std::make_unique<wxHtmlEasyPrinting>())
{
  m_html_printer->SetFonts(wxEmptyString, wxEmptyString); // use defaults
  m_html_printer->GetPageSetupData()->SetMarginBottomRight(wxPoint(15, 5));
  m_html_printer->GetPageSetupData()->SetMarginTopLeft(wxPoint(15, 5));

  m_html_printer->SetHeader(print_header(path_lexer()));
  m_html_printer->SetFooter(print_footer());
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
  m_self             = printing;
  return old;
}
