////////////////////////////////////////////////////////////////////////////////
// Name:      file.cpp
// Purpose:   Implementation of class wex::file
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <fstream>
#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wex/config.h>
#include <wex/file.h>
#include <wex/stat.h>

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
      , m_Stat(m_Path.Path().string()) 
      , m_fs(m_Path.Path(), mode) {;};

    void Assign(const path& p) {
      Close();

      m_Path = p;
      m_Stat.sync(m_Path.Path().string());
      m_fs = std::fstream(m_Path.Path());};

    bool Close() {
      if (!m_fs.is_open()) return true;
      m_fs.close();
      return !m_fs.is_open();};

    bool Open(
      const path& p, 
      std::ios_base::openmode mode = std::ios_base::in | std::ios_base::out) {
      if (m_fs.is_open())
      { 
        return true;
      }
      if (m_Path != p)
      {
        Assign(p);
      }
      m_fs.open(m_Path.Path(), mode);
      return m_fs.is_open();};

    auto & Path() {return m_Path;};

    bool Read(char* s, std::streamsize n) {
      m_fs.read(s, n);
      return m_fs.gcount() == n;};

    auto & Stat() {return m_Stat;};

    auto & Stream() {return m_fs;};

    bool Write(const char* c, size_t s) {
      if (!m_fs.is_open()) return false;
      m_fs.write(c, s);
      return m_fs.good();};
  private:
    path m_Path;
    file_stat m_Stat; // used to check for sync

    std::fstream m_fs;
  };
};

wex::file::file(bool open_file)
  : m_OpenFile(open_file)
  , m_File(std::make_unique<file_imp>()) 
{
}

wex::file::file(
  const path& p,
  std::ios_base::openmode mode,
  bool open_file)
  : m_File(std::make_unique<file_imp>(p, mode))
  , m_OpenFile(open_file)
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
    m_File = std::make_unique<file_imp>(f.m_File->Path());
    m_IsLoaded = f.m_IsLoaded;
    m_OpenFile = f.m_OpenFile;
  }

  return *this;
}

void wex::file::Assign(const path& p) 
{
  m_IsLoaded = true;
  m_File->Assign(p);
}

bool wex::file::CheckSync()
{
  // config might be used without wxApp.
  if (config con(config::DATA_NO_STORE);
      IsOpened() ||
      !m_File->Path().GetStat().is_ok() ||
      !con.item("AllowSync").get(true))
  {
    return false;
  }

  if (m_File->Path().m_Stat.sync())
  {
    bool sync_needed = false;
    
    if (m_File->Path().m_Stat.st_mtime != m_File->Stat().st_mtime)
    {
      // Do not check return value,
      // we sync anyhow, to force nex time no sync.
      FileLoad(true);
      
      sync_needed = true;
    }
    
    if (m_File->Path().m_Stat.is_readonly() != m_File->Stat().is_readonly())
    {
      sync_needed = true;
    }

    if (sync_needed)
    {
      // Update the stat member, so next time no sync.
      if (!m_File->Stat().sync())
      {
        wxLogStatus("Could not sync: ", m_File->Path().Path().string().c_str());
      }
        
      return true;
    }
  }
  
  return false;
}

bool wex::file::FileLoad(bool synced)
{
  if ((synced && !Open()) ||
     (!synced && m_OpenFile && !Open()))
  {
    return false;
  }

  if (!DoFileLoad(synced))
  {
    m_File->Close();
    return false;
  }

  m_File->Close();
  
  m_IsLoaded = true;

  ResetContentsChanged();

  return true;
}

bool wex::file::FileLoad(const path& p)
{
  if (!p.FileExists())
  {
    return false;
  }

  Assign(p);
  FileLoad(false);

  return true;
}

bool wex::file::FileNew(const path& p)
{
  Assign(p);
  DoFileNew();

  return true;
}

bool wex::file::FileSave(const path& p)
{
  bool save_as = false;

  if (!p.Path().empty())
  {
    Assign(p);
    save_as = true;
  }

  if (!save_as && !m_IsLoaded)
  {
    wxLogStatus("File has not been loaded");
    return false;
  }

  if (m_OpenFile && !Open(std::ios_base::out))
  {
    return false;
  }

  DoFileSave(save_as);

  m_File->Close();

  ResetContentsChanged();
  
  m_File->Path().m_Stat.sync();
  m_File->Stat().sync();

  return true;
}

const wex::path& wex::file::GetFileName() const 
{
  return m_File->Path();
}

bool wex::file::IsOpened() const 
{
  return m_File->Stream().is_open();
}

bool wex::file::Open(std::ios_base::openmode mode)
{
  return Open(m_File->Path(), mode);
}

bool wex::file::Open(const path& p, std::ios_base::openmode mode)
{
  return m_File->Open(p, mode);
}

const std::string* wex::file::Read(std::streampos seek_position)
{
  wxASSERT(IsOpened());
  
  if ((m_Buffer.get() != nullptr && seek_position > 0) || 
      (m_Buffer.get() != nullptr && m_File->Stream().tellg() != seek_position))
  {
    m_File->Stream().seekg(seek_position);
  }

  m_Buffer = std::make_unique<std::string> ();
  m_Buffer->resize(m_File->Path().GetStat().st_size - seek_position);

  if (!m_File->Read(m_Buffer->data(), m_Buffer->size()))
  {
    return nullptr;
  }

  return m_Buffer.get();
}

bool wex::file::Write(const char* s, size_t n)
{
  return m_File->Write(s, n);
}

bool wex::file::Write(const std::string& s) 
{
  return m_File->Write(s.c_str(), s.size());
} 
