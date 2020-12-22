////////////////////////////////////////////////////////////////////////////////
// Name:      stc/ex-stream.cpp
// Purpose:   Implementation of class wex::ex_stream
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <regex>
#include <stdio.h>
#include <string.h>
#include <wex/addressrange.h>
#include <wex/core.h>
#include <wex/ex-stream.h>
#include <wex/frd.h>
#include <wex/log.h>
#include <wex/managed-frame.h>
#include <wex/stc.h>

wex::ex_stream::ex_stream(wex::stc* stc)
  : m_context_size(500)
  , m_line_size(500)
  , m_current_line(new char[m_line_size])
  , m_stc(stc)
{
}

wex::ex_stream::~ex_stream()
{
  delete[] m_current_line;
}

bool wex::ex_stream::erase(const addressrange& range)
{
  if (m_stream == nullptr)
  {
    return false;
  }

  log::trace("ex stream erase")
    << range.get_begin().get_line() << range.get_end().get_line();

  m_stream->seekg(0);

  char tmp_filename[L_tmpnam];
  tmpnam(tmp_filename);
  int nr_deletes = 0, current = 0;

  {
    std::fstream tfs(tmp_filename, std::ios_base::out);
    int  i = 0;
    char c;

    while (m_stream->get(c))
    {
      m_current_line[i++] = c;
      
      if (c == '\n')
      {
        if (current >= range.get_begin().get_line() - 1 &&
            current <= range.get_end().get_line() - 1)
        {
          nr_deletes++;
        }
        else
        {
          tfs.write(m_current_line, i);
        }
        
        i = 0;
        current++;
      }
    }
  }
  
  m_last_line_no = current - nr_deletes;

  m_file->close();
  m_file->open(std::ios_base::out);
  std::fstream tfs(tmp_filename);
  *m_stream << tfs.rdbuf();
  m_file->close();
  std::remove(tmp_filename);

  log::trace("ex stream deletes") << nr_deletes;
  
  m_stc->get_frame()->show_ex_message(
    std::to_string(nr_deletes) +     " fewer lines");
  
  m_file->open();
  goto_line(0);

  return true;
}

  
bool wex::ex_stream::find(const std::string& text)
{
  if (m_stream == nullptr)
  {
    return false;
  }

  log::trace("ex stream find") << text;

  auto line_no = m_line_no;
  auto pos     = m_stream->tellg();
  bool found   = false;

  const std::regex r(text);
  const bool       use_regex(find_replace_data::get()->is_regex());

  while (!found && get_next_line())
  {
    if (
      (use_regex && std::regex_search(m_current_line, r)) ||
      (!use_regex && strstr(m_current_line, text.c_str()) != nullptr))
    {
      found = true;
    }
  }

  if (!found)
  {
    m_line_no = line_no;
    m_stream->clear();
    m_stream->seekg(pos);

    m_stc->get_frame()->statustext(
      get_find_result(text, true, true),
      std::string());
  }
  else
  {
    log::trace("ex stream found") << text << m_line_no;
    m_context = m_current_line;
    set_text();
  }

  return found;
}

int wex::ex_stream::get_current_line() const
{
  return m_line_no;
}

int wex::ex_stream::get_line_count() const
{
  return m_last_line_no;
}

int wex::ex_stream::get_line_count_request()
{
  if (m_stream == nullptr)
  {
    return LINE_COUNT_UNKNOWN;
  }

  auto pos = m_stream->tellg();
  m_stream->clear();
  m_stream->seekg(0);
  m_last_line_no = 0;

  char c;
  while (m_stream->get(c))
  {
    if (c == '\n')
    {
      m_last_line_no++;
    }
  }

  m_stream->clear();
  m_stream->seekg(pos);

  return m_last_line_no;
}

bool wex::ex_stream::get_next_line()
{
  if (!m_stream->getline(m_current_line, m_line_size))
  {
    log::debug("no next line") << m_line_no;
    return false;
  }

  m_line_no++;

  return true;
}

void wex::ex_stream::goto_line(int no)
{
  if (m_stream == nullptr || no == m_line_no)
  {
    return;
  }

  log::trace("ex stream goto_line") << no << m_line_no;

  if (no < m_line_no)
  {
    // currently reset.
    m_line_no = -1;
    m_stream->clear();
    m_stream->seekg(0);
    log::trace("ex stream reset");
    m_context.clear();

    while ((m_line_no < no) && get_next_line())
    {
    }
  }
  else
  {
    while ((no > m_line_no) && get_next_line())
    {
    }
  }

  set_context();
  set_text();
}

bool wex::ex_stream::insert_text(int line, const std::string& text, loc_t loc)
{
  if (m_stream == nullptr)
  {
    return false;
  }

  m_stream->seekg(0);

  char tmp_filename[L_tmpnam];
  tmpnam(tmp_filename);

  {
    std::fstream tfs(tmp_filename, std::ios_base::out);

    char c;
    int  current = 0;
    bool done    = false;

    if (line == 0 && loc == INSERT_BEFORE)
    {
      if (!tfs.write(text.c_str(), text.size()))
      {
        return false;
      }

      done = true;
    }

    while (m_stream->get(c))
    {
      if (c != '\n')
      {
        if (!tfs.put(c))
        {
          return false;
        }
      }
      else
      {
        if (current++ == line && !done)
        {
          switch (loc)
          {
            case INSERT_AFTER:
              tfs.put(c);
              tfs.write(text.c_str(), text.size());
              break;

            case INSERT_BEFORE:
              tfs.write(text.c_str(), text.size());
              tfs.put(c);
              break;
          }
          done = true;
        }
        else
        {
          tfs.put(c);
        }
      }
    }
  }

  m_file->close();
  m_file->open(std::ios_base::out);
  std::fstream tfs(tmp_filename);
  *m_stream << tfs.rdbuf();
  m_file->close();
  std::remove(tmp_filename);
  
  m_file->open();
  goto_line(line);

  return true;
}

void wex::ex_stream::set_context()
{
  m_context += (!m_context.empty() ? "\n" : std::string()) + m_current_line;

  if (m_context.size() > m_context_size)
  {
    m_context = m_context.substr(strlen(m_current_line));
  }
}

void wex::ex_stream::set_text()
{
  m_stc->SetReadOnly(false);
  m_stc->SetText(m_context);
  m_stc->DocumentEnd();
  m_stc->EmptyUndoBuffer();
  m_stc->SetSavePoint();
  m_stc->SetReadOnly(true);
  m_stc->use_modification_markers(false);
}

void wex::ex_stream::stream(file& f)
{
  m_file = &f;
  m_stream = &f.stream();
  f.use_stream();

  goto_line(0);
}

bool wex::ex_stream::substitute(
  const addressrange& range,
  const std::string&  find,
  const std::string&  replace)
{
  if (m_stream == nullptr)
  {
    return false;
  }

  log::trace("ex stream substitute")
    << range.get_begin().get_line() << range.get_end().get_line() << find
    << replace;

  m_stream->seekg(0);

  char tmp_filename[L_tmpnam];
  tmpnam(tmp_filename);
  int nr_replacements = 0;

  {
    std::fstream tfs(tmp_filename, std::ios_base::out);
    int  current = 0, i = 0;
    const std::regex r(find);
    const bool       use_regex(find_replace_data::get()->is_regex());
    char* pch;
    char c;
    std::smatch m;

    while (m_stream->get(c))
    {
      m_current_line[i++] = c;
      
      if (c == '\n')
      {
        if (current >= range.get_begin().get_line() -1 &&
            current <= range.get_end().get_line() -1)
        {
          if (use_regex)
          { 
            std::string text(m_current_line, i); 
            
            if (std::regex_search(text, m, r))
            {
              text = std::regex_replace(text, r, replace);
              nr_replacements++;
            }
            
            tfs.write(text.c_str(), text.size());
          }
          else if ((pch = strstr(m_current_line, find.c_str())) != nullptr)
          {
            strncpy(pch, replace.c_str(), replace.size());

            if (!tfs.write(m_current_line, strlen(m_current_line)))
            {
              log("ex stream substitute") << "line" << m_current_line;
              return false;
            }

            nr_replacements++;
          }
          else
          {
            tfs.write(m_current_line, i);
          }
        }
        else
        {
          tfs.write(m_current_line, i);
        }
        
        i = 0;
        current++;
      }
    }
  }
    
  m_file->close();
  m_file->open(std::ios_base::out);
  std::fstream tfs(tmp_filename);
  *m_stream << tfs.rdbuf();
  m_file->close();
  std::remove(tmp_filename);
  
  log::trace("ex stream substitute") << nr_replacements;
  
  m_stc->get_frame()->show_ex_message(
    "Replaced: " + std::to_string(nr_replacements) +
    " occurrences of: " + find);

  m_file->open();
  goto_line(0);

  return true;
}
