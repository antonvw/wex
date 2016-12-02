////////////////////////////////////////////////////////////////////////////////
// Name:      cmdline.cpp
// Purpose:   Implementation of wxExCmdLineParser class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/app.h>
#include <wx/datetime.h>
#include <wx/extension/cmdline.h>

wxExCmdLineParser::wxExCmdLineParser(
  const std::string& cmdline, 
  const CmdSwitches & s, 
  const CmdOptions & o, 
  const CmdParams & p,
  size_t short_option_size) 
  : m_Parser(cmdline) 
  , m_Switches(s) 
  , m_Options(o)
  , m_Params(p) 
{
  for (const auto& it : s) 
  {
    m_Parser.AddSwitch(
      it.first.first.size() <= short_option_size ? it.first.first: std::string(), 
      it.first.first.size() <= short_option_size ? std::string(): it.first.first, 
      it.first.second, it.second.first);
  }

  for (const auto& it : o) 
  {
    m_Parser.AddOption(
      it.first.first.size() <= short_option_size ? it.first.first: std::string(), 
      it.first.first.size() <= short_option_size ? std::string(): it.first.first, 
      it.first.second, it.second.first);
  }

  for (const auto& it : p) 
  {
    if (!it.first.empty())
    {
      m_Parser.AddParam(it.first, wxCMD_LINE_VAL_STRING, it.second.first);
    }
  }

  if (!s.empty() || !o.empty() || !p.empty())
  {
    m_Parser.AddSwitch("h", std::string(), "help", wxCMD_LINE_OPTION_HELP);
  }
}

wxExCmdLineParser::wxExCmdLineParser(
  const CmdSwitches & s, const CmdOptions & o, const CmdParams & p)
  : wxExCmdLineParser(std::string(), s, o, p)
{
  m_Parser.SetCmdLine(wxTheApp->argc, wxTheApp->argv);
}
    
int wxExCmdLineParser::Parse(bool giveUsage) 
{
  const int res = m_Parser.Parse(giveUsage);

  if (res == 0) // ok
  {
    for (const auto& it : m_Switches) 
    {
      if (m_Parser.Found(it.first.first))
        it.second.second(
          m_Parser.FoundSwitch(it.first.first) == wxCMD_SWITCH_ON);
    }

    for (const auto& it : m_Options) 
    {
      if (m_Parser.Found(it.first.first))
      {
        wxAny any;
        switch (it.second.first)
        {
          case wxCMD_LINE_VAL_DATE:
          {
            wxDateTime val;
            m_Parser.Found(it.first.first, &val);
            any = val;
          }
          break;
          case wxCMD_LINE_VAL_DOUBLE:
          {
            double val;
            m_Parser.Found(it.first.first, &val);
            any = val;
          }
          break;
          case wxCMD_LINE_VAL_NUMBER:
          {
            long val;
            m_Parser.Found(it.first.first, &val);
            any = val;
          }
          break;
          case wxCMD_LINE_VAL_STRING:
          {
            wxString val;
            m_Parser.Found(it.first.first, &val);
            any = val;
          }
          break;
          default: ; // do nothing
        }
        it.second.second(any);
      }
    }

    for (const auto& it : m_Params) 
    {
      if (!it.first.empty())
      {
        std::vector<std::string> v;
        for (size_t i = 0; i < m_Parser.GetParamCount(); i++)
        {
          v.emplace_back(m_Parser.GetParam(i).ToStdString());
        }
        it.second.second(v);
      }
    }
  }

  return res;
}
