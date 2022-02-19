////////////////////////////////////////////////////////////////////////////////
// Name:      data-to-stdin.h
// Purpose:   Implementation of class wex::data_to_std_in
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/factory/process-data.h>

namespace wex
{
/// Offers a small class to convert std_in from process data to
/// a FILE pointer.
class data_to_std_in
{
public:
  /// Constructor.
  /// If there is a std_in component in the process data, it will
  /// be converted to a FILE pointer accessible using std_in.
  /// Otherwise the FILE pointer points to stdin.
  data_to_std_in(const process_data& data);

  /// Destructor, if a file was opened, it is closed.
  ~data_to_std_in();

  /// Returns the file pointer.
  FILE* std_in();

private:
  const process_data& m_data;

  FILE* m_in{stdin};
  bool  m_opened{false};
};
}; // namespace wex
