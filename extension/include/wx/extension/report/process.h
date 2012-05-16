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
protected:
  /// Override from base class.
  virtual bool ReportAdd(
    const wxString& line, 
    const wxString& path,
    const wxString& lineno);
    
  /// Override from base class.
  virtual bool ReportCreate();
private:
  wxExFrameWithHistory* m_Frame;
  wxExListView* m_ListView;
};
#endif
