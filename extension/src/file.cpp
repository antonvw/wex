////////////////////////////////////////////////////////////////////////////////
// Name:      file.cpp
// Purpose:   Implementation of class wex::file
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <fstream>
#include <wex/config.h>
#include <wex/file.h>
#include <wex/stat.h>
#include <wex/util.h>

namespace wex
{
  class file_imp
  {
  public:
    file_imp() {;};
    file_imp(
      const path& filename,
      std::ios_base::openmode mode = std::ios_base::in) 
      : m_Path(filename)
      , m_Stat(m_Path.data().string()) 
      , m_fs(m_Path.data(), mode) {;};
    
    virtual ~file_imp() {;};

    void assign(const path& p) {
      close();

      m_Path = p;
      m_Stat.sync(m_Path.data().string());
      m_fs = std::fstream(m_Path.data());};

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
      if (m_Path != p)
      {
        assign(p);
      }
      m_fs.open(m_Path.data(), mode);
      return m_fs.is_open();};

    auto & path() {return m_Path;};

    const std::string* read(std::streampos seek_position) {
      if ((m_Buffer.get() != nullptr && seek_position > 0) || 
          (m_Buffer.get() != nullptr && m_fs.tellg() != seek_position))
      {
        m_fs.seekg(seek_position);
      }

      m_Buffer = std::make_unique<std::string> ();
      m_Buffer->resize(path().stat().st_size - seek_position);
      m_fs.read(m_Buffer->data(), m_Buffer->size());

      return m_Buffer.get();};
      
    auto & stat() {return m_Stat;};

    auto & stream() const {return m_fs;};

    bool write(const char* c, size_t s) {
      if (!m_fs.is_open()) return false;
      m_fs.write(c, s);
      return m_fs.good();};
  private:
    wex::path m_Path;
    file_stat m_Stat; // used to check for sync
    std::fstream m_fs;
    std::unique_ptr<std::string> m_Buffer;
  };
};

wex::file::file(bool open_file)
  : m_open_file(open_file)
  , m_File(std::make_unique<file_imp>()) 
{
}

wex::file::file(
  const path& p,
  std::ios_base::openmode mode,
  bool open_file)
  : m_File(std::make_unique<file_imp>(p, mode))
  , m_open_file(open_file)
{
}
  
wex::file::file(
  const std::string& filename,
  std::ios_base::openmode mode,
  bool open_file)
  : wex::file(path(filename), mode, open_file) 
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
    m_File = std::make_unique<file_imp>(f.m_File->path());
    m_IsLoaded = f.m_IsLoaded;
    m_open_file = f.m_open_file;
  }

  return *this;
}

void wex::file::assign(const path& p) 
{
  m_IsLoaded = true;
  m_File->assign(p);
}

bool wex::file::check_sync()
{
  if (config con(config::DATA_NO_STORE);
      is_opened() ||
      !m_File->path().stat().is_ok() ||
      !con.item("AllowSync").get(true))
  {
    return false;
  }

  if (m_File->path().m_Stat.sync())
  {
    bool sync_needed = false;
    
    if (m_File->path().m_Stat.st_mtime != m_File->stat().st_mtime)
    {
      // Do not check return value,
      // we sync anyhow, to force nex time no sync.
      file_load(true);
      
      sync_needed = true;
    }
    
    if (m_File->path().m_Stat.is_readonly() != m_File->stat().is_readonly())
    {
      sync_needed = true;
    }

    if (sync_needed)
    {
      // Update the stat member, so next time no sync.
      if (!m_File->stat().sync())
      {
        log::status("Could not sync") << m_File->path();
      }
        
      return true;
    }
  }
  
  return false;
}

bool wex::file::file_load(bool synced)
{
  if ((synced && !open()) ||
      (!synced && m_open_file && !open()))
  {
    return false;
  }

  if (!do_file_load(synced))
  {
    m_File->close();
    return false;
  }

  m_File->close();
  
  m_IsLoaded = true;

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

  return true;
}

bool wex::file::file_save(const path& p)
{
  bool save_as = false;

  if (!p.data().empty())
  {
    assign(p);
    save_as = true;
  }

  if (!save_as && !m_IsLoaded)
  {
    log::status("File has not been loaded");
    return false;
  }

  if (m_open_file && !open(std::ios_base::out))
  {
    return false;
  }

  do_file_save(save_as);

  m_File->close();

  reset_contents_changed();
  
  m_File->path().m_Stat.sync();
  m_File->stat().sync();

  return true;
}

const wex::path& wex::file::get_filename() const 
{
  return m_File->path();
}

bool wex::file::is_opened() const 
{
  return m_File->stream().is_open();
}

bool wex::file::open(std::ios_base::openmode mode)
{
  return open(m_File->path(), mode);
}

bool wex::file::open(const path& p, std::ios_base::openmode mode)
{
  return m_File->open(p, mode);
}

const std::string* wex::file::read(std::streampos seek_position)
{
  assert(is_opened());
  
  return m_File->read(seek_position);
}

bool wex::file::write(const char* s, size_t n)
{
  return m_File->write(s, n);
}

bool wex::file::write(const std::string& s) 
{
  return m_File->write(s.c_str(), s.size());
} 
