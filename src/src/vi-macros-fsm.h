////////////////////////////////////////////////////////////////////////////////
// Name:      vi-macros-fsm.h
// Purpose:   Declaration of class wex::vi_macros_fsm
// Author:    Anton van Wezenbeek
// Copyright: (c) 2019 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <string>
#include <boost/statechart/state_machine.hpp>

namespace sc = boost::statechart;

namespace wex
{
  class ex;
  class variable;

  // Forward the simple states.
  struct ssACTIVE;
  struct ssIDLE;
  struct ssRECORDING;

  /// This class offers the state machine 
  /// and initially enters the idle mode.
  class vi_macros_fsm : public sc::state_machine< vi_macros_fsm, ssACTIVE >
  {
  public:
    enum state_t
    {
      IDLE,
      RECORDING,
    };

    // All events.
    struct evDONE : sc::event< evDONE > {};
    struct evRECORD : sc::event< evRECORD > {};

    /// Default constructor.
    vi_macros_fsm();

    /// Expands a variable template into a string.
    /// The ex component is used for info.
    bool expand_template(
      const variable& v, 
      ex* ex, 
      std::string& expanded) const;

    /// Expands a variable into a ex component.
    bool expand_variable(const std::string& v, ex* ex) const;

    /// Returns the internal state.
    auto get() const {return m_state;};

    /// Are we playng back?
    bool is_playback() const {return m_playback;};

    /// Plays back macro to ex.
    void playback(const std::string& macro, ex* ex, int repeat);
    
    /// Starts or stops recording a macro.
    void record(const std::string& macro, ex* ex = nullptr);

    /// Sets internal state.
    void state(state_t s);

    /// Returns state string.
    const std::string str() const {
      switch (m_state)
      {
        case IDLE: return std::string(); 
        case RECORDING: return "recording";
        default: return "unhandled state";
      };};
  private:
    bool expanding_variable(
      ex* ex, const std::string& name, std::string* value) const;
    void set_ask_for_input() const;

    bool m_playback {false};
    std::string m_macro;
    state_t m_state {IDLE};
  };
};
