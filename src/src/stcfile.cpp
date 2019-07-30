////////////////////////////////////////////////////////////////////////////////
// Name:      stcfile.cpp
// Purpose:   Implementation of class wex::stc_file
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <pugixml.hpp>
#include <wex/stcfile.h>
#include <wex/filedlg.h>
#include <wex/lexers.h>
#include <wex/log.h>
#include <wex/path.h>
#include <wex/stc.h>
#include <wex/util.h> // for STAT_ etc.

void CheckWellFormed(wex::stc* stc, const wex::path& fn)
{
  if (fn.lexer().language() == "xml")
  {
    if (const pugi::xml_parse_result result = 
      pugi::xml_document().load_file(fn.string().c_str());
      !result)
    {
      wex::xml_error(fn, &result, stc);
    }
  }
}

wex::stc_file::stc_file(stc* stc, const std::string& filename)
  : file(filename)
  , m_STC(stc)
{
}

bool wex::stc_file::do_file_load(bool synced)
{
  file_dialog dlg(this);
   
  if (
    get_contents_changed() &&
    dlg.show_modal_if_changed() == wxID_CANCEL)
  {
    return false;
  }

  // Synchronizing by appending only new data only works for log files.
  // Other kind of files might get new data anywhere inside the file,
  // we cannot sync that by keeping pos. 
  // Also only do it for reasonably large files.
  const bool isLog = (get_filename().extension().find(".log") == 0);
  
  m_STC->use_modification_markers(false);

  read_from_file(
    synced &&
    isLog &&
    m_STC->GetTextLength() > 1024,
    dlg.hexmode() | m_STC->data().flags().test(stc_data::WIN_HEX));

  if (!synced)
  {
    // read_from_file might already have set the lexer using a modeline.
    if (m_STC->get_lexer().scintilla_lexer().empty())
    {
      m_STC->get_lexer().set(get_filename().lexer(), true);
    }

    log::status(_("Opened")) << get_filename();
    log::verbose("opened", 1) << get_filename();
  }
  
  m_STC->properties_message(path::status_t().set(synced ? path::STAT_SYNC: 0));
  m_STC->use_modification_markers(true);
  
  CheckWellFormed(m_STC, get_filename());
  
  return true;
}

void wex::stc_file::do_file_new()
{
  m_STC->SetName(get_filename().string());
  m_STC->properties_message();
  m_STC->clear();
  m_STC->get_lexer().set(get_filename().lexer(), true); // allow fold
}

void wex::stc_file::do_file_save(bool save_as)
{
  if (m_STC->get_hexmode().is_active())
  {
    write(m_STC->get_hexmode().buffer());
  }
  else
  {
    const auto& buffer(m_STC->GetTextRaw());
    write(buffer.data(), buffer.length());
  }
  
  if (save_as)
  {
    m_STC->SetReadOnly(get_filename().is_readonly());
    m_STC->get_lexer().set(get_filename().lexer());
    m_STC->SetName(get_filename().string());
  }
  
  m_STC->marker_delete_all_change();
  
  log::status(_("Saved")) << get_filename();
  log::verbose("saved", 1) << get_filename();
  
  CheckWellFormed(m_STC, get_filename());
}

bool wex::stc_file::get_contents_changed() const 
{
  return m_STC->GetModify();
}

void wex::stc_file::read_from_file(bool get_only_new_data, bool hexmode)
{
  const bool pos_at_end = (m_STC->GetCurrentPos() >= m_STC->GetTextLength() - 1);

  int startPos, endPos;
  m_STC->GetSelection(&startPos, &endPos);

  std::streampos offset = 0;

  if (m_PreviousLength < m_STC->get_filename().stat().st_size && get_only_new_data)
  {
    offset = m_PreviousLength;
  }

  if (offset == std::streampos(0))
  {
    m_STC->clear();
  }

  m_PreviousLength = m_STC->get_filename().stat().st_size;

  if (const auto buffer = read(offset);
    !m_STC->get_hexmode().is_active() && !hexmode)
  {
    m_STC->Allocate(buffer->size());
    
    get_only_new_data ? 
      m_STC->AppendTextRaw((const char *)buffer->data(), buffer->size()):
      m_STC->AddTextRaw((const char *)buffer->data(), buffer->size());
  }
  else
  {
    if (!m_STC->get_hexmode().is_active())
    {
      m_STC->get_hexmode().set(true, false);
    }
    
    m_STC->get_hexmode().append_text(*buffer);
  }

  if (get_only_new_data)
  {
    if (pos_at_end)
    {
      m_STC->DocumentEnd();
    }
  }
  else
  {
    m_STC->guess_type();
    m_STC->DocumentStart();
  }

  if (startPos != endPos)
  {
    m_STC->SetSelection(startPos, endPos);
  }
  
  m_STC->EmptyUndoBuffer();
}

void wex::stc_file::reset_contents_changed()
{
  m_STC->SetSavePoint();
}
