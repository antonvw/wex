////////////////////////////////////////////////////////////////////////////////
// Name:      cmdline.h
// Purpose:   Declaration of wxExCmdLineParser class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2015 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <functional>
#include <utility>
#include <vector>
#include <wx/any.h>
#include <wx/cmdline.h>
#include <wx/datetime.h>

/// This class offers a command line parser.
class WXDLLIMPEXP_BASE wxExCmdLineParser : public wxCmdLineParser
{
  public:
    /// Switches: 
    /// - pair of option name and description
    /// - pair of flags and process callback of option is found
    typedef std::vector<std::pair<
      std::pair<wxString, wxString>, 
      std::pair<int, std::function<void(bool)>>>> CmdSwitches;

    /// Options: 
    /// - pair of option name and description
    /// - pair of command line param type and process callback of option is found
    typedef std::vector<std::pair<
      std::pair<wxString, wxString>, 
      std::pair<wxCmdLineParamType, std::function<void(wxAny)>>>> CmdOptions;

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
    
    /// Parses the command line and invokes callbacks, returns -1 for help, 
    /// 0 if ok, and a positive value if error occurred. 
    int Parse(bool giveUsage = true) {
      const int res = wxCmdLineParser::Parse(giveUsage);
      if (res == 0) // ok
      {
        for (const auto it : m_Switches) 
        {
          if (Found(it.first.first))
            it.second.second(FoundSwitch(it.first.first) == wxCMD_SWITCH_ON);
        }
        for (const auto it : m_Options) 
        {
          if (Found(it.first.first))
          {
            wxAny any;
            switch (it.second.first)
            {
              case wxCMD_LINE_VAL_DATE:
              {
                wxDateTime val;
                Found(it.first.first, &val);
                any = val;
              }
              break;
              case wxCMD_LINE_VAL_DOUBLE:
              {
                double val;
                Found(it.first.first, &val);
                any = val;
              }
              break;
              case wxCMD_LINE_VAL_NUMBER:
              {
                long val;
                Found(it.first.first, &val);
                any = val;
              }
              break;
              case wxCMD_LINE_VAL_STRING:
              {
                wxString val;
                Found(it.first.first, &val);
                any = val;
              }
              break;
            }
            it.second.second(any);
          }
        }
      }
      return res;};
private:
  const CmdOptions m_Options; 
  const CmdSwitches m_Switches; 
};
