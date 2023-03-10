////////////////////////////////////////////////////////////////////////////////
// Name:      stc/file.h
// Purpose:   Declaration of class wex::stc_file
// Author:    Anton van Wezenbeek
// Copyright: (c) 2021-2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#include <wex/core/file.h>

namespace wex
{
class ex_stream;
class stc;

/// Adds file read and write to stc.
class stc_file : public file
{
public:
  /// After loading / saving an event is sent to stc to update gui,
  /// this is the event number.
  enum
  {
    FILE_LOAD,      ///< file was loaded
    FILE_LOAD_SYNC, ///< file was reloaded
    FILE_SAVE,      ///< file was saved
    FILE_SAVE_AS,   ///< file was saved as
  };

  /// Constructor.
  stc_file(
    /// the stc component
    stc* stc,
    /// the path to be assigned
    const wex::path& p = wex::path());

  /// Destructor.
  ~stc_file() override;

  /// The ex stream (used if in ex mode), might be nullptr.
  class ex_stream*       ex_stream();
  const class ex_stream* ex_stream() const;

  bool is_contents_changed() const override;
  void reset_contents_changed() override;

private:
  bool do_file_load(bool synced = false) override;
  void do_file_new() override;
  void do_file_save(bool save_as = false) override;

  stc*           m_stc;
  std::streampos m_previous_size{0};
};
}; // namespace wex
