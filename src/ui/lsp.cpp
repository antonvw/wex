////////////////////////////////////////////////////////////////////////////////
// Name:      lsp.cpp
// Purpose:   Implementation of classes related to Language Server Protocol
//            support in wex.
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>
#include <utility>

#include <wex/core/log.h>
#include <wex/ui/lsp.h>

namespace wex
{
std::string
json_to_string(const boost::json::value& val, const std::string& key)
{
  try
  {
    return val.is_object() && val.as_object().contains(key) ?
             val.at(key).as_string().c_str() :
             std::string();
  }
  catch (const std::exception& e)
  {
    log(e) << "wex::json_to_string" << key;
  }

  return std::string();
}

code_action_item::code_action_item(std::string t, std::string k)
  : title(std::move(t))
  , kind(std::move(k))
{
}

code_action_item::code_action_item(const boost::json::object& obj)
  : title(json_to_string(obj, "title"))
  , kind(json_to_string(obj, "kind"))
  , command(json_to_string(obj, "command"))
{
  for (const auto& item : obj.at("arguments").as_array())
  {
    edits.emplace_back(item.as_object());
  }
}

code_action_edit_change_item::code_action_edit_change_item(
  const range_item& rnge,
  std::string       nw_text)
  : range(rnge)
  , new_text(std::move(nw_text))
{
}

std::stringstream code_action_edit_change_item::log() const
{
  std::stringstream ss;

  ss << "new_text: " << new_text << range.log().str();

  return ss;
}

int code_action_edit_change_item::replace_target(wxStyledTextCtrl* stc) const
{
  range.set_target(stc);
  const int old_target_size = stc->GetTargetText().size();
  return stc->ReplaceTarget(new_text) - old_target_size;
}

code_action_edit_item::code_action_edit_item(const boost::json::object& obj)
{
  for (const auto& [url, edits_value] : obj.at("changes").as_object())
  {
    {
      const boost::json::array&                 edits = edits_value.as_array();
      std::vector<code_action_edit_change_item> ones;

      for (const boost::json::value& edit_value : edits)
      {
        const boost::json::object& edit = edit_value.as_object();
        ones.emplace_back(edit);
      }

      changes[url] = ones;
    }
  }
}

code_action_edit_change_item::code_action_edit_change_item(
  const boost::json::object& obj)
  : new_text(json_to_string(obj, "newText"))
  , range(obj)
{
}

completion_item::completion_item(
  const position_item&       p,
  const boost::json::object& obj)
  : pos(p)
{
  if (!obj.empty())
  {
    elements.reserve(obj.at("items").as_array().size());

    for (const auto& item : obj.at("items").as_array())
    {
      elements.emplace_back(item.as_object());
    }
  }
}

completion_item_element::completion_item_element(std::string text)
  : insert_text(std::move(text))
{
}

completion_item_element::completion_item_element(const boost::json::object& obj)
  : insert_text(boost::algorithm::trim_copy(json_to_string(obj, "insertText")))
  , detail(json_to_string(obj, "detail"))
  , kind(obj.contains("kind") ? obj.at("kind").as_int64() : 0)
{
  if (obj.contains("documentation"))
  {
    // The documentation is an array, not yet handled
    // documentation =
  }
}

definition_or_implementation_item::definition_or_implementation_item(
  std::string       u,
  const range_item& r)
  : uri(std::move(u))
  , range(r)
{
}

definition_or_implementation_item::definition_or_implementation_item(
  const boost::json::object& obj)
  : range(obj)
  , uri(obj.at("uri").as_string().data())
{
}

diagnostic_item::diagnostic_item(
  const range_item& r,
  std::string       msg,
  severity_t        s)
  : range(r)
  , message(std::move(msg))
  , severity(s)
{
}

diagnostic_item::diagnostic_item(const boost::json::object& obj)
  : range(obj)
  , code(json_to_string(obj, "code"))
  , message(json_to_string(obj, "message"))
  , source(json_to_string(obj, "source"))
  , severity(static_cast<wex::severity_t>(obj.at("severity").as_int64()))
  , is_fix_available(message.contains("fix available"))
{
}

hover_item::hover_item(const position_item& p, std::string c)
  : pos(p)
  , contents(std::move(c))
{
}

hover_item::hover_item(const boost::json::object& obj)
{
  const auto con(obj.at("contents"));
  const auto val(con.at("value").as_string());

  contents = boost::json::serialize(val);
  kind     = json_to_string(con, "kind");
}

on_type_formatting_item::on_type_formatting_item(
  const range_item& rnge,
  std::string       nw_text)
  : code_action_edit_change_item(rnge, std::move(nw_text))
{
}

on_type_formatting_item::on_type_formatting_item(const boost::json::object& obj)
  : code_action_edit_change_item(obj)
{
}

position_item::position_item(int l, int c)
  : line(l)
  , character(c)
{
}

position_item::position_item(wxStyledTextCtrl* stc)
  : line(stc->LineFromPosition(stc->GetCurrentPos()))
  , character(stc->GetCurrentPos() - stc->PositionFromLine(line))
{
}

position_item::position_item(const boost::json::object& obj)
  : line(obj.at("line").as_int64())
  , character(obj.at("character").as_int64())
{
}

boost::json::object position_item::json_object() const
{
  boost::json::object obj;

  obj["line"]      = line;
  obj["character"] = character;

  return obj;
}

std::stringstream position_item::log() const
{
  std::stringstream ss;

  ss << "line: " << line << " char: " << character;

  return ss;
}

int position_item::to_pos(wxStyledTextCtrl* stc) const
{
  return stc->PositionFromLine(line) + character;
}

range_item::range_item(const position_item& strt, const position_item& nd)
  : start(strt)
  , end(nd)
{
}

range_item::range_item(const boost::json::object& obj)
{
  set(obj);
}

boost::json::object range_item::json_object() const
{
  boost::json::object obj;

  obj["start"] = start.json_object();
  obj["end"]   = end.json_object();

  return obj;
}

bool range_item::set(const boost::json::object& obj)
{
  if (!obj.contains("range"))
  {
    return false;
  }

  const auto ro = obj.at("range");

  start = position_item(ro.at("start").as_object());
  end   = position_item(ro.at("end").as_object());

  return true;
}

std::stringstream range_item::log() const
{
  std::stringstream ss;

  ss << "start: " << start.log().str() << " end: " << end.log().str();

  return ss;
}

show_message_item::show_message_item(
  std::string msg,
  message_t   t,
  bool        is_show_item)
  : type(t)
  , message(std::move(msg))
  , is_show(is_show_item)
{
}

show_message_item::show_message_item(
  const boost::json::object& obj,
  bool                       is_show_item)
  : type(
      obj.contains("type") ?
        static_cast<show_message_item::message_t>(obj.at("type").as_int64()) :
        show_message_item::INFO)
  , is_show(is_show_item)
  , message(json_to_string(obj, "message"))
{
}
} // namespace wex
