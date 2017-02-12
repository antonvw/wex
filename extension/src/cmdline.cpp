////////////////////////////////////////////////////////////////////////////////
// Name:      cmdline.cpp
// Purpose:   Implementation of wxExCmdLine class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <wx/app.h>
#include <wx/config.h>
#include <wx/extension/cmdline.h>
#include <wx/extension/tokenizer.h>
#include <wx/extension/version.h>

wxExCmdLine::wxExCmdLine(
  const CmdSwitches & s, 
  const CmdOptions & o, 
  const CmdParams & p,
  const std::string& message,
  const char delimiter,
  const std::string &version,
  bool helpAndVersion)
  : m_CmdLine(
      message, delimiter, 
      version.empty() ? wxExGetVersionInfo().GetVersionOnlyString().ToStdString(): version, 
      helpAndVersion) 
{
  m_CmdLine.setExceptionHandling(false);

  try
  {
    for (auto it = o.rbegin(); it != o.rend(); ++it)
    {
      switch (it->second.first)
      {
        case CMD_LINE_FLOAT: {
          auto* arg = new TCLAP::ValueArg<float>(
            std::get<0>(it->first), std::get<1>(it->first), std::get<2>(it->first),
            false, -1, "float");
          m_CmdLine.add(arg);
          m_Options.push_back(new wxExCmdLineContent(it->second.second, arg));
          }
          break;

        case CMD_LINE_INT: {
          auto* arg = new TCLAP::ValueArg<int>(
            std::get<0>(it->first), std::get<1>(it->first), std::get<2>(it->first),
            false, -1, "int");
          m_CmdLine.add(arg);
          m_Options.push_back(new wxExCmdLineContent(it->second.second, arg));
          }
          break;
        
        case CMD_LINE_STRING: {
          auto* arg = new TCLAP::ValueArg<std::string>(
            std::get<0>(it->first), std::get<1>(it->first), std::get<2>(it->first),
            false, std::string(), "string");
          m_CmdLine.add(arg);
          m_Options.push_back(new wxExCmdLineContent(it->second.second, arg));
          }
          break;
        
        default: std::cerr << "unknown type\n";
      }
    }

    for (auto it = s.rbegin(); it != s.rend(); ++it)
    {
      const bool def = !wxConfigBase::Get()->ReadBool(std::get<1>(it->first), true);
      auto* arg = new TCLAP::SwitchArg(
        std::get<0>(it->first), std::get<1>(it->first), std::get<2>(it->first), def); 
      m_Switches.push_back({arg, it->second});
      m_CmdLine.add(arg);
    }

    if (!p.first.first.empty())
    {
      auto* arg = new TCLAP::UnlabeledMultiArg<std::string>(
        p.first.first, p.first.second, false, std::string());
      m_Params.push_back({arg, p.second});
      m_CmdLine.add(arg);
    }
  }
  catch (TCLAP::ArgException& e)
  {
    std::cerr << e.what() << "\n";
  }
  catch (TCLAP::ExitException& )
  {
  }
}

wxExCmdLine::~wxExCmdLine()
{
  for (auto& it : m_Options) 
  {
    switch (it->m_Type)
    {
      case CMD_LINE_FLOAT: delete it->m_tclap_u.m_val_f; break;
      case CMD_LINE_INT: delete it->m_tclap_u.m_val_i; break;
      case CMD_LINE_STRING: delete it->m_tclap_u.m_val_s; break;
    }
    delete it;
  }

  for (auto& it : m_Params) delete it.first;
  for (auto& it : m_Switches) delete it.first;
}
  
bool wxExCmdLine::Parse(const std::string& cmdline, bool toggle)
{
  try
  {
    if (cmdline.empty())
    {
      m_CmdLine.parse(wxTheApp->argc, wxTheApp->argv);
    }
    else
    {
      wxExTokenizer tkz(cmdline);
      auto v = tkz.Tokenize<std::vector<std::string>>();
      m_CmdLine.parse(v);
    }

    for (const auto& it : m_Switches) 
    {
      it.second(it.first->getValue());

      if (toggle)
      {
        wxConfigBase::Get()->Write(it.first->getName(), !it.first->getValue());
      }
    }

    for (const auto& it : m_Options)
    {
      switch (it->m_Type)
      {
        case CMD_LINE_FLOAT:
          if (it->m_tclap_u.m_val_f->getValue() != -1) 
          {
            it->m_f(it->m_tclap_u.m_val_f->getValue());
          }
          break;

        case CMD_LINE_INT:
          if (it->m_tclap_u.m_val_i->getValue() != -1) 
          {
            it->m_f(it->m_tclap_u.m_val_i->getValue());
          }
          break;

        case CMD_LINE_STRING:
          if (!it->m_tclap_u.m_val_s->getValue().empty()) 
          {
            it->m_f(it->m_tclap_u.m_val_s->getValue());
          }
          break;
      }
    }

    if (!m_Params.empty() && !m_Params[0].first->getValue().empty()) 
    {
      if (!m_Params[0].second(m_Params[0].first->getValue()))
      {
         return false;
      }
    }
    
    return true;
  }
  catch (TCLAP::ArgException& e)
  {
    std::cerr << e.what() << "\n";
    return false;
  }
  catch (TCLAP::ExitException& )
  {
    return false;
  }
}
