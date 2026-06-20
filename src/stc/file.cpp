////////////////////////////////////////////////////////////////////////////////
// Name:      stc/file.cpp
// Purpose:   Implementation of class wex::stc_file
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020-2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/config.h>
#include <wex/core/log.h>
#include <wex/ex/ex-stream.h>
#include <wex/lsp/client.h>
#include <wex/stc/file.h>
#include <wex/stc/stc.h>
#include <wex/syntax/path-lexer.h>
#include <wex/ui/defs.h>
#include <wex/ui/file-dialog.h>
#include <wex/ui/frame.h>

// #define USE_THREAD 1

#define FILE_POST(ACTION)                                                      \
  auto* event =                                                                \
    new wxCommandEvent(wxEVT_COMMAND_MENU_SELECTED, ID_EDIT_FILE_ACTION);      \
  event->SetInt(ACTION);                                                       \
  wxQueueEvent(m_stc, event);

#ifdef USE_THREAD
#include <thread>
#endif

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

bool wex::stc_file::do_file_load(bool synced)
{
  file_dialog dlg(this);

  if (is_contents_changed() && dlg.show_modal_if_changed() == wxID_CANCEL)
  {
    return false;
  }

  m_stc->use_modification_markers(false);
  m_stc->keep_event_data(synced);

  if (
    m_stc->path().stat().get_size() >
    config("stc.max.Size visual").get(1000000))
  {
    m_stc->visual(false);
  }

  const bool hexmode =
    dlg.is_hexmode() || m_stc->data().flags().test(data::stc::WIN_HEX) ||
    (config(_("stc.Ex mode show hex")).get(false) && !m_stc->is_visual());

  const std::streampos offset =
    m_previous_size < m_stc->path().stat().get_size() &&
        m_stc->data().event().is_synced_log() ?
      m_previous_size :
      std::streampos(0);

  if (offset == std::streampos(0))
  {
    m_stc->clear();
  }

  m_previous_size = m_stc->path().stat().get_size();

#ifdef USE_THREAD
  std::thread t(
    [&]
    {
#endif
      if (!m_stc->is_visual())
      {
        if (hexmode && !m_stc->get_hexmode().is_active())
        {
          m_stc->get_hexmode().set(true, false);
        }

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

  if (!m_stc->is_visual())
  {
    if (!open(std::ios_base::out | std::ios_base::in | std::ios_base::trunc))
    {
      log("file not opened") << path();
    }
    else
    {
      ex_stream()->stream(*this);
    }

    m_stc->SetReadOnly(true);
  }

  if (m_stc->data().control().command().empty())
  {
    m_stc->clear();
  }

  m_stc->get_lexer().set(path_lexer(path()).lexer(),
                         true); // allow fold
}

bool wex::stc_file::do_file_save(bool save_as)
{
  m_stc->SetReadOnly(true); // prevent changes during saving

  bool ok = true;

  if (!m_stc->is_visual())
  {
    if (ok = ex_stream()->write(); ok)
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
        if (ok = write(m_stc->get_hexmode().buffer()); ok)
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
        if (ok = write(m_stc->get_text()); ok)
        {
          FILE_POST(save_as ? FILE_SAVE_AS : FILE_SAVE);
        }
#ifdef USE_THREAD
      });
    t.detach();
#endif
  }

  if (
    auto* client = m_stc->get_frame()->lsp_clients_find(m_stc->path());
    client != nullptr)
  {
    client->did_save(m_stc->path());
  }

  return ok;
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
