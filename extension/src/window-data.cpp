////////////////////////////////////////////////////////////////////////////////
// Name:      stc-wnidow.cpp
// Purpose:   Implementation of wxExWindowData
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/app.h>
#include <wx/extension/window-data.h>
#include <wx/extension/managedframe.h>

wxExWindowData::wxExWindowData()
{
  if (m_Parent == nullptr && wxTheApp != nullptr)
  {
    m_Parent = dynamic_cast<wxExManagedFrame*>(wxTheApp->GetTopWindow());
  }
}

wxExWindowData& wxExWindowData::Button(long button)
{
  m_Button = button;
  return *this;
}

wxExWindowData& wxExWindowData::Id(wxWindowID id) 
{
  m_Id = id;
  return *this;
}
  
wxExWindowData& wxExWindowData::Name(const std::string& name) 
{
  m_Name = name;
  return *this;
}
  
wxExWindowData& wxExWindowData::Parent(wxWindow* parent)
{
  m_Parent = parent;
  return *this;
}

wxExWindowData& wxExWindowData::Pos(const wxPoint& point)
{
  m_Pos = point;
  return *this;
}
  
wxExWindowData& wxExWindowData::Size(const wxSize& size)
{
  m_Size = size;
  return *this;
}
  
wxExWindowData& wxExWindowData::Style(long style) 
{
  m_Style = style;
  return *this;
}

wxExWindowData& wxExWindowData::Title(const std::string& title) 
{
  m_Title = title;
  return *this;
}
