////////////////////////////////////////////////////////////////////////////////
// Name:      macro-fsm.h
// Purpose:   Declaration of class wex::macro_fsm
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/statechart/state_machine.hpp>
#include <string>

namespace sc = boost::statechart;

namespace wex
{
class ex;
class variable;
class macro_mode;

// Forward the simple states.
struct ssmACTIVE;
struct ssmIDLE;
struct ssmRECORDING;

/// This class offers the state machine
/// and initially enters the idle mode.
class macro_fsm : public sc::state_machine<macro_fsm, ssmACTIVE>
{
public:
  enum state_t
  {
    IDLE,
    RECORDING,
  };

  // All events.
  struct evRECORD : sc::event<evRECORD>
  {
  };

  /// Constructor.
  macro_fsm(macro_mode* mode);

  /// Expands a variable template into a string.
  /// The ex component is used for info.
  bool expand_template(const variable& v, ex* ex, std::string& expanded);

  /// Expands a variable into a ex component.
  bool expand_variable(const std::string& v, ex* ex) const;

  /// Returns the internal state.
  auto get() const { return m_state; }

  /// Returns the macro.
  auto& get_macro() const { return m_macro; }

  /// Are we playing back?
  bool is_playback() const { return m_playback; }

  /// Plays back macro to ex.
  void playback(const std::string& macro, ex* ex, int repeat);

  /// Starts or stops recording a macro.
  void record(const std::string& macro, ex* ex = nullptr);

  /// Callback after recording.
  void recorded();

  /// Sets internal state.
  void state(state_t s);

  /// Returns state string.
  const std::string str() const
  {
    switch (m_state)
    {
      case IDLE:
        return std::string();
      case RECORDING:
        return "recording";
      default:
        return "unhandled state";
    };
  };

private:
  bool
  expanding_variable(ex* ex, const std::string& name, std::string* value) const;
  std::string read_variable(
    std::ifstream&  ifs,
    const char      separator,
    ex*             ex,
    const variable& current);
  void set_ask_for_input() const;

  bool        m_playback{false};
  std::string m_macro;
  state_t     m_state{IDLE};
  macro_mode* m_mode;
};
}; // namespace wex
