////////////////////////////////////////////////////////////////////////////////
// Name:      cmdline.cpp
// Purpose:   Implementation of wxExCmdLine class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <sstream> // for tclap!
#include <variant>
#include <tclap/CmdLine.h>
#include <wx/app.h>
#include <wx/config.h>
#include <wx/extension/cmdline.h>
#include <wx/extension/log.h>
#include <wx/extension/tokenizer.h>
#include <wx/extension/version.h>

class wxExCmdLineOption
{
public:
  wxExCmdLineOption(
    TCLAP::ValueArg<float>* f, std::function<void(const std::any& any)> fu)
    : m_val(f), m_f(fu) {;};

  wxExCmdLineOption(
    TCLAP::ValueArg<int>* i, std::function<void(const std::any& any)> fu)
    : m_val(i), m_f(fu) {;};

  wxExCmdLineOption(
    TCLAP::ValueArg<std::string>* s, std::function<void(const std::any& any)> fu)
    : m_val(s), m_f(fu) {;};

 ~wxExCmdLineOption()
  {
    if (std::holds_alternative<TCLAP::ValueArg<float>*>(m_val))
      delete std::get<0>(m_val);
    else if (std::holds_alternative<TCLAP::ValueArg<int>*>(m_val))
      delete std::get<1>(m_val);
    else if (std::holds_alternative<TCLAP::ValueArg<std::string>*>(m_val))
      delete std::get<2>(m_val);
  };

  void Run() const
  {
    if (auto pval = std::get_if<TCLAP::ValueArg<float>*>(&m_val))
    {
      if ((*pval)->getValue() != -1) 
      {
        m_f((*pval)->getValue());
      }
    }
    else if (auto pval = std::get_if<TCLAP::ValueArg<int>*>(&m_val))
    {
      if ((*pval)->getValue() != -1) 
      {
        m_f((*pval)->getValue());
      }
    }
    else if (auto pval = std::get_if<TCLAP::ValueArg<std::string>*>(&m_val))
    {
      if (!(*pval)->getValue().empty()) 
      {
        m_f((*pval)->getValue());
      }
    }
  }
private:    
  std::function<void(const std::any& any)> m_f; 

  const std::variant <
    TCLAP::ValueArg<float>*,
    TCLAP::ValueArg<int>*,
    TCLAP::ValueArg<std::string>*> m_val; 
};

class wxExCmdLineParam
{
public:
  wxExCmdLineParam(
    TCLAP::UnlabeledMultiArg<std::string>* arg, 
    std::function<bool(std::vector<std::string>)> f) 
  : m_val({arg, f}) {;}; 

 ~wxExCmdLineParam() {delete m_val.first;};

  const bool Run()
  {
    return 
      m_val.first->getValue().empty() || m_val.second(m_val.first->getValue());
  }
private:
  const std::pair<
    TCLAP::UnlabeledMultiArg<std::string>*, 
    std::function<bool(std::vector<std::string>)>> m_val; 
};

class wxExCmdLineParser : public TCLAP::CmdLine
{
public:
  wxExCmdLineParser(
    const std::string& message,
    char delim,
    const std::string& version,
    bool help)
  : TCLAP::CmdLine(message, delim, version, help) {;};
};

class wxExCmdLineSwitch
{
public:
  wxExCmdLineSwitch(
    TCLAP::SwitchArg* arg, 
    std::function<void(bool)> f)
  : m_val({arg, f}) {;};

 ~wxExCmdLineSwitch() {delete m_val.first;};

  void Run(bool toggle)
  {
    m_val.second(m_val.first->getValue());

    if (toggle)
    {
      wxConfigBase::Get()->Write(m_val.first->getName(), !m_val.first->getValue());
    }
  }
private:
  const std::pair<
    TCLAP::SwitchArg*, 
    std::function<void(bool)>> m_val; 
};

wxExCmdLine::wxExCmdLine(
  const CmdSwitches & s, 
  const CmdOptions & o, 
  const CmdParams & p,
  const std::string& message,
  const std::string &version,
  bool helpAndVersion)
  : m_Parser(new wxExCmdLineParser(
      message, ' ', 
      version.empty() ? wxExGetVersionInfo().GetVersionOnlyString().ToStdString(): version, 
      helpAndVersion))
{
  m_Parser->setExceptionHandling(false);

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
          m_Parser->add(arg);
          m_Options.emplace_back(new wxExCmdLineOption(arg, it->second.second));
          }
          break;

        case CMD_LINE_INT: {
          auto* arg = new TCLAP::ValueArg<int>(
            std::get<0>(it->first), std::get<1>(it->first), std::get<2>(it->first),
            false, -1, "int");
          m_Parser->add(arg);
          m_Options.emplace_back(new wxExCmdLineOption(arg, it->second.second));
          }
          break;
        
        case CMD_LINE_STRING: {
          auto* arg = new TCLAP::ValueArg<std::string>(
            std::get<0>(it->first), std::get<1>(it->first), std::get<2>(it->first),
            false, std::string(), "string");
          m_Parser->add(arg);
          m_Options.emplace_back(new wxExCmdLineOption(arg, it->second.second));
          }
          break;
        
        default: wxExLog() << "unknown type";
      }
    }

    for (auto it = s.rbegin(); it != s.rend(); ++it)
    {
      const bool def = !wxConfigBase::Get()->ReadBool(std::get<1>(it->first), true);
      auto* arg = new TCLAP::SwitchArg(
        std::get<0>(it->first), std::get<1>(it->first), std::get<2>(it->first), def); 
      m_Switches.emplace_back(new wxExCmdLineSwitch(arg, it->second));
      m_Parser->add(arg);
    }

    if (!p.first.first.empty())
    {
      auto* arg = new TCLAP::UnlabeledMultiArg<std::string>(
        p.first.first, p.first.second, false, std::string());
      m_Params.emplace_back(new wxExCmdLineParam(arg, p.second));
      m_Parser->add(arg);
    }
  }
  catch (TCLAP::ArgException& e)
  {
    wxExLog(e) << "tclap";
  }
  catch (TCLAP::ExitException& )
  {
  }
}

wxExCmdLine::~wxExCmdLine()
{
  for (auto& it : m_Options) delete it;
  for (auto& it : m_Params) delete it;
  for (auto& it : m_Switches) delete it;

  delete m_Parser;
}
  
char wxExCmdLine::Delimiter() const
{
  return TCLAP::Arg::delimiter();
}
  
void wxExCmdLine::Delimiter(char c)
{
  TCLAP::Arg::setDelimiter(c);
}

bool wxExCmdLine::Parse(
  const std::string& cmdline, bool toggle, const char delimiter)
{
  Delimiter(delimiter);
  
  try
  {
    if (cmdline.empty())
    {
      m_Parser->parse(wxTheApp->argc, wxTheApp->argv);
    }
    else
    {
      wxExTokenizer tkz(cmdline);
      auto v = tkz.Tokenize<std::vector<std::string>>();
      m_Parser->parse(v);
    }

    for (const auto& it : m_Switches) 
    {
      it->Run(toggle);
    }

    for (const auto& it : m_Options)
    {
      it->Run();
    }

    if (!m_Params.empty() && !m_Params[0]->Run())
    {
      wxExLog() << "could not run params";
      return false;
    }
    
    return true;
  }
  catch (TCLAP::ArgException& e)
  {
    wxExLog(e) << "tclap";
    return false;
  }
  catch (TCLAP::ExitException& )
  {
    return false;
  }
}
