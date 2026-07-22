////////////////////////////////////////////////////////////////////////////////
// Name:      lsp.cpp
// Purpose:   Implementation of classes related to Language Server Protocol
//            support in wex.
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>

#include <wex/ui/lsp.h>

namespace wex
{
bool json_to_string(
  const boost::json::value& val,
  const std::string&        key,
  std::string&              text)
{
  if (!val.is_object() || !val.as_object().contains(key))
  {
    return false;
  }

  text = val.at(key).as_string().c_str();

  return true;
}

bool range_from_json(const boost::json::object& obj, range_item& range)
{
  if (!obj.contains("range"))
  {
    return false;
  }

  auto ro = obj.at("range");

  range.start.line      = ro.at("start").at("line").as_int64();
  range.start.character = ro.at("start").at("character").as_int64();
  range.end.line        = ro.at("end").at("line").as_int64();
  range.end.character   = ro.at("end").at("character").as_int64();

  return true;
}

completion_item_element::completion_item_element(const std::string& lbl)
  : label(lbl)
{
}

completion_item_element::completion_item_element(const boost::json::object& obj)
{
  if (json_to_string(obj, "label", label))
  {
    boost::algorithm::trim(label);
  }

  if (obj.contains("kind"))
  {
    kind = obj.at("kind").as_int64();
  }

  json_to_string(obj, "detail", detail);

  if (obj.contains("documentation"))
  {
    // The documentation is an array, not yet handled
    // documentation =
  }
}

definition_or_implementation_item::definition_or_implementation_item(
  const std::string& u,
  const range_item&  r)
  : uri(u)
  , range(r)
{
}

definition_or_implementation_item::definition_or_implementation_item(
  const boost::json::object& obj)
{
  range_from_json(obj, range);
  uri = obj.at("uri").as_string().data();
}

diagnostic_item::diagnostic_item(const range_item& r, const std::string& msg)
  : range(r)
  , message(msg)
{
}

diagnostic_item::diagnostic_item(const boost::json::object& obj)
{
  range_from_json(obj, range);
  severity = static_cast<wex::severity_t>(obj.at("severity").as_int64());

  json_to_string(obj, "code", code);
  json_to_string(obj, "message", message);
  json_to_string(obj, "source", source);
}

hover_item::hover_item(const position_item& p, const std::string& c)
  : pos(p)
  , contents(c)
{
}

hover_item::hover_item(const boost::json::object& obj)
{
  const auto con(obj.at("contents"));
  const auto val(con.at("value").as_string());

  contents = boost::json::serialize(val);
  json_to_string(con, "kind", kind);
}

on_type_formatting_item::on_type_formatting_item(
  const range_item&  rnge,
  const std::string& nw_text)
  : range(rnge)
  , new_text(nw_text)
{
}

on_type_formatting_item::on_type_formatting_item(const boost::json::object& obj)
{
  range_from_json(obj, range);
  json_to_string(obj, "newText", new_text);
}

std::stringstream on_type_formatting_item::log() const
{
  std::stringstream ss;

  ss << "new_text: " << new_text << range.log().str();

  return ss;
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

boost::json::object range_item::json_object() const
{
  boost::json::object obj;

  obj["start"] = start.json_object();
  obj["end"]   = end.json_object();

  return obj;
}

std::stringstream range_item::log() const
{
  std::stringstream ss;

  ss << "start: " << start.log().str() << " end: " << end.log().str();

  return ss;
}

show_message_item::show_message_item(
  const std::string& msg,
  message_t          t,
  bool               is_show_item)
  : type(t)
  , message(msg)
  , is_show(is_show_item)
{
}

show_message_item::show_message_item(
  const boost::json::object& obj,
  bool                       is_show_item)
  : type(show_message_item::INFO)
  , is_show(is_show_item)
{
  json_to_string(obj, "message", message);

  if (obj.contains("type"))
  {
    type = static_cast<show_message_item::message_t>(obj.at("type").as_int64());
  }
}
} // namespace wex