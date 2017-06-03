////////////////////////////////////////////////////////////////////////////////
// Name:      control-data.cpp
// Purpose:   Implementation of wxExControlData
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/extension/control-data.h>

wxExControlData& wxExControlData::Col(int col)
{
  m_Col = col;
  return *this;
}
  
wxExControlData& wxExControlData::Command(const std::string& command)
{
  m_Command = command;
  return *this;
}
  
wxExControlData& wxExControlData::Find(const std::string& text) 
{
  m_Find = text;
  return *this;
}

bool wxExControlData::Inject(
  std::function<bool(void)> line,
  std::function<bool(void)> col,
  std::function<bool(void)> find,
  std::function<bool(void)> command) const
{
  bool injected = false;
  
  if (m_Line != DATA_NUMBER_NOT_SET && line != nullptr)
  {
    if (line())
    {
      injected = true;
    }
  }
  
  if (m_Col != DATA_NUMBER_NOT_SET && col != nullptr)
  {
    if (col())
    {
      injected = true;
    }
  }
  
  if (!m_Find.empty() && find != nullptr)
  {
    if (find())
    {
      injected = true;
    }
  }

  if (!m_Command.empty() && command != nullptr)
  {
    if (command())
    {
      injected = true;
    }
  }

  return injected;
}

wxExControlData& wxExControlData::Line(int line, std::function<int(int)> valid)
{
  m_Line = (valid != nullptr ? valid(line): line);

  return *this;
}

void wxExControlData::Reset()
{
  m_Col = DATA_NUMBER_NOT_SET;
  m_Command.clear();
  m_Find.clear();
  m_Line = DATA_NUMBER_NOT_SET;
  m_Required = false;
  m_Validator = nullptr;
}

wxExControlData& wxExControlData::Validator(wxValidator* validator)
{
  m_Validator = validator;

  return *this;
}
