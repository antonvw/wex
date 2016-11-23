////////////////////////////////////////////////////////////////////////////////
// Name:      cmdline.h
// Purpose:   Declaration of wxExCmdLineParser class
// Author:    Anton van Wezenbeek
// Copyright: (c) 2016 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <functional>
#include <utility>
#include <vector>
#include <wx/any.h>
#include <wx/app.h>
#include <wx/cmdline.h>
#include <wx/datetime.h>

/// This class offers a command line parser.
class WXDLLIMPEXP_BASE wxExCmdLineParser : public wxCmdLineParser
{
  public:
    /// Switches: 
    typedef std::vector<std::pair<
      /// pair of option name and description
      std::pair<wxString, wxString>, 
      /// pair of flags and process callback if option is found
      std::pair<
        /// flags:
        /// - wxCMD_LINE_OPTION_MANDATORY 	
        /// - wxCMD_LINE_OPTION_HELP 	
        /// - wxCMD_LINE_SWITCH_NEGATABLE 
        int, 
        std::function<void(bool)>>>> CmdSwitches;

    /// Options: 
    typedef std::vector<std::pair<
      /// pair of option name and description
      std::pair<wxString, wxString>, 
      /// pair of command line param type and process callback if option is found
      std::pair<
        /// type:
        /// - wxCMD_LINE_VAL_STRING 	
        /// - wxCMD_LINE_VAL_NUMBER 	
        /// - wxCMD_LINE_VAL_DATE 	
        /// - wxCMD_LINE_VAL_DOUBLE 	
        /// - wxCMD_LINE_VAL_NONE 	
        wxCmdLineParamType, 
        std::function<void(wxAny)>>>> CmdOptions;

    /// Params (currently only string value supported): 
    typedef std::vector<std::pair<
      /// description
      wxString, 
      /// pair of flags and process callback if param is present
      std::pair<
        /// flags:
        /// - wxCMD_LINE_PARAM_OPTIONAL 	
        /// - wxCMD_LINE_PARAM_MULTIPLE 	
        /// - wxCMD_LINE_NEEDS_SEPARATOR 	
        int, 
        std::function<void(std::vector<std::string> &)>>>> CmdParams;
  
    /// Contructor, 
    /// Default (i.e. if flags are just 0), options are optional.
    /// If you specify an option name of max short_option_size, it is considered a
    /// short option, if you specify more chars, it is considered
    /// a long option.
    wxExCmdLineParser(
      /// the command line to be parsed
      const wxString& cmdline, 
      /// switches
      const CmdSwitches & s, 
      /// options
      const CmdOptions & o, 
      /// params
      const CmdParams & p = CmdParams(),
      /// default size for short options
      size_t short_option_size = 2) 
      : wxCmdLineParser(cmdline) 
      , m_Switches(s) 
      , m_Options(o)
      , m_Params(p) {
      for (const auto it : s) 
      {
        AddSwitch(
          it.first.first.size() <= short_option_size ? it.first.first: wxString(), 
          it.first.first.size() <= short_option_size ? wxString(): it.first.first, 
          it.first.second, it.second.first);
      };
      for (const auto it : o) 
      {
        AddOption(
          it.first.first.size() <= short_option_size ? it.first.first: wxString(), 
          it.first.first.size() <= short_option_size ? wxString(): it.first.first, 
          it.first.second, it.second.first);
      };
      for (const auto it : p) 
      {
        if (!it.first.empty())
        {
          AddParam(it.first, wxCMD_LINE_VAL_STRING, it.second.first);
        }
      };
      if (!s.empty() || !o.empty() || !p.empty())
      {
        AddSwitch("h", wxEmptyString, "help", wxCMD_LINE_OPTION_HELP);
      }}

    /// Constructor using command line from wxTheApp.
    wxExCmdLineParser(
      const CmdSwitches & s, const CmdOptions & o, const CmdParams & p = CmdParams())
      : wxExCmdLineParser(wxEmptyString, s, o, p) {
      SetCmdLine(wxTheApp->argc, wxTheApp->argv);};
    
    /// Parses the command line and invokes callbacks, returns:
    /// - -1 for help, 
    /// - 0 if ok, 
    /// - a positive value if an error occurred. 
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
              default: ; // do nothing
            }
            it.second.second(any);
          }
        }
        for (const auto it : m_Params) 
        {
          if (!it.first.empty())
          {
            std::vector<std::string> v;
            for (size_t i = 0; i < GetParamCount(); i++)
            {
              v.emplace_back(GetParam(i).ToStdString());
            }
            it.second.second(v);
          }
        }
      }
      return res;};
private:
  const CmdOptions m_Options; 
  const CmdParams m_Params; 
  const CmdSwitches m_Switches; 
};
