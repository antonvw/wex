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
  enum class States
  {
    IDLE,
    PLAYINGBACK,
    PLAYINGBACK_WHILE_RECORDING,
    RECORDING,
    EXPANDING_TEMPLATE,
    EXPANDING_VARIABLE,
  };

  enum class Triggers
  {
    DONE,
    EXPAND_TEMPLATE,
    EXPAND_VARIABLE,
    PLAYBACK,
    RECORD,
  };

  class ex;
  class variable;

  /// This class holds the table containing the states.
  class vi_macros_fsm
  {
  public:
    /// Default constructor.
    vi_macros_fsm();

    /// Transitions according to trigger.
    bool Execute(
      /// trigger
      Triggers trigger, 
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
      return Get() == States::IDLE ? std::string(): State(Get());};

    /// Returns any state as a string.
    static const std::string State(States state) {
      switch (state)
      {
        case States::IDLE: return "idle"; 
        case States::EXPANDING_TEMPLATE: return "template";
        case States::EXPANDING_VARIABLE: return "variable";
        case States::PLAYINGBACK: return "playback";
        case States::PLAYINGBACK_WHILE_RECORDING: return "recording playback";
        case States::RECORDING: return "recording";
        default: return "unhandled state";
      };};

    /// Returns any trigger as a string.
    static const std::string Trigger(Triggers trigger) {
      switch (trigger)
      {
        case Triggers::DONE: return "done";
        case Triggers::EXPAND_TEMPLATE: return "expand_template";
        case Triggers::EXPAND_VARIABLE: return "expand_variable";
        case Triggers::PLAYBACK: return "playback";
        case Triggers::RECORD: return "record";
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
    static void Verbose(States, States, Triggers);

    int m_count{1};
    bool m_error {false}, m_playback {false};
    ex* m_ex {nullptr};
    variable m_variable;
    std::string* m_expanded {nullptr};
    static inline std::string m_macro;

    FSM::Fsm<States, States::IDLE, Triggers> m_fsm;
  };
};
