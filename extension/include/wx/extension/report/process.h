////////////////////////////////////////////////////////////////////////////////
// Name:      process.h
// Purpose:   Declaration of class wxExProcessListView
// Author:    Anton van Wezenbeek
// Copyright: (c) 2012 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#ifndef _EX_REPORT_PROCESS_H
#define _EX_REPORT_PROCESS_H

#include <wx/extension/process.h>

class wxExFrameWithHistory;
class wxExListView;

/// Offers a wxExProcess with reporting to a listview.
class WXDLLIMPEXP_BASE wxExProcessListView : public wxExProcess
{
public:
  /// Constructor.
  wxExProcessListView(wxExFrameWithHistory* frame);
  
  /// Sets report, this can be used if you do not override ReportCreate.
  void SetReport(wxExListView* report);
protected:
  /// Override from base class.
  /// Adds line as item on the report listview.
  virtual bool ReportAdd(
    const wxString& lineno) const;
    
  /// Override from base class.
  /// Calls Activate to activate on frame to select a report listview. 
  virtual bool ReportCreate();
private:
  wxExFrameWithHistory* m_Frame;
  wxExListView* m_ListView;
};
#endif
