/******************************************************************************\
* File:          appl.h
* Purpose:       Declaration of class 'Application'
* Author:        Anton van Wezenbeek
* RCS-ID:        $Id$
*
* Copyright (c) 1998-2008, Anton van Wezenbeek
* All rights are reserved. Reproduction in whole or part is prohibited
* without the written consent of the copyright owner.
\******************************************************************************/

#ifndef _APPL_H
#define _APPL_H

#include <wx/wxprec.h>
#ifndef WX_PRECOMP
#include <wx/wx.h>
#endif
#include <wx/extension/extension.h>

/// Adds initialization to exApp.
class Application : public exApp
{
private:
  bool OnInit();
};

#endif
