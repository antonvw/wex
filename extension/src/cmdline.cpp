////////////////////////////////////////////////////////////////////////////////
// Name:      cmdline.cpp
// Purpose:   Implementation of wex::cmdline class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <sstream> // for tclap!
#include <variant>
#include <tclap/CmdLine.h>
#include <wx/app.h>
#include <wex/cmdline.h>
#include <wex/config.h>
#include <wex/lexer-props.h>
#include <wex/log.h>
#include <wex/stc.h>
#include <wex/stcdlg.h>
#include <wex/tokenizer.h>
#include <wex/version.h>

namespace wex
{
  template<class... Ts> struct overloaded : Ts... { using Ts::operator()...; };
  template<class... Ts> overloaded(Ts...) -> overloaded<Ts...>;
  
  class cmdline_option
  {
  public:
    cmdline_option(
      TCLAP::ValueArg<float>* f, std::function<void(const std::any& any)> fu)
      : m_val(f), m_f(fu) {;};

    cmdline_option(
      TCLAP::ValueArg<int>* i, std::function<void(const std::any& any)> fu)
      : m_val(i), m_f(fu) {;};

    cmdline_option(
      TCLAP::ValueArg<std::string>* s, std::function<void(const std::any& any)> fu)
      : m_val(s), m_f(fu) {;};

   ~cmdline_option() {
      std::visit(overloaded {
        [](auto arg) {delete arg;}}, m_val);};

    const std::string GetDescription() const {
      return std::visit(overloaded {
        [](auto arg) {return arg->getDescription();}}, m_val);};
    
    const std::string GetName() const {
      return std::visit(overloaded {
        [](auto arg) {return arg->getName();}}, m_val);};

    const std::string GetValue() const {
      return std::visit(overloaded {
        [](auto arg) {return 
          arg->getValue() != -1 ? std::to_string(arg->getValue()): std::string();},
        [](TCLAP::ValueArg<std::string>* arg) {return arg->getValue();},
        }, m_val);};

    void Run(bool save) const
    {
      std::visit(overloaded {
        [&](auto arg) {
          if (const auto v = arg->getValue(); v != -1) 
          {
            m_f(v);

            if (save)
            {
              config(GetName()).set(v);
            }
          };},
        [&](TCLAP::ValueArg<std::string>* arg) {
          if (const auto v = arg->getValue(); !v.empty()) 
          {
            m_f(v);

            if (save)
            {
              config(GetName()).set(v.c_str());
            }
          }},
      }, m_val);
    }
  private:
    std::function<void(const std::any& any)> m_f; 

    const std::variant <
      TCLAP::ValueArg<float>*,
      TCLAP::ValueArg<int>*,
      TCLAP::ValueArg<std::string>*> m_val; 
  };

  class cmdline_param
  {
  public:
    cmdline_param(
      TCLAP::UnlabeledMultiArg<std::string>* arg, 
      std::function<bool(std::vector<std::string>)> f) 
    : m_val({arg, f}) {;}; 

   ~cmdline_param() {delete m_val.first;};

    bool Run() const
    {
      return 
        m_val.first->getValue().empty() || m_val.second(m_val.first->getValue());
    }
  private:
    const std::pair<
      TCLAP::UnlabeledMultiArg<std::string>*, 
      std::function<bool(std::vector<std::string>)>> m_val; 
  };

  class cmdline_parser : public TCLAP::CmdLine
  {
  public:
    cmdline_parser(
      const std::string& message,
      char delim,
      const std::string& version,
      bool help)
    : TCLAP::CmdLine(message, delim, version, help) {;};
  };

  class cmdline_switch
  {
  public:
    cmdline_switch(
      TCLAP::SwitchArg* arg, 
      std::function<void(bool)> f)
    : m_val({arg, f}) {;};

   ~cmdline_switch() {delete m_val.first;};

    const std::string GetDescription() const {return m_val.first->getDescription();};
    
    const std::string& GetName() const {return m_val.first->getName();};

    bool GetValue() const {return *m_val.first;};

    void Run(bool save) const
    {
      const bool def = config(GetName()).get(false);
      
      if (def != GetValue())
      {
        m_val.second(GetValue());

        if (save)
        {
          config(GetName()).set(GetValue());
        }
      }
    }
  private:
    const std::pair<
      TCLAP::SwitchArg*, 
      std::function<void(bool)>> m_val; 
  };
};

wex::cmdline::cmdline(
  const cmd_switches & s, 
  const cmd_options & o, 
  const cmd_params & p,
  const std::string& message,
  const std::string& version,
  bool helpAndVersion)
  : m_Parser(new cmdline_parser(
      message, ' ', 
      version.empty() ? 
        get_version_info().Get(): version, 
      helpAndVersion))
{
  m_Parser->setExceptionHandling(false);

  char c = 'A';

  try
  {
    for (auto it = o.rbegin(); it != o.rend(); ++it)
    {
      std::string flag(it->first[0]);
      size_t p_n{1}, p_d{2}; // par name, description

      if (it->first[0].size() > 1)
      {
        flag = std::string(1, c++);
        p_n--;
        p_d--;
      }
      
      const std::string def_specified = 
        (it->first.size() > p_d + 1 ? it->first.back(): std::string());

      switch (it->second.first)
      {
        case FLOAT: {
          const float def = config(it->first[p_n]).get(
            def_specified.empty() ? (float)-1: (float)std::stod(def_specified));
          
          auto* arg = new TCLAP::ValueArg<float>(
            flag, it->first[p_n], it->first[p_d],
            false, def, "float");
          m_Parser->add(arg);
          m_Options.emplace_back(new cmdline_option(arg, it->second.second));
          }
          break;

        case INT: {
          const int def = config(it->first[p_n]).get(
            def_specified.empty() ? -1: std::stoi(def_specified));
          
          auto* arg = new TCLAP::ValueArg<int>(
            flag, it->first[p_n], it->first[p_d],
            false, def, "int");
          m_Parser->add(arg);
          m_Options.emplace_back(new cmdline_option(arg, it->second.second));
          }
          break;
        
        case STRING: {
          const std::string def = config(it->first[p_n]).get();
          
          auto* arg = new TCLAP::ValueArg<std::string>(
            flag, it->first[p_n], it->first[p_d],
            false, def_specified, "string");
          m_Parser->add(arg);
          m_Options.emplace_back(new cmdline_option(arg, it->second.second));
          }
          break;
        
        default: log() << "unknown type";
      }
    }

    for (auto it = s.rbegin(); it != s.rend(); ++it)
    {
      std::string flag(it->first[0]);
      size_t p_n{1}, p_d{2}; // par name, description

      if (it->first[0].size() > 1)
      {
        flag = std::string(1, c++);
        p_n--;
        p_d--;
      }

      const std::string def_specified = 
        (it->first.size() > p_d + 1 ? it->first.back(): std::string());

      const bool def = config(it->first[p_n]).get(
        def_specified.empty() ? false: (bool)std::stoi(def_specified));
      
      auto* arg = new TCLAP::SwitchArg(
        flag, it->first[p_n], it->first[p_d], def); 
      m_Switches.emplace_back(new cmdline_switch(arg, it->second));
      m_Parser->add(arg);
    }

    if (!p.first.first.empty())
    {
      auto* arg = new TCLAP::UnlabeledMultiArg<std::string>(
        p.first.first, p.first.second, false, std::string());
      m_Params.emplace_back(new cmdline_param(arg, p.second));
      m_Parser->add(arg);
    }
  }
  catch (TCLAP::ArgException& e)
  {
    log(e) << "tclap";
  }
  catch (TCLAP::ExitException& )
  {
  }
  catch (std::exception& e)
  {
    log(e) << "wex::cmdline";
  }
}

wex::cmdline::~cmdline()
{
  for (auto& it : m_Options) delete it;
  for (auto& it : m_Params) delete it;
  for (auto& it : m_Switches) delete it;

  delete m_Parser;
}
  
char wex::cmdline::Delimiter() const
{
  return TCLAP::Arg::delimiter();
}
  
void wex::cmdline::Delimiter(char c)
{
  TCLAP::Arg::setDelimiter(c);
}

bool wex::cmdline::Parse(
  const std::string& cmdline, bool save, const char delimiter)
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
      tokenizer tkz(cmdline);
      auto v = tkz.Tokenize<std::vector<std::string>>();
      m_Parser->parse(v);
    }

    for (const auto& it : m_Switches) 
    {
      it->Run(save);
    }

    for (const auto& it : m_Options)
    {
      it->Run(save);
    }

    if (!m_Params.empty() && !m_Params[0]->Run())
    {
      log() << "could not run params";
      return false;
    }
    
    return true;
  }
  catch (TCLAP::ArgException& e)
  {
    log(e) << "tclap";
    return false;
  }
  catch (TCLAP::ExitException& )
  {
    return false;
  }
  catch (std::exception& e)
  {
    log(e) << "parse";
    return false;
  }
}

void wex::cmdline::ShowOptions(const window_data& data) const
{
  static stc_entry_dialog* dlg = nullptr;
  const lexer_props l;

  if (dlg == nullptr)
  {
    dlg = new stc_entry_dialog(
      std::string(),
      std::string(),
      window_data(data).Button(wxOK).Title("Options"));

    dlg->GetSTC()->GetLexer().Set(l);
  }
  else
  {
    dlg->GetSTC()->ClearAll();
  }

  for (const auto& it : m_Options)
  {
    dlg->GetSTC()->InsertText(0, l.MakeKey(
      it->GetName(), 
      it->GetValue(), 
      it->GetDescription() != it->GetName() ? it->GetDescription(): std::string()));
  }

  dlg->GetSTC()->InsertText(0, "\n" + l.MakeSection("options"));
  
  for (const auto& it : m_Switches)
  {
    dlg->GetSTC()->InsertText(0, l.MakeKey(
      it->GetName(), 
      std::to_string(it->GetValue()), 
      it->GetDescription() != it->GetName() ? it->GetDescription(): std::string()));
  }

  dlg->GetSTC()->InsertText(0, l.MakeSection("switches"));
  dlg->GetSTC()->EmptyUndoBuffer();
  dlg->ShowModal();
}
