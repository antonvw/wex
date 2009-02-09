/******************************************************************************\
* File:          statistics.h
* Purpose:       Include file for statistics classes
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EXSTATISTICS_H
#define _EXSTATISTICS_H

#include <wx/extension/grid.h>
#include <wx/extension/tool.h>

/// Offers base statistics. All statistics involve a key value pair,
/// where the key is a wxString, and the value a template.
template <class T> class exStatistics
{
public:
  /// Constructor.
  exStatistics() {
#if wxUSE_GRID
    m_Grid = NULL;
#endif
  };

  /// Adds other statistics.
  exStatistics& operator+=(const exStatistics& s) {
    for (
      typename std::map<wxString, T>::const_iterator it = s.m_Items.begin();
      it != s.m_Items.end();
      ++it)
    {
      Inc(it->first, it->second);
    }
    return *this;}

  /// Clears the items. If you have Shown the statistics
  /// the window is updated as well.
  void Clear() {
    m_Items.clear();

#if wxUSE_GRID
    m_Rows.clear();

    if (m_Grid != NULL)
    {
      m_Grid->DeleteCols(0, m_Grid->GetNumberCols());
      m_Grid->DeleteRows(0, m_Grid->GetNumberRows());
      m_Grid->CreateGrid(0, 0);
      m_Grid->AppendCols(2);
    }
#endif
  };

  /// Gets all items as a string. All items are returned as a string,
  /// with newlines separating items.
  const wxString Get() const {
    wxString text;
    for (
      typename std::map<wxString, T>::const_iterator it = m_Items.begin();
      it != m_Items.end();
      ++it)
    {
      // An cEOL gives incorrect result (CRCRLF)
      text << "\n" << it->first << " " << it->second;
    }
    return text;};

  /// Gets the items.
  const std::map<wxString, T> & GetItems() const {return m_Items;};

  /// Gets value for specified key.
  /// If the key is not in the items it is added.
  T Get(const wxString& key) {return m_Items[key];};

  /// Decrements key with value.
  T Dec(const wxString& key, T dec_value = 1) {
    return Set(key, Get(key) - dec_value);};

  /// Increments key with value.
  T Inc(const wxString& key, T inc_value = 1) {
    return Set(key, Get(key) + inc_value);};

  /// Sets key to value. If you have Shown the statistics
  /// the window is updated as well.
  T Set(const wxString& key, T value) {
    m_Items[key] = value;

#if wxUSE_GRID
    if (m_Grid != NULL)
    {
      std::map<wxString, int>::const_iterator it = m_Rows.find(key);

      if (it != m_Rows.end())
      {
        m_Grid->SetCellValue(it->second, 1, wxString::Format("%d", value));
      }
      else
      {
        m_Grid->AppendRows(1);
        const int row = m_Grid->GetNumberRows() - 1;
        m_Rows.insert(std::make_pair(key, row));

        m_Grid->SetCellValue(row, 0, key);
        m_Grid->SetCellValue(row, 1, wxString::Format("%d", value));

        m_Grid->AutoSizeColumn(0);
      }

      m_Grid->ForceRefresh();
    }
#endif

    return value;};

#if wxUSE_GRID
  /// Shows the statistics as a grid window on the parent,
  /// and specify whether to show row labels (i.e. row numbers)
  /// and col labels (Item, Value).
  /// Returns the window that is created, or is activated,
  /// if it already was created.
  exGrid* Show(wxWindow* parent, 
    bool hide_row_labels = true,
    bool hide_col_labels = true,
    wxWindowID id = wxID_ANY)
    {
    if (m_Grid == NULL)
    {
      m_Grid = new exGrid(parent, id);
      m_Grid->CreateGrid(0, 0);
      m_Grid->AppendCols(2);
      m_Grid->EnableEditing(false);
#if wxUSE_DRAG_AND_DROP
      m_Grid->UseDragAndDrop(false);
#endif

      if (hide_row_labels)
      {
        m_Grid->HideRowLabels();
      }

      m_Grid->SetColLabelValue(0, _("Item"));
      m_Grid->SetColLabelValue(1, _("Value"));

      if (hide_col_labels)
      {
        m_Grid->HideColLabels();
      }

      int i = 0;
      for (
        typename std::map<wxString, T>::const_iterator it = m_Items.begin();
        it != m_Items.end();
        ++it)
      {
       m_Grid->AppendRows(1);
       m_Grid->SetCellValue(i, 0, it->first);
       m_Grid->SetCellValue(i, 1, wxString::Format("%d", it->second));
       m_Rows[it->first] = i;
       i++;
      }
    }
    m_Grid->Show();
    return m_Grid;}
#endif
private:
  std::map<wxString, T> m_Items;
#if wxUSE_GRID
  std::map<wxString, int> m_Rows;
  exGrid* m_Grid;
#endif
};

/// Offers filename statistics. 
/// Adds element and keyword statistics to exFileName.
/// Used in exTextFile to keep statistics like comments and lines of code.
/// These are stored as elements.
class exFileNameStatistics : public exFileName
{
public:
  /// Constructor.
  exFileNameStatistics(
    const wxString& fullpath,
    wxPathFormat format = wxPATH_NATIVE)
    : exFileName(fullpath, format){}

  /// Constructor from an exFileName.
  exFileNameStatistics(const exFileName& filename)
    : exFileName(filename){}

  /// Adds other statistics.
  exFileNameStatistics& operator+=(const exFileNameStatistics& s) {
    m_Elements += s.m_Elements;
    m_Keywords += s.m_Keywords;
    return *this;}

  /// Gets all items as a string. All items are returned as a string,
  /// with newlines separating items.
  const wxString Get() const {
    return m_Elements.Get() + m_Keywords.Get();};

  /// Gets the key, first the elements are tried,
  /// if not present, the keywords are tried, if not present
  /// 0 is returned.
  long Get(const wxString& key) const;

  /// Gets the elements.
  exStatistics<long>& GetElements() {return m_Elements;};

  /// Gets the const elements.
  const exStatistics<long>& GetElements() const {return m_Elements;};

  /// Gets the keywords.
  exStatistics<long>& GetKeywords() {return m_Keywords;};

  /// Gets the const keywords.
  const exStatistics<long>& GetKeywords() const {return m_Keywords;};

  /// Returns the statistics log filename.
  static const exFileName GetLogfileName();

  /// For the last exTextFile::ToolRun logs the elements statistics to 
  /// the statusbar (always), to the statistics logfile (if specified),
  /// and open the statistics logfile (if specified) and the tool was a count type.
  void Log(
    const exTool& tool,
    bool log_to_file = true,
    bool open_file = true) const;
private:
  exStatistics<long> m_Elements;
  exStatistics<long> m_Keywords;
};
#endif
