////////////////////////////////////////////////////////////////////////////////
// Name:      file.cpp
// Purpose:   Implementation of class wex::file
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <fstream>
#include <wex/config.h>
#include <wex/file.h>
#include <wex/log.h>

#include "file-imp.h"

wex::file::file()
  : m_file(std::make_unique<file_imp>())
{
}

wex::file::file(const path& p)
  : m_file(std::make_unique<file_imp>(p))
{
}

wex::file::file(const path& p, std::ios_base::openmode mode)
  : m_file(std::make_unique<file_imp>(p, mode))
{
}

wex::file::file(const char* filename)
  : wex::file(path(filename))
{
}

wex::file::file(const char* filename, std::ios_base::openmode mode)
  : wex::file(path(filename), mode)
{
}

wex::file::file(const file& rhs)
{
  *this = rhs;
}

wex::file::~file() {}

wex::file& wex::file::operator=(const file& f)
{
  if (this != &f)
  {
    m_file      = std::make_unique<file_imp>(f.m_file->path());
    m_is_loaded = f.m_is_loaded;
  }

  return *this;
}

void wex::file::assign(const path& p)
{
  m_file->assign(p);
}

bool wex::file::check_sync()
{
  if (
    is_open() || !m_file->path().stat().is_ok() ||
    !config("AllowSync").get(true))
  {
    return false;
  }

  if (m_file->path().m_stat.sync())
  {
    bool sync_needed = false;

    if (m_file->path().m_stat.st_mtime != m_file->stat().st_mtime)
    {
      // Do not check return value,
      // we sync anyhow, to force nex time no sync.
      file_load(true);

      sync_needed = true;
    }

    if (m_file->path().m_stat.is_readonly() != m_file->stat().is_readonly())
    {
      sync_needed = true;
    }

    if (sync_needed)
    {
      // Update the stat member, so next time no sync.
      if (!m_file->stat().sync())
      {
        log::status("Could not sync") << m_file->path();
      }

      return true;
    }
  }

  return false;
}

bool wex::file::close()
{
  return m_file->close();
}

bool wex::file::file_load(bool synced)
{
  if (synced && !open(std::ios_base::in | std::ios_base::out))
  {
    return false;
  }

  if (!do_file_load(synced))
  {
    m_file->close();
    return false;
  }

  if (!m_use_stream)
  {
    m_file->close();
  }

  m_is_loaded = true;
  reset_contents_changed();

  return true;
}

bool wex::file::file_load(const path& p)
{
  assign(p);

  if (!p.file_exists())
  {
    return false;
  }

  file_load(false);

  return true;
}

bool wex::file::file_new(const path& p)
{
  assign(p);
  do_file_new();
  m_is_loaded = true;

  return true;
}

bool wex::file::file_save(const path& p)
{
  bool save_as = false;

  if (!p.empty())
  {
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
    m_file->close();
  }

  reset_contents_changed();

  return m_file->path().m_stat.sync() && m_file->stat().sync();
}

const wex::path& wex::file::get_filename() const
{
  return m_file->path();
}

bool wex::file::is_open() const
{
  return m_file->stream().is_open();
}

bool wex::file::is_written() const
{
  return m_file->is_written();
}

bool wex::file::open(std::ios_base::openmode mode)
{
  return open(m_file->path(), mode);
}

bool wex::file::open(const path& p, std::ios_base::openmode mode)
{
  return m_file->open(p, mode);
}

void wex::file::put(char c)
{
  m_file->put(c);
}

const std::string* wex::file::read(std::streampos seek_position)
{
  return m_file->read(seek_position);
}

std::fstream& wex::file::stream()
{
  return m_file->stream();
}

void wex::file::use_stream()
{
  m_use_stream = true;
}

bool wex::file::write(const char* s, size_t n)
{
  return m_file->write(s, n);
}

bool wex::file::write(const std::string& s)
{
  return m_file->write(s.c_str(), s.size());
}
