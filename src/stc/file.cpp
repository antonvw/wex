////////////////////////////////////////////////////////////////////////////////
// Name:      stc/file.cpp
// Purpose:   Implementation of class wex::stc_file
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <thread>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/config.h>
#include <wex/defs.h>
#include <wex/ex-stream.h>
#include <wex/file-dialog.h>
#include <wex/path-lexer.h>
#include <wex/stc-file.h>
#include <wex/stc.h>

//#define USE_THREAD 1

#define FILE_POST(ACTION)                                                 \
  wxCommandEvent event(wxEVT_COMMAND_MENU_SELECTED, ID_EDIT_FILE_ACTION); \
  event.SetInt(ACTION);                                                   \
  wxPostEvent(m_stc, event);

namespace wex
{
#ifdef USE_THREAD
// from wxWidgets/src/stc/scintilla/include/ILexer.h
class ILoader
{
public:
  virtual int   Release()                       = 0;
  virtual int   AddData(char* data, int length) = 0;
  virtual void* ConvertToDocument()             = 0;
};

class loader : public ILoader
{
public:
  int   Release() override { return 0; };
  int   AddData(char* data, int length) override { return 0; };
  void* ConvertToDocument() override { return nullptr; };
};
#endif
} // namespace wex

wex::stc_file::stc_file(stc* stc, const wex::path& path)
  : file(path)
  , m_stc(stc)
{
}

wex::stc_file::~stc_file() {}

bool wex::stc_file::do_file_load(bool synced)
{
  file_dialog dlg(this);

  if (is_contents_changed() && dlg.show_modal_if_changed() == wxID_CANCEL)
  {
    return false;
  }

  m_stc->use_modification_markers(false);
  m_stc->keep_event_data(synced);

  const bool hexmode =
    dlg.is_hexmode() || m_stc->data().flags().test(data::stc::WIN_HEX);

  const std::streampos offset =
    m_previous_size < m_stc->path().stat().st_size &&
        m_stc->data().event().is_synced_log() ?
      m_previous_size :
      std::streampos(0);

  if (offset == std::streampos(0))
  {
    m_stc->clear();
  }

  m_previous_size = m_stc->path().stat().st_size;

  if (
    m_stc->path().stat().st_size > config("stc.max.Size visual").get(10000000))
  {
    m_stc->visual(false);
  }

#ifdef USE_THREAD
  std::thread t(
    [&]
    {
#endif
      if (!m_stc->is_visual())
      {
        ex_stream()->stream(*this);
      }
      else if (const auto buffer(read(offset)); buffer != nullptr)
      {
        if (!m_stc->get_hexmode().is_active() && !hexmode)
        {
#ifdef USE_THREAD
          loader* load =
            reinterpret_cast<loader*>(m_stc->CreateLoader(buffer->size()));
#endif
          m_stc->append_text(*buffer);
          m_stc->DocumentStart();
        }
        else
        {
          if (!m_stc->get_hexmode().is_active())
          {
            m_stc->get_hexmode().set(true, false);
          }

          m_stc->get_hexmode().append_text(*buffer);
        }
      }
      else
      {
        m_stc->SetText("READ ERROR");
      }

      const int action =
        m_stc->data().event().is_synced() ? FILE_LOAD_SYNC : FILE_LOAD;
      FILE_POST(action);
#ifdef USE_THREAD
    });
  t.detach();
#endif

  return true;
}

void wex::stc_file::do_file_new()
{
  m_stc->SetName(path().string());
  m_stc->properties_message();

  if (m_stc->data().control().command().empty())
  {
    m_stc->clear();
  }

  m_stc->get_lexer().set(path_lexer(path()).lexer(),
                         true); // allow fold
}

void wex::stc_file::do_file_save(bool save_as)
{
  m_stc->SetReadOnly(true); // prevent changes during saving

  if (!m_stc->is_visual())
  {
    if (ex_stream()->write())
    {
      FILE_POST(save_as ? FILE_SAVE_AS : FILE_SAVE);
    }
  }
  else if (m_stc->get_hexmode().is_active())
  {
#ifdef USE_THREAD
    std::thread t(
      [&]
      {
#endif
        if (write(m_stc->get_hexmode().buffer()))
        {
          FILE_POST(save_as ? FILE_SAVE_AS : FILE_SAVE);
        }
#ifdef USE_THREAD
      });
    t.detach();
#endif
  }
  else
  {
#ifdef USE_THREAD
    std::thread t(
      [&]
      {
#endif
        if (write(m_stc->get_text()))
        {
          FILE_POST(save_as ? FILE_SAVE_AS : FILE_SAVE);
        }
#ifdef USE_THREAD
      });
    t.detach();
#endif
  }
}

wex::ex_stream* wex::stc_file::ex_stream()
{
  return m_stc->get_vi().ex_stream();
}

const wex::ex_stream* wex::stc_file::ex_stream() const
{
  return m_stc->get_vi().ex_stream();
}

bool wex::stc_file::is_contents_changed() const
{
  return m_stc->IsModified();
}

void wex::stc_file::reset_contents_changed()
{
  m_stc->SetSavePoint();
}
