////////////////////////////////////////////////////////////////////////////////
// Name:      file_imp.cpp
// Purpose:   Implementation of class wex::file_imp
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <fstream>
#include <wex/log.h>

#include "file-imp.h"

wex::file_imp::file_imp(const wex::path& filename)
  : m_path(filename)
  , m_stat(m_path.string())
{
  ;
}

wex::file_imp::file_imp(const wex::path& filename, std::ios_base::openmode mode)
  : m_path(filename)
  , m_stat(m_path.string())
  , m_fs(m_path.data(), mode)
{
  ;
}

void wex::file_imp::assign(const wex::path& p)
{
  close();
  m_path = p;
  m_stat.sync(m_path.string());
  m_fs = std::fstream(m_path.data());
}

bool wex::file_imp::close()
{
  if (!m_fs.is_open())
    return true;
  m_fs.close();
  return !m_fs.is_open();
}

void wex::file_imp::log_stream_info(const std::string& info, size_t s)
{
  log(info) << "eofbit" << m_fs.eof() << "fail" << m_fs.fail() << "bad"
            << m_fs.bad() << "tellg" << (size_t)m_fs.tellg() << "tellp"
            << (size_t)m_fs.tellp() << "count" << (size_t)m_fs.gcount()
            << "size" << s;
}

bool wex::file_imp::open(const wex::path& p, std::ios_base::openmode mode)
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

const std::string* wex::file_imp::read(std::streampos seek_position)
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
  m_buffer->resize(path().stat().st_size - seek_position);
  m_fs.read(m_buffer->data(), m_buffer->size());

  if (m_fs.bad())
  {
    log_stream_info("read", m_buffer->size());
    return nullptr;
  }

  return m_buffer.get();
}

bool wex::file_imp::write(const char* s, size_t n)
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
