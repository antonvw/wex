# External options: turn off all we do not need 
# (regex is needed by jsscriptwrapper.h)

option(wxBUILD_INSTALL "Create install/uninstall target for wxWidgets" OFF)
option(wxBUILD_SHARED "Build wWidgets as shared library" OFF)
option(wxUSE_GLCANVAS "Use gl canvas" OFF)
option(wxUSE_PROPGRID "Use Propgrid" OFF)
option(wxUSE_REGEX "Use wx regex" sys)
option(wxUSE_RIBBON "Use Ribbon" OFF)
option(wxUSE_RICHTEXT "Use Richtext" OFF)
option(wxUSE_STL "Use STL" ON)
option(wxUSE_XML "Use XML" OFF)
option(wxUSE_XRC "Use XRC" OFF)
