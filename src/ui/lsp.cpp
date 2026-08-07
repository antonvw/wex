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

completion_item_element::completion_item_element(std::string text)
  : insert_text(std::move(text))
{
}

completion_item_element::completion_item_element(const boost::json::object& obj)
  : insert_text(boost::algorithm::trim_copy(json_to_string(obj, "insertText")))
  , detail(json_to_string(obj, "detail"))
{
  if (obj.contains("kind"))
  {
    kind = obj.at("kind").as_int64();
  }

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

diagnostic_item::diagnostic_item(const range_item& r, std::string msg)
  : range(r)
  , message(std::move(msg))
{
}

diagnostic_item::diagnostic_item(const boost::json::object& obj)
  : range(obj)
  , code(json_to_string(obj, "code"))
  , message(json_to_string(obj, "message"))
  , source(json_to_string(obj, "source"))
  , severity(static_cast<wex::severity_t>(obj.at("severity").as_int64()))
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
  : range(rnge)
  , new_text(std::move(nw_text))
{
}

on_type_formatting_item::on_type_formatting_item(const boost::json::object& obj)
  : range(obj)
  , new_text(json_to_string(obj, "newText"))
{
}

std::stringstream on_type_formatting_item::log() const
{
  std::stringstream ss;

  ss << "new_text: " << new_text << range.log().str();

  return ss;
}

int on_type_formatting_item::replace_target(wxStyledTextCtrl* stc) const
{
  range.set_target(stc);
  const int old_target_size = stc->GetTargetText().size();
  return stc->ReplaceTarget(new_text) - old_target_size;
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

  auto ro = obj.at("range");

  start.line      = ro.at("start").at("line").as_int64();
  start.character = ro.at("start").at("character").as_int64();
  end.line        = ro.at("end").at("line").as_int64();
  end.character   = ro.at("end").at("character").as_int64();

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
  : type(show_message_item::INFO)
  , is_show(is_show_item)
  , message(json_to_string(obj, "message"))
{
  if (obj.contains("type"))
  {
    type = static_cast<show_message_item::message_t>(obj.at("type").as_int64());
  }
}
} // namespace wex