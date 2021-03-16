////////////////////////////////////////////////////////////////////////////////
// Name:      vi/mode.cpp
// Purpose:   Implementation of class wex::vi_mode
// Author:    Anton van Wezenbeek
// Copyright: (c) 2020 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <algorithm>
#include <boost/mpl/list.hpp>
#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/transition.hpp>
#include <wex/config.h>
#include <wex/log.h>
#include <wex/macro-mode.h>
#include <wex/macros.h>
#include <wex/frame.h>
#include <wex/statusbar.h>
#include <wex/stc.h>
#include <wex/vi-mode.h>
#include <wex/vi.h>

namespace mpl = boost::mpl;
namespace sc  = boost::statechart;

namespace wex
{
  struct ssACTIVE;

  /// This class offers the vi mode state machine
  /// and initially enters the active mode.
  class vi_fsm : public sc::state_machine<vi_fsm, ssACTIVE>
  {
  public:
    vi_fsm(
      stc*                                            stc,
      std::function<void(const std::string& command)> insert,
      std::function<void()>                           command)
      : m_stc(stc)
      , m_f_insert(insert)
      , m_f_command(command)
    {
      ;
    };

    stc* get_stc() { return m_stc; };

    void insert_mode()
    {
      if (m_f_insert != nullptr)
      {
        m_f_insert(m_command);
      }
    };

    void command_mode()
    {
      if (m_f_command != nullptr)
      {
        m_f_command();
      }
    };

    void process(const std::string& command, const event_base_type& ev)
    {
      m_command = command;
      process_event(ev);
    };

    const vi_mode::state_t state() const { return m_state; };

    void state(vi_mode::state_t s) { m_state = s; };

    const std::string state_string() const
    {
      switch (state())
      {
        case vi_mode::state_t::INSERT:
          return m_stc != nullptr && m_stc->GetOvertype() ? "replace" :
                                                            "insert";
        case vi_mode::state_t::INSERT_BLOCK:
          return m_stc != nullptr && m_stc->GetOvertype() ? "replace block" :
                                                            "insert block";
        case vi_mode::state_t::VISUAL:
          return "visual";

        case vi_mode::state_t::VISUAL_LINE:
          return "visual line";

        case vi_mode::state_t::VISUAL_BLOCK:
          return "visual block";

        default:
          return ex::get_macros().mode().str();
      }
    };

  private:
    std::string                                     m_command;
    std::function<void(const std::string& command)> m_f_insert{nullptr};
    std::function<void()>                           m_f_command{nullptr};
    vi_mode::state_t                                m_state{vi_mode::COMMAND};
    stc*                                            m_stc;
  };

  // All events.
  struct evESCAPE : sc::event<evESCAPE>
  {
  };
  struct evINSERT : sc::event<evINSERT>
  {
  };
  struct evVISUAL : sc::event<evVISUAL>
  {
  };
  struct evVISUAL_LINE : sc::event<evVISUAL_LINE>
  {
  };
  struct evVISUAL_BLOCK : sc::event<evVISUAL_BLOCK>
  {
  };

  // Forward the simple states.
  struct ssCOMMAND;
  struct ssTEXTINPUT;
  struct ssVISUAL;
  struct ssVISUAL_LINE;
  struct ssVISUAL_MODE;
  struct ssVISUAL_BLOCK;
  struct ssVISUAL_BLOCK_TEXTINPUT;

  // Implement the simple states.
  struct ssACTIVE : sc::simple_state<ssACTIVE, vi_fsm, ssCOMMAND>
  {
  };

  struct ssCOMMAND : sc::state<ssCOMMAND, ssACTIVE>
  {
    typedef mpl::list<
      sc::transition<evESCAPE, ssCOMMAND>,
      sc::custom_reaction<evINSERT>,
      sc::transition<evVISUAL, ssVISUAL_MODE>,
      sc::transition<evVISUAL_LINE, ssVISUAL_LINE>,
      sc::transition<evVISUAL_BLOCK, ssVISUAL_BLOCK>>
      reactions;

    explicit ssCOMMAND(my_context ctx)
      : my_base(ctx)
    {
      log::trace("vi mode") << "command";
      context<vi_fsm>().state(vi_mode::COMMAND);
    };

    sc::result react(const evINSERT&)
    {
      return context<vi_fsm>().get_stc()->GetReadOnly() ?
               forward_event() :
               transit<ssTEXTINPUT>();
    };
  };

  struct ssTEXTINPUT : sc::state<ssTEXTINPUT, ssACTIVE>
  {
    typedef sc::custom_reaction<evESCAPE> reactions;

    explicit ssTEXTINPUT(my_context ctx)
      : my_base(ctx)
    {
      log::trace("vi mode") << "insert";
      context<vi_fsm>().state(vi_mode::INSERT);
      context<vi_fsm>().insert_mode();
    };

    sc::result react(const evESCAPE&)
    {
      context<vi_fsm>().command_mode();
      return transit<ssCOMMAND>();
    };
  };

  struct ssVISUAL_MODE : sc::simple_state<ssVISUAL_MODE, ssACTIVE, ssVISUAL>
  {
    typedef mpl::list<
      sc::transition<evESCAPE, ssCOMMAND>,
      sc::transition<evVISUAL, ssVISUAL>,
      sc::transition<evVISUAL_LINE, ssVISUAL_LINE>,
      sc::transition<evVISUAL_BLOCK, ssVISUAL_BLOCK>>
      reactions;
  };

  struct ssVISUAL : sc::state<ssVISUAL, ssVISUAL_MODE>
  {
    explicit ssVISUAL(my_context ctx)
      : my_base(ctx)
    {
      log::trace("vi mode") << "visual";
      context<vi_fsm>().state(vi_mode::VISUAL);
    };
  };

  struct ssVISUAL_LINE : sc::state<ssVISUAL_LINE, ssVISUAL_MODE>
  {
    explicit ssVISUAL_LINE(my_context ctx)
      : my_base(ctx)
    {
      log::trace("vi mode") << "visual line";
      context<vi_fsm>().state(vi_mode::VISUAL_LINE);
    };
  };

  struct ssVISUAL_BLOCK : sc::state<ssVISUAL_BLOCK, ssVISUAL_MODE>
  {
    typedef sc::transition<evINSERT, ssVISUAL_BLOCK_TEXTINPUT> reactions;

    explicit ssVISUAL_BLOCK(my_context ctx)
      : my_base(ctx)
    {
      log::trace("vi mode") << "visual block";
      context<vi_fsm>().state(vi_mode::VISUAL_BLOCK);
    };
  };

  struct ssVISUAL_BLOCK_TEXTINPUT
    : sc::state<ssVISUAL_BLOCK_TEXTINPUT, ssVISUAL_MODE>
  {
    typedef sc::transition<evESCAPE, ssVISUAL_BLOCK> reactions;

    explicit ssVISUAL_BLOCK_TEXTINPUT(my_context ctx)
      : my_base(ctx)
    {
      log::trace("vi mode") << "visual block insert";
      context<vi_fsm>().state(vi_mode::INSERT_BLOCK);
      context<vi_fsm>().insert_mode();
    };
  };
}; // namespace wex

#define NAVIGATE(SCOPE, DIRECTION) m_vi->get_stc()->SCOPE##DIRECTION();

wex::vi_mode::vi_mode(
  vi*                                     vi,
  std::function<void(const std::string&)> insert,
  std::function<void()>                   command)
  : m_vi(vi)
  , m_fsm(std::make_unique<vi_fsm>(vi->get_stc(), insert, command))
  , m_insert_commands{
      {'a',
       [&]() {
         NAVIGATE(Char, Right);
       }},
      {'c',
       [&]() {
         ;
       }},
      {'i',
       [&]() {
         ;
       }},
      {'o',
       [&]() {
         NAVIGATE(Line, End);
         m_vi->get_stc()->NewLine();
       }},
      {'A',
       [&]() {
         NAVIGATE(Line, End);
       }},
      {'C',
       [&]() {
         m_vi->get_stc()->LineEndExtend();
         m_vi->cut();
       }},
      {'I',
       [&]() {
         NAVIGATE(Line, Home);
       }},
      {'O',
       [&]() {
         NAVIGATE(Line, Home);
         m_vi->get_stc()->NewLine();
         NAVIGATE(Line, Up);
       }},
      {'R', [&]() {
         m_vi->get_stc()->SetOvertype(true);
       }}}
{
  m_fsm->initiate();
}

wex::vi_mode::~vi_mode() {}

bool wex::vi_mode::escape()
{
  std::string command("\x1b");
  return transition(command);
}

wex::vi_mode::state_t wex::vi_mode::get() const
{
  return m_fsm->state();
}

bool wex::vi_mode::is_insert() const
{
  return get() == INSERT || get() == INSERT_BLOCK;
}

bool wex::vi_mode::is_visual() const
{
  return get() == VISUAL || get() == VISUAL_LINE || get() == VISUAL_BLOCK;
}

bool wex::vi_mode::transition(std::string& command)
{
  if (command.empty())
  {
    return false;
  }
  else if (command[0] == 'c')
  {
    if (command.size() == 1)
    {
      return false;
    }
    else
    {
      if (command.size() == 2 && (command[1] == 'f' || command[1] == 'F'))
      {
        return false;
      }
    }
  }

  if (
    std::find_if(
      m_insert_commands.begin(),
      m_insert_commands.end(),
      [command](auto const& e) {
        return e.first == command[0];
      }) != m_insert_commands.end())
  {
    m_fsm->process(command, evINSERT());
  }
  else
    switch (command[0])
    {
      case 'K':
      case WXK_CONTROL_V:
        m_fsm->process(command, evVISUAL_BLOCK());
        break;

      case 'v':
        m_fsm->process(command, evVISUAL());
        break;

      case 'V':
        m_fsm->process(command, evVISUAL_LINE());
        break;

      case WXK_ESCAPE:
        m_fsm->process(command, evESCAPE());
        break;

      default:
        return false;
    }

  switch (get())
  {
    case INSERT:
      if (!m_vi->get_stc()->is_hexmode())
      {
        if (const auto& it = std::find_if(
              m_insert_commands.begin(),
              m_insert_commands.end(),
              [command](auto const& e) {
                return e.first == command[0];
              });
            it != m_insert_commands.end() && it->second != nullptr)
        {
          it->second();
          m_vi->append_insert_command(command.substr(0, 1));
        }
      }
      break;

    case VISUAL_LINE:
      if (m_vi->get_stc()->SelectionIsRectangle())
      {
        m_vi->get_stc()->Home();
        m_vi->get_stc()->LineDownExtend();
      }
      else if (m_vi->get_stc()->GetSelectedText().empty())
      {
        m_vi->get_stc()->Home();
        m_vi->get_stc()->LineDownExtend();
      }
      else
      {
        const auto selstart =
          m_vi->get_stc()->PositionFromLine(m_vi->get_stc()->LineFromPosition(
            m_vi->get_stc()->GetSelectionStart()));
        const auto selend = m_vi->get_stc()->PositionFromLine(
          m_vi->get_stc()->LineFromPosition(
            m_vi->get_stc()->GetSelectionEnd()) +
          1);
        m_vi->get_stc()->SetSelection(selstart, selend);
        m_vi->get_stc()->HomeExtend();
      }
      break;

    default:
      break;
  }

  m_vi->frame()->get_statusbar()->pane_show(
    "PaneMode",
    (!is_command() || ex::get_macros().mode().is_recording()) &&
      config(_("stc.Show mode")).get(true));
  m_vi->frame()->statustext(str(), "PaneMode");

  command.erase(0, 1);

  return true;
}

const std::string wex::vi_mode::str() const
{
  return m_fsm->state_string();
}
