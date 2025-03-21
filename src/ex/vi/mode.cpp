////////////////////////////////////////////////////////////////////////////////
// Name:      mode.cpp
// Purpose:   Implementation of class wex::vi_mode
// Author:    Anton van Wezenbeek
// Copyright: (c) 2017-2025 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/mpl/list.hpp>
#include <boost/statechart/custom_reaction.hpp>
#include <boost/statechart/state.hpp>
#include <boost/statechart/state_machine.hpp>
#include <boost/statechart/transition.hpp>
#include <wex/core/config.h>
#include <wex/core/log.h>
#include <wex/ex/macro-mode.h>
#include <wex/ex/macros.h>
#include <wex/syntax/stc.h>
#include <wex/ui/frame.h>
#include <wex/ui/statusbar.h>
#include <wex/vi/mode.h>
#include <wex/vi/vi.h>

#include <algorithm>
#include <utility>

namespace mpl = boost::mpl;
namespace sc  = boost::statechart;

namespace wex
{
struct ssvACTIVE;

/// This class offers the vi mode state machine
/// and initially enters the active mode.
class vi_fsm : public sc::state_machine<vi_fsm, ssvACTIVE>
{
public:
  vi_fsm(
    factory::stc*                                   stc,
    std::function<void(const std::string& command)> insert,
    std::function<void()>                           command)
    : m_stc(stc)
    , m_f_insert(std::move(insert))
    , m_f_command(std::move(command))
  {
    ;
  };

  auto* get_stc() { return m_stc; }

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

  const auto state() const { return m_state; }

  void state(vi_mode::state_t s) { m_state = s; }

  const std::string state_string() const
  {
    switch (state())
    {
      case vi_mode::state_t::INSERT:
        return m_stc != nullptr && m_stc->GetOvertype() ? "replace" : "insert";

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
  vi_mode::state_t m_state{vi_mode::state_t::COMMAND};
  factory::stc*    m_stc;
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
struct ssvCOMMAND;
struct ssvTEXTINPUT;
struct ssvVISUAL;
struct ssvVISUAL_LINE;
struct ssvVISUAL_MODE;
struct ssvVISUAL_BLOCK;
struct ssvVISUAL_BLOCK_TEXTINPUT;

// Implement the simple states.
struct ssvACTIVE : sc::simple_state<ssvACTIVE, vi_fsm, ssvCOMMAND>
{
};

struct ssvCOMMAND : sc::state<ssvCOMMAND, ssvACTIVE>
{
  using reactions = mpl::list<
    sc::transition<evESCAPE, ssvCOMMAND>,
    sc::custom_reaction<evINSERT>,
    sc::transition<evVISUAL, ssvVISUAL_MODE>,
    sc::transition<evVISUAL_LINE, ssvVISUAL_LINE>,
    sc::transition<evVISUAL_BLOCK, ssvVISUAL_BLOCK>>;

  explicit ssvCOMMAND(my_context ctx)
    : my_base(std::move(ctx))
  {
    context<vi_fsm>().state(vi_mode::state_t::COMMAND);
  };

  sc::result react(const evINSERT&)
  {
    return context<vi_fsm>().get_stc()->GetReadOnly() ? forward_event() :
                                                        transit<ssvTEXTINPUT>();
  };
};

struct ssvTEXTINPUT : sc::state<ssvTEXTINPUT, ssvACTIVE>
{
  using reactions = sc::custom_reaction<evESCAPE>;

  explicit ssvTEXTINPUT(my_context ctx)
    : my_base(std::move(ctx))
  {
    context<vi_fsm>().state(vi_mode::state_t::INSERT);
    context<vi_fsm>().insert_mode();
  };

  sc::result react(const evESCAPE&)
  {
    context<vi_fsm>().command_mode();
    return transit<ssvCOMMAND>();
  };
};

struct ssvVISUAL_MODE : sc::simple_state<ssvVISUAL_MODE, ssvACTIVE, ssvVISUAL>
{
  using reactions = mpl::list<
    sc::transition<evESCAPE, ssvCOMMAND>,
    sc::transition<evVISUAL, ssvVISUAL>,
    sc::transition<evVISUAL_LINE, ssvVISUAL_LINE>,
    sc::transition<evVISUAL_BLOCK, ssvVISUAL_BLOCK>>;
};

struct ssvVISUAL : sc::state<ssvVISUAL, ssvVISUAL_MODE>
{
  explicit ssvVISUAL(my_context ctx)
    : my_base(std::move(ctx))
  {
    context<vi_fsm>().state(vi_mode::state_t::VISUAL);
  };
};

struct ssvVISUAL_LINE : sc::state<ssvVISUAL_LINE, ssvVISUAL_MODE>
{
  explicit ssvVISUAL_LINE(my_context ctx)
    : my_base(std::move(ctx))
  {
    context<vi_fsm>().state(vi_mode::state_t::VISUAL_LINE);
  };
};

struct ssvVISUAL_BLOCK : sc::state<ssvVISUAL_BLOCK, ssvVISUAL_MODE>
{
  using reactions = sc::transition<evINSERT, ssvVISUAL_BLOCK_TEXTINPUT>;

  explicit ssvVISUAL_BLOCK(my_context ctx)
    : my_base(std::move(ctx))
  {
    context<vi_fsm>().state(vi_mode::state_t::VISUAL_BLOCK);
  };
};

struct ssvVISUAL_BLOCK_TEXTINPUT
  : sc::state<ssvVISUAL_BLOCK_TEXTINPUT, ssvVISUAL_MODE>
{
  using reactions = sc::transition<evESCAPE, ssvVISUAL_BLOCK>;

  explicit ssvVISUAL_BLOCK_TEXTINPUT(my_context ctx)
    : my_base(std::move(ctx))
  {
    context<vi_fsm>().state(vi_mode::state_t::INSERT_BLOCK);
    context<vi_fsm>().insert_mode();
  };
};
}; // namespace wex

#define NAVIGATE(SCOPE, DIRECTION) m_vi->get_stc()->SCOPE##DIRECTION();

wex::vi_mode::vi_mode(
  vi*                                            vi,
  const std::function<void(const std::string&)>& insert,
  const std::function<void()>&                   command)
  : m_vi(vi)
  , m_fsm(std::make_unique<vi_fsm>(vi->get_stc(), insert, command))
  , m_insert_commands{
      {'a',
       [&]()
       {
         NAVIGATE(Char, Right);
       }},
      {'c',
       [&]()
       {
         ;
       }},
      {'i',
       [&]()
       {
         ;
       }},
      {'o',
       [&]()
       {
         NAVIGATE(Line, End);
         m_vi->get_stc()->NewLine();
       }},
      {'A',
       [&]()
       {
         NAVIGATE(Line, End);
       }},
      {'C',
       [&]()
       {
         m_vi->get_stc()->LineEndExtend();
         m_vi->cut();
       }},
      {'I',
       [&]()
       {
         NAVIGATE(Line, Home);
       }},
      {'O',
       [&]()
       {
         NAVIGATE(Line, Home);
         m_vi->get_stc()->NewLine();
         NAVIGATE(Line, Up);
       }},
      {'R',
       [&]()
       {
         m_vi->get_stc()->SetOvertype(true);
       }}}
{
  m_fsm->initiate();
}

wex::vi_mode::~vi_mode() = default;

void wex::vi_mode::command()
{
  if (get() == state_t::COMMAND)
  {
    return;
  }

  escape();

  if (get() == state_t::COMMAND)
  {
    return;
  }

  escape();

  if (get() != state_t::COMMAND)
  {
    log("vi command mode") << m_fsm->state_string();
  }
}

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
  return get() == state_t::INSERT || get() == state_t::INSERT_BLOCK;
}

bool wex::vi_mode::is_visual() const
{
  return get() == state_t::VISUAL || get() == state_t::VISUAL_LINE ||
         get() == state_t::VISUAL_BLOCK;
}

const std::string wex::vi_mode::str() const
{
  return m_fsm->state_string();
}

bool wex::vi_mode::transition(std::string& command)
{
  if (!transition_prep(command))
  {
    return false;
  }

  if (
    std::ranges::find_if(
      m_insert_commands,
      [command](auto const& e)
      {
        return e.first == command[0];
      }) != m_insert_commands.end())
  {
    m_fsm->process(command, evINSERT());
  }
  else
  {
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
  }

  switch (get())
  {
    case state_t::INSERT:
      if (!m_vi->get_stc()->is_hexmode())
      {
        if (const auto& it = std::ranges::find_if(
              m_insert_commands,
              [command](auto const& e)
              {
                return e.first == command[0];
              });
            it != m_insert_commands.end() && it->second != nullptr)
        {
          it->second();
          m_vi->append_insert_command(command.substr(0, 1));
        }
      }
      break;

    case state_t::VISUAL_LINE:
      if (
        m_vi->get_stc()->SelectionIsRectangle() ||
        m_vi->get_stc()->GetSelectedText().empty())
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

  log::trace("vi mode") << str();

  m_vi->frame()->get_statusbar()->pane_show(
    "PaneMode",
    (!is_command() || ex::get_macros().mode().is_recording()) &&
      config(_("stc.Show mode")).get(true));
  m_vi->frame()->statustext(str(), "PaneMode");

  command.erase(0, 1);

  return true;
}

bool wex::vi_mode::transition_prep(const std::string& command)
{
  if (command.empty())
  {
    return false;
  }

  if (command[0] == 'c')
  {
    if (command.size() == 1)
    {
      return false;
    }

    if (command.size() == 2 && (command[1] == 'f' || command[1] == 'F'))
    {
      return false;
    }
  }

  return true;
}

void wex::vi_mode::visual()
{
  if (!is_visual())
  {
    std::string visual("v");
    transition(visual);
  }
}
