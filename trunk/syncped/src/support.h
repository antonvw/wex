/******************************************************************************\
* File:          support.h
* Purpose:       Declaration of DecoratedFrame class
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2009, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _SUPPORT_H
#define _SUPPORT_H

#include <wx/extension/report/frame.h>

class DecoratedFrame : public wxExFrameWithHistory
{
public:
  DecoratedFrame();
  virtual bool AllowClose(wxWindowID id, wxWindow* page);
  virtual void OnNotebook(wxWindowID id, wxWindow* page);
};
#endif
