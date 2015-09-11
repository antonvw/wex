////////////////////////////////////////////////////////////////////////////////
// Name:      cmdline.h
// Purpose:   Declaration of wxExCmdLineParser class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <utility>
#include <vector>
#include <wx/cmdline.h>

/// This class offers a command line parser.
class WXDLLIMPEXP_BASE wxExCmdLineParser : public wxCmdLineParser
{
  public:
    typedef std::vector<std::pair<
      std::pair<wxString, wxString>, 
      std::pair<int, std::function<void(bool)>>>> CmdSwitches;
    typedef std::vector<std::pair<
      std::pair<wxString, wxString>, 
      std::pair<wxCmdLineParamType, std::function<void()>>>> CmdOptions;

    /// Contructor, specify switches and options.
    wxExCmdLineParser(
      const wxString& cmdline, const CmdSwitches & s, const CmdOptions & o) 
      : wxCmdLineParser(cmdline) 
      , m_Switches(s) 
      , m_Options(o) {
      for (const auto it : s) 
      {
        AddSwitch(it.first.first, wxEmptyString, it.first.second, it.second.first);
      };
      for (const auto it : o) 
      {
        AddOption(it.first.first, wxEmptyString, it.first.second, it.second.first);
      };
      if (!s.empty() || !o.empty())
      {
        AddSwitch("h", wxEmptyString, "help", wxCMD_LINE_OPTION_HELP);
      }}

    /// Constructor from argc and argv.
    wxExCmdLineParser(
      int argc, char **argv, const CmdSwitches & s, const CmdOptions & o)
      : wxExCmdLineParser(wxEmptyString, s, o) {SetCmdLine(argc, argv);};
    
    /// Parse the command line and invokes callbacks, return false if error occurred. 
    bool Parse(bool giveUsage = true) {
      switch (wxCmdLineParser::Parse(giveUsage))
      {
        case -1: return true; // help
        case 0: break; // ok 
        default: return false; // error
      }
      for (const auto it : m_Switches) 
      {
        Switch(it.first.first, it.second.second);
      }
      for (const auto it : m_Options) 
      {
        if (Found(it.first.first))
          it.second.second();
      }
      return true;};
private:
  void Switch(const wxString& name, std::function<void(bool)> process ) const {
    if (Found(name))
      process((FoundSwitch(name) == wxCMD_SWITCH_ON));};
  
  const CmdOptions m_Options; 
  const CmdSwitches m_Switches; 
};
