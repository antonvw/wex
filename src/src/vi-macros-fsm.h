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
  struct ssEXPANDING_TEMPLATE;
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
      EXPANDING_TEMPLATE,
    };

    // All events.
    struct evDONE : sc::event< evDONE > {};
    struct evEXPAND_TEMPLATE : sc::event< evEXPAND_TEMPLATE > {};
    struct evRECORD : sc::event< evRECORD > {};

    vi_macros_fsm();

    void expand_variable(const std::string& name, ex* ex);
    void expanding_template();
    bool expanding_variable(const std::string& name, std::string* value) const;

    auto get() const {return m_state;};
    std::string get_macro() const {return m_macro;};
    const auto & get_variable() const {return m_variable;};
    bool is_playback() const {return m_playback;};

    void playback(const std::string& name, ex* ex, int repeat);
    
    bool process_expand(
      ex* ex, 
      const variable& v, 
      std::string& expanded);

    bool record(
      const std::string& macro = std::string(), 
      ex* ex = nullptr);
    
    static void set_macro(const std::string& m) {m_macro = m;};

    void start_recording();

    void state(state_t s) {m_state = s;};

    const std::string state() const {
      return get() == IDLE ? std::string(): state_string(get());};

    static const std::string state_string(state_t state) {
      switch (state)
      {
        case IDLE: return "idle"; 
        case EXPANDING_TEMPLATE: return "template";
        case RECORDING: return "recording";
        default: return "unhandled state";
      };};
  private:
    void set_ask_for_input() const;

    bool m_error {false}, m_playback {false};
    ex* m_ex {nullptr};
    variable m_variable;
    std::string* m_expanded {nullptr};
    static inline std::string m_macro;
    state_t m_state = IDLE;
  };
};
