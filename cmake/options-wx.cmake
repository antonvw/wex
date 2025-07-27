# External options: turn off all we do not need 
# (regex is needed by jsscriptwrapper.h)

# option to change toolkit
# set(wxBUILD_TOOLKIT "gtk3")

set(wxBUILD_CXX_STANDARD 20 CACHE INTERNAL "WX_CXX_STANDARD")

option(wxBUILD_INSTALL "Create install/uninstall target for wxWidgets" OFF)

option(wxUSE_CMDLINE_PARSER "Use cmdline parser" OFF)
option(wxUSE_UNICODE_UTF8 "Use utf8" ON)
option(wxUSE_EXPAT "Use expat" OFF)
option(wxUSE_GLCANVAS "Use gl canvas" OFF)
option(wxUSE_LIBTIFF "Use libtiff" OFF)
option(wxUSE_MEDIACTRL "Use mediactrl" OFF)
option(wxUSE_PROPGRID "Use propgrid" OFF)
option(wxUSE_RIBBON "Use ribbon" OFF)
option(wxUSE_RICHTEXT "Use richtext" OFF)
set(wxUSE_REGEX builtin)

option(wxUSE_STD_STRING_CONV_IN_WXSTRING "Use std::string" ON)
option(wxUSE_UNICODE "Use Unicode" ON)
option(wxUSE_WEBVIEW "Use webview" OFF)
option(wxUSE_WEBVIEW_WEBKIT "Use webviewkit" OFF)
option(wxUSE_WEBVIEW_WEBKIT2 "Use webviewkit2" OFF)
option(wxUSE_XML "Use XML" OFF)
option(wxUSE_XRC "Use XRC" OFF)
