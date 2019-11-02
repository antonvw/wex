////////////////////////////////////////////////////////////////////////////////
// Name:      file.cpp
// Purpose:   Implementation of class wex::file
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <fstream>
#include <wex/file.h>
#include <wex/config.h>
#include <wex/stat.h>

namespace wex
{
  class file_imp
  {
  public:
    file_imp() {;};

    file_imp(const path& filename) 
      : m_path(filename)
      , m_stat(m_path.string()) {;};
    
    file_imp(const path& filename, std::ios_base::openmode mode) 
      : m_path(filename)
      , m_stat(m_path.string())
      , m_fs(m_path.data(), mode) {;};

    virtual ~file_imp() {;};

    void assign(const path& p) {
      close();
      m_path = p;
      m_stat.sync(m_path.string());
      m_fs = std::fstream(m_path.data());};

    bool close() {
      if (!m_fs.is_open()) return true;
      m_fs.close();
      return !m_fs.is_open();};

    bool open(
      const path& p, 
      std::ios_base::openmode mode = std::ios_base::in) {
      if (m_fs.is_open())
      { 
        return true;
      }
      if (m_path != p)
      {
        assign(p);
      }
      m_fs.open(m_path.data(), mode);
      return m_fs.is_open();};

    auto & path() {return m_path;};

    const std::string* read(std::streampos seek_position) {
      if (!m_fs.is_open()) 
      {
        if (!open(m_path.data()))
        {
          log("read") << "open" << m_path;
          return nullptr;
        }
      }

      if ((m_buffer.get() != nullptr && seek_position > 0) || 
          (m_fs.tellg() != seek_position))
      {
        m_fs.seekg(seek_position);
      }

      m_buffer = std::make_unique<std::string> ();

#ifndef __WXMSW__      
      m_buffer->resize(path().stat().st_size - seek_position);
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

      return m_buffer.get();};
      
    auto & stat() {return m_stat;};

    auto & stream() const {return m_fs;};

    bool write(const char* s, size_t n) {
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
      return m_fs.good();};
  private:
    void log_stream_info(const std::string& info, size_t s) {
      log(info) 
        << "eofbit" << m_fs.eof()
        << "fail" << m_fs.fail()
        << "bad" << m_fs.bad()
        << "tellg" << (size_t)m_fs.tellg()
        << "tellp" << (size_t)m_fs.tellp()
        << "count" << (size_t)m_fs.gcount() 
        << "size" << s;};
      
    wex::path m_path;
    file_stat m_stat; // used to check for sync
    std::fstream m_fs;
    std::unique_ptr<std::string> m_buffer;
  };
};

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

wex::file::~file()
{
}

wex::file& wex::file::operator=(const file& f)
{
  if (this != &f)
  {
    m_file = std::make_unique<file_imp>(f.m_file->path());
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
  if (config con(config::DATA_NO_STORE);
      is_open() ||
      !m_file->path().stat().is_ok() ||
      !con.item("AllowSync").get(true))
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

bool wex::file::file_load(bool synced)
{
  if (synced && !open())
  {
    return false;
  }

  if (!do_file_load(synced))
  {
    m_file->close();
    return false;
  }

  m_file->close();
  m_is_loaded = true;
  reset_contents_changed();

  return true;
}

bool wex::file::file_load(const path& p)
{
  if (!p.file_exists())
  {
    return false;
  }

  assign(p);
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
    log("file") << "not loaded" << p;
    return false;
  }

  do_file_save(save_as);
  m_file->close();
  reset_contents_changed();
  
  return 
    m_file->path().m_stat.sync() &&
    m_file->stat().sync();
}

const wex::path& wex::file::get_filename() const 
{
  return m_file->path();
}

bool wex::file::is_open() const 
{
  return m_file->stream().is_open();
}

bool wex::file::open(std::ios_base::openmode mode)
{
  return open(m_file->path(), mode);
}

bool wex::file::open(const path& p, std::ios_base::openmode mode)
{
  return m_file->open(p, mode);
}

const std::string* wex::file::read(std::streampos seek_position)
{
  return m_file->read(seek_position);
}

bool wex::file::write(const char* s, size_t n)
{
  return m_file->write(s, n);
}

bool wex::file::write(const std::string& s) 
{
  return m_file->write(s.c_str(), s.size());
} 
