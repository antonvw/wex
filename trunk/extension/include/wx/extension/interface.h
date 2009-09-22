/******************************************************************************\
* File:          interface.h
* Purpose:       Declaration of wxExInterface class
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _EXINTERFACE_H
#define _EXINTERFACE_H

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/fdrepdlg.h> // for wxFindReplaceDialog

// Only if we have a gui.
#if wxUSE_GUI

/// Offers a general find and printer interface.
class wxExInterface
{
public:
  /// Default constructor.
  wxExInterface()
    : m_FindReplaceDialog(NULL) {;};

  /// Destructor.
  virtual ~wxExInterface() {
    if (m_FindReplaceDialog != NULL) 
    {
      m_FindReplaceDialog->Destroy();
      m_FindReplaceDialog = NULL;
    };};

  /// Shows a find dialog.
  void FindDialog(
    wxWindow* parent, 
    const wxString& caption = _("Find"));

  /// Finds next (or previous) occurrence.
  /// Default returns false.
  virtual bool FindNext(
    const wxString& WXUNUSED(text), 
    bool WXUNUSED(find_next) = true) {
    return false;};

  /// Shows searching for in the statusbar, and calls FindNext.
  bool FindResult(const wxString& text, bool find_next, bool& recursive);

  /// Shows a replace dialog.
  void ReplaceDialog(
    wxWindow* parent, 
    const wxString& caption = _("Replace"));

  /// Allows you to update the find and replace data before Find or
  /// Replace Dialog is shown.
  virtual void SetFindReplaceData() {;};

  /// Build the page, for the htmleasyprinting.
  virtual const wxString BuildPage() {return wxEmptyString;};

  /// Invokes wxExApp PrintText with BuildPage.
  void Print();

  /// Adds a caption.
  virtual const wxString PrintCaption() const {return _("Printout");};

  /// You can use macros in PrintFooter and in PrintHeader:
  ///   \@PAGENUM\@ is replaced by page number
  ///   \@PAGESCNT\@ is replaced by total number of pages
  virtual const wxString PrintFooter() const
    {return _("Page @PAGENUM@ of @PAGESCNT@");};

  /// Adds a header.
  virtual const wxString PrintHeader() const
    {return _("Printed") + ": " + wxDateTime::Now().Format();};

  /// Invokes wxExApp PreviewText with BuildPage.
  void PrintPreview();
protected:
  void OnFindDialog(wxFindDialogEvent& event);
private:
  wxFindReplaceDialog* m_FindReplaceDialog;
};
#endif // wxUSE_GUI
#endif
