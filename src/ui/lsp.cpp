////////////////////////////////////////////////////////////////////////////////
// Name:      lsp.cpp
// Purpose:   Implementation of classes related to Language Server Protocol
//            support in wex.
// Author:    Anton van Wezenbeek
// Copyright: (c) 2026 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#include <boost/algorithm/string.hpp>

#include <wex/core/log.h>
#include <wex/ui/lsp.h>

namespace wex
{
bool json_to_string(
  const boost::json::value& val,
  const std::string&        key,
  std::string&              dest)
{
  if (val.is_object() && val.as_object().contains(key))
  {
    dest = val.at(key).as_string().c_str();
    return true;
  }

  return false;
}

bool range_from_json(const boost::json::object& obj, range_item& range)
{
  auto ro = obj.at("range");

  range.start.line      = ro.at("start").at("line").as_int64();
  range.start.character = ro.at("start").at("character").as_int64();
  range.end.line        = ro.at("end").at("line").as_int64();
  range.end.character   = ro.at("end").at("character").as_int64();

  return true;
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
  const boost::json::object& obj)
{
  range_from_json(obj, range);
  uri = obj.at("uri").as_string().data();
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