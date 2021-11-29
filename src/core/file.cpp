////////////////////////////////////////////////////////////////////////////////
// Name:      file.cpp
// Purpose:   Implementation of class wex::file
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wex/core/config.h>
#include <wex/core/file.h>
#include <wex/core/log.h>

namespace fs = std::filesystem;

namespace wex
{
bool copy(const wex::path& from, const wex::path& to)
{
  return fs::copy_file(from.data(), to.data());
}
} // namespace wex

wex::file::file()
  : m_stat(std::string())
{
}

wex::file::file(const wex::path& p)
  : m_path(p)
  , m_stat(m_path.string())
{
}

wex::file::file(const wex::path& p, std::ios_base::openmode mode)
  : m_path(p)
  , m_stat(m_path.string())
  , m_fs(m_path.data(), mode)
{
}

wex::file::file(const char* filename)
  : wex::file(wex::path(filename))
{
}

wex::file::file(const char* filename, std::ios_base::openmode mode)
  : wex::file(wex::path(filename), mode)
{
}

wex::file::file(const file& rhs)
  : m_stat(rhs.m_stat)
{
  *this = rhs;
}

wex::file::~file() {}

wex::file& wex::file::operator=(const file& f)
{
  if (this != &f)
  {
    m_is_loaded  = f.m_is_loaded;
    m_is_written = f.m_is_written;
    m_path       = f.m_path;
    m_stat       = f.m_stat;
    m_use_stream = f.m_use_stream;
  }

  return *this;
}

void wex::file::assign(const wex::path& p)
{
  close();
  m_path = p;
  m_stat.sync(m_path.string());
  m_fs = std::fstream(m_path.data());
}

bool wex::file::check_sync()
{
  if (is_open() || !m_path.m_stat.is_ok() || !config("AllowSync").get(true))
  {
    return false;
  }

  if (m_path.m_stat.sync())
  {
    bool sync_needed = false;

    if (m_path.m_stat.get_modification_time() != m_stat.get_modification_time())
    {
      // Do not check return value,
      // we sync anyhow, to force nex time no sync.
      file_load(true);

      sync_needed = true;
    }

    if (m_path.m_stat.is_readonly() != m_stat.is_readonly())
    {
      sync_needed = true;
    }

    if (sync_needed)
    {
      // Update the stat member, so next time no sync.
      if (!m_stat.sync())
      {
        log::status("Could not sync") << m_path;
      }

      return true;
    }
  }

  return false;
}

bool wex::file::close()
{
  if (!m_fs.is_open())
    return true;

  m_fs.close();

  return !m_fs.is_open();
}

void wex::file::do_file_save(bool save_as)
{
  if (save_as)
  {
    copy(m_path_prev, m_path);
  }
}

bool wex::file::file_load(bool synced)
{
  if (synced && !open(std::ios_base::in | std::ios_base::out))
  {
    return false;
  }

  if (!do_file_load(synced))
  {
    close();
    return false;
  }

  if (!m_use_stream)
  {
    close();
  }

  m_is_loaded = true;
  reset_contents_changed();

  return true;
}

bool wex::file::file_load(const wex::path& p)
{
  assign(p);

  if (!p.file_exists())
  {
    return false;
  }

  file_load(false);

  return true;
}

bool wex::file::file_new(const wex::path& p)
{
  assign(p);
  do_file_new();
  m_is_loaded = true;

  return true;
}

bool wex::file::file_save(const wex::path& p)
{
  bool save_as = false;

  if (!p.empty())
  {
    m_path_prev = m_path;
    assign(p);
    save_as = true;
  }

  if (!save_as && !m_is_loaded)
  {
    if (!p.empty())
    {
      log("file not loaded") << p;
    }

    return false;
  }

  do_file_save(save_as);

  if (!m_use_stream)
  {
    close();
  }

  reset_contents_changed();

  return m_path.m_stat.sync() && m_stat.sync();
}

void wex::file::log_stream_info(const std::string& info, size_t s)
{
  log(info) << "eofbit" << m_fs.eof() << "fail" << m_fs.fail() << "bad"
            << m_fs.bad() << "tellg" << (size_t)m_fs.tellg() << "tellp"
            << (size_t)m_fs.tellp() << "count" << (size_t)m_fs.gcount()
            << "size" << s;
}

bool wex::file::open(const wex::path& p, std::ios_base::openmode mode)
{
  if (m_fs.is_open())
  {
    return true;
  }

  if (m_path != p)
  {
    assign(p);
  }

  m_fs.open(m_path.data(), mode);

  return m_fs.is_open();
}

const std::string* wex::file::read(std::streampos seek_position)
{
  if (!m_fs.is_open())
  {
    if (!open(m_path.data()))
    {
      log("read") << "open" << m_path;
      return nullptr;
    }
  }

  if (
    (m_buffer.get() != nullptr && seek_position > 0) ||
    (m_fs.tellg() != seek_position))
  {
    m_fs.seekg(seek_position);
  }

  m_buffer = std::make_unique<std::string>();

#ifndef __WXMSW__
  m_buffer->resize(m_path.m_stat.get_size() - seek_position);
  m_fs.read(m_buffer->data(), m_buffer->size());
#else
  char c;
  while (m_fs.get(c))
  {
    m_buffer->push_back(c);
  }
#endif

  if (m_fs.bad())
  {
    log_stream_info("read", m_buffer->size());
    return nullptr;
  }

  return m_buffer.get();
}

bool wex::file::write(const char* s, size_t n)
{
  if (!m_fs.is_open())
  {
    if (!open(m_path.data(), std::ios_base::out))
    {
      log("write") << "open" << m_path;
      return false;
    }
  }

  m_fs.write(s, n);

  if (!m_fs.good())
  {
    log_stream_info("write", n);
  }
  else
  {
    m_is_written = true;
  }

  return m_fs.good();
}
