////////////////////////////////////////////////////////////////////////////////
// Name:      addressrange.h
// Purpose:   Declaration of class wex::addressrange
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <vector> 
#include <wex/address.h>
#include <wex/indicator.h>

namespace wex
{
  class process;
  class stc;

  /// Offers an address range for vi (ex).
  /// - The range is derived from a number of lines, 
  /// - or by a range string (including visual range for 
  ///   already selected text on the STC component).
  /// All methods return false if the range is not ok.
  class addressrange
  {
  public:
    /// Constructor for a range from current position 
    /// extending with number of lines.
    addressrange(
      /// the ex (or vi) component
      ex* ex, 
      /// lines 1 is current line only
      /// lines 0 is illegal
      int lines = 1);
    
    /// Constructor for a range (including visual range).
    addressrange(
      /// the ex (or vi) component
      ex* ex, 
      /// the range, being a string containing:
      /// - .   : current line 
      /// - %   : entire document
      /// - *   : current screen visible area
      /// - x,y : range from begin x and end y address.
      /// -     : (empty), the range is empty
      const std::string& range);

    /// Changes range into text.
    bool change(const std::string& text) const;
    
    /// Copies range to destination.
    bool copy(const address& destination) const;
    
    /// Deletes range.
    bool erase(bool show_message = true) const;
    
    /// Filters range with command.
    /// The address range is used as input for the command,
    /// and the output of the command replaces the address range.
    /// For example: addressrange(96, 99).escape("sort")
    /// or (ex command::96,99!sort)
    /// will pass lines 96 through 99 through the sort filter and 
    /// replace those lines with the output of sort.  
    /// Of course, you could also do: addressrange(96,99).Sort().
    /// If you did not specify an address range,
    /// the command is run as an asynchronous process.
    bool escape(const std::string& command);

    /// Performs the global command on this range.
    bool global(
      /// command
      const std::string& command, 
      /// normally performs command on each match, if inverse 
      /// performs command if line does not match
      bool inverse = false) const;
    
    /// Is this range ok.
    bool is_ok() const;
    
    /// joins range.
    bool join() const;
    
    /// moves range to destination.
    bool move(const address& destination) const;
    
    /// Cleans up (process).
    static void on_exit();
    
    /// Prints range to print file.
    bool print(const std::string& flags = std::string()) const;
    
    /// Acccess to the process.
    /// Returns NULLL if escape without range was not yet invoked.
    static process* process() {return m_Process;};
    
    /// Shifts the specified lines to the start of the line.
    bool shift_left() const {return Indent(false);};

    /// Shifts the specified lines away from the start of the line.
    bool shift_right() const {return Indent(true);};

    /// Sorts range, with optional parameters:
    /// -u to sort unique lines
    /// -r to sort reversed (descending)
    ///  - x,y sorts rectangle within range: x start col, y end col (exclusive).
    bool sort(const std::string& parameters = std::string()) const;
    
    /// substitutes range.
    bool substitute(
      /// text format: /pattern/replacement/options
      /// Pattern might contain:
      /// - $ to match a line end
      /// Replacement might contain:
      /// - & or \\0 to represent the target in the replacement
      /// - \\U to convert target to uppercase 
      /// - \\L to convert target to lowercase
      /// Options can be:
      /// - c : Ask for confirm
      /// - i : Case insensitive
      /// - g : Do global on line, without this flag replace first match only
      /// e.g. /$/EOL appends the string EOL at the end of each line. 
      /// Merging is not yet possible using a \n target,
      /// you can create a macro for that.  
      const std::string& text,
      /// cmd is one of s, & or ~
      /// - s : default, normal substitute
      /// - & : repeat last substitute (text contains options)
      /// - ~ : repeat last substitute with pattern from find replace data (text contains options)
      const char cmd = 's');
      
    /// Writes range to filename.
    bool write(const std::string& filename) const;
    
    /// Yanks range to register, default to yank register.
    bool yank(const char name = '0') const;
  private:  
    const std::string BuildReplacement(const std::string& text) const;
    int Confirm(const std::string& pattern, const std::string& replacement);
    /// Indents range.
    bool Indent(bool forward = true) const;
    bool Parse(const std::string& command, 
      std::string& pattern, std::string& replacement, std::string& options) const;
    void Set(const std::string& begin, const std::string& end) {
      m_Begin.m_Address = begin;
      const int begin_line = m_Begin.get_line();
      if (begin_line > 0) m_Begin.SetLine(begin_line);
      m_End.m_Address = end;
      const int end_line = m_End.get_line();
      if (end_line > 0) m_End.SetLine(end_line);};
    void Set(int begin, int end) {
      m_Begin.SetLine(begin);
      m_End.SetLine(end);};
    void Set(address& begin, address& end, int lines);
    bool SetSelection() const;

    static inline std::string m_Pattern;
    static inline std::string m_Replacement;
    static inline wex::process* m_Process {nullptr};
    
    const indicator m_FindIndicator {indicator(0)};

    address m_Begin, m_End;
    ex* m_Ex;
    stc* m_STC;
  };
};
