////////////////////////////////////////////////////////////////////////////////
// Name:      vi-macros-fsm.h
// Purpose:   Declaration of class wex::vi_macros_fsm
// Author:    Anton van Wezenbeek
// Copyright: (c) 2018 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <string>
#include <fsm.h>

namespace wex
{
  class ex;
  class variable;

  /// This class holds the table containing the states.
  class vi_macros_fsm
  {
  public:
    enum state
    {
      IDLE,
      PLAYINGBACK,
      PLAYINGBACK_WHILE_RECORDING,
      RECORDING,
      EXPANDING_TEMPLATE,
      EXPANDING_VARIABLE,
    };

    enum trigger
    {
      DONE,
      EXPAND_TEMPLATE,
      EXPAND_VARIABLE,
      PLAYBACK,
      RECORD,
    };

    /// Default constructor.
    vi_macros_fsm();

    /// Transitions according to trigger.
    bool Execute(
      /// trigger
      trigger trigger, 
      /// command
      const std::string& macro = std::string(), 
      // ex component
      ex* ex = nullptr, 
      /// number of times (in case of playback)
      int count = 1);

    /// Expands template variable (transitions into expand and back to idle).
    bool Expand(
      /// ex component
      ex* ex, 
      /// template variable
      const variable& v, 
      /// result
      std::string& expanded);

    /// Returns internal state.
    auto Get() const {return m_fsm.state();};

    /// Returns true if busy playing back.
    bool IsPlayback() const;

    /// Returns internal state as a string.
    const std::string State() const {
      return Get() == IDLE ? std::string(): State(Get());};

    /// Returns any state as a string.
    static const std::string State(state state) {
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

    /// Returns any trigger as a string.
    static const std::string Trigger(trigger trigger) {
      switch (trigger)
      {
        case DONE: return "done";
        case EXPAND_TEMPLATE: return "expand_template";
        case EXPAND_VARIABLE: return "expand_variable";
        case PLAYBACK: return "playback";
        case RECORD: return "record";
        default: return "unhandled trigger";
      };};
  private:
    void ExpandingTemplate();
    void ExpandingVariable();
    bool ExpandingVariable(const std::string& name, std::string* value) const;
    void Playback();
    void SetAskForInput() const;
    void StartRecording();
    void StopRecording();
    static void Verbose(state, state, trigger);

    int m_count{1};
    bool m_error {false}, m_playback {false};
    ex* m_ex {nullptr};
    variable m_variable;
    std::string* m_expanded {nullptr};
    static inline std::string m_macro;

    FSM::Fsm<state, IDLE, trigger> m_fsm;
  };
};
