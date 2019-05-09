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
  struct ssEXPANDING_VARIABLE;
  struct ssIDLE;
  struct ssPLAYINGBACK;
  struct ssPLAYINGBACK_WHILE_RECORDING;
  struct ssRECORDING;

  /// This class offers the state machine 
  /// and initially enters the idle mode.
  class vi_macros_fsm : public sc::state_machine< vi_macros_fsm, ssACTIVE >
  {
  public:
    enum state_t
    {
      IDLE,
      PLAYINGBACK,
      PLAYINGBACK_WHILE_RECORDING,
      RECORDING,
      EXPANDING_TEMPLATE,
      EXPANDING_VARIABLE,
    };

    // All events.
    struct evDONE : sc::event< evDONE > {};
    struct evEXPAND_TEMPLATE : sc::event< evEXPAND_TEMPLATE > {};
    struct evEXPAND_VARIABLE : sc::event< evEXPAND_VARIABLE > {};
    struct evPLAYBACK : sc::event< evPLAYBACK > {};
    struct evRECORD : sc::event< evRECORD > {};

    vi_macros_fsm();

    void expanding_template();

    bool expanding_variable(const std::string& name, std::string* value) const;

    auto get() const {return m_state;};

    int get_count() const {return m_count;};
    
    std::string get_macro() const {return m_macro;};
    
    const auto & get_variable() const {return m_variable;};

    bool is_playback() const {return m_state == PLAYINGBACK;};

    void playback();
    
    /// Process general event.
    bool process(
      const event_base_type& ev, 
      const std::string& macro = std::string(), 
      ex* ex = nullptr, 
      int count = 1);
    
    /// Process expand template variable.
    bool process_expand(
      ex* ex, 
      const variable& v, 
      std::string& expanded);

    void set_error() {m_error = true;};

    void start_recording();

    void state(state_t s) {m_state = s;};

    const std::string state() const {
      return get() == IDLE ? std::string(): state_string(get());};

    static const std::string state_string(state_t state) {
      switch (state)
      {
        case IDLE: return "idle"; 
        case EXPANDING_TEMPLATE: return "template";
        case EXPANDING_VARIABLE: return "variable";
        case PLAYINGBACK: return "playback";
        case PLAYINGBACK_WHILE_RECORDING: return "recording playback";
        case RECORDING: return "recording";
        default: return "unhandled state";
      };};
  private:
    void set_ask_for_input() const;

    int m_count{1};
    bool m_error {false};
    ex* m_ex {nullptr};
    variable m_variable;
    std::string* m_expanded {nullptr};
    static inline std::string m_macro;
    state_t m_state = IDLE;
  };
};
