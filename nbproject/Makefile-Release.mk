#
# Generated Makefile - do not edit!
#
# Edit the Makefile in the project folder instead (../Makefile). Each target
# has a -pre and a -post target defined where you can add customized code.
#
# This makefile implements configuration specific macros and targets.


# Environment
MKDIR=mkdir
CP=cp
CCADMIN=CCadmin
RANLIB=ranlib
CC=gcc
CCC=
CXX=
FC=

# Macros
PLATFORM=GNU-Solaris-Sparc

# Include project Makefile
include Makefile

# Object Directory
OBJECTDIR=build/Release/${PLATFORM}

# Object Files
OBJECTFILES= \
	${OBJECTDIR}/trunk/extension/src/dir.o \
	${OBJECTDIR}/trunk/extension/src/app.o \
	${OBJECTDIR}/trunk/extension/test/base/test.o \
	${OBJECTDIR}/trunk/syncped/src/app.o \
	${OBJECTDIR}/trunk/extension/src/dialog.o \
	${OBJECTDIR}/trunk/extension/src/textfile.o \
	${OBJECTDIR}/trunk/extension/src/shell.o \
	${OBJECTDIR}/trunk/extension/src/file.o \
	${OBJECTDIR}/trunk/extension/src/lexer.o \
	${OBJECTDIR}/trunk/extension/src/statistics.o \
	${OBJECTDIR}/trunk/extension/src/configitem.o \
	${OBJECTDIR}/trunk/extension/src/tool.o \
	${OBJECTDIR}/trunk/extension/test/report/test.o \
	${OBJECTDIR}/trunk/extension/src/otl.o \
	${OBJECTDIR}/trunk/extension/src/notebook.o \
	${OBJECTDIR}/trunk/extension/sample/report/app.o \
	${OBJECTDIR}/trunk/extension/src/lexers.o \
	${OBJECTDIR}/trunk/extension/test/app/main.o \
	${OBJECTDIR}/trunk/extension/src/art.o \
	${OBJECTDIR}/trunk/syncped/src/support.o \
	${OBJECTDIR}/trunk/extension/src/frame.o \
	${OBJECTDIR}/trunk/extension/src/report/textfile.o \
	${OBJECTDIR}/trunk/extension/src/renderer.o \
	${OBJECTDIR}/trunk/extension/src/listview.o \
	${OBJECTDIR}/trunk/extension/src/stcdlg.o \
	${OBJECTDIR}/trunk/extension/src/log.o \
	${OBJECTDIR}/trunk/extension/src/util.o \
	${OBJECTDIR}/trunk/extension/src/vi.o \
	${OBJECTDIR}/trunk/extension/src/report/stc.o \
	${OBJECTDIR}/trunk/extension/src/report/listview.o \
	${OBJECTDIR}/trunk/extension/test/report/main.o \
	${OBJECTDIR}/trunk/extension/src/grid.o \
	${OBJECTDIR}/trunk/extension/src/report/dir.o \
	${OBJECTDIR}/trunk/syncsocketserver/src/app.o \
	${OBJECTDIR}/trunk/extension/src/configdlg.o \
	${OBJECTDIR}/trunk/syncodbcquery/src/app.o \
	${OBJECTDIR}/trunk/extension/test/base/main.o \
	${OBJECTDIR}/trunk/extension/src/report/util.o \
	${OBJECTDIR}/trunk/extension/src/frd.o \
	${OBJECTDIR}/trunk/extension/src/filedlg.o \
	${OBJECTDIR}/trunk/extension/src/report/listitem.o \
	${OBJECTDIR}/trunk/extension/src/header.o \
	${OBJECTDIR}/trunk/extension/src/stc.o \
	${OBJECTDIR}/trunk/extension/sample/app.o \
	${OBJECTDIR}/trunk/extension/src/printing.o \
	${OBJECTDIR}/trunk/syncped/src/frame.o \
	${OBJECTDIR}/trunk/extension/src/report/process.o \
	${OBJECTDIR}/trunk/extension/src/svn.o \
	${OBJECTDIR}/trunk/extension/test/app/test.o \
	${OBJECTDIR}/trunk/extension/src/report/frame.o \
	${OBJECTDIR}/trunk/extension/src/menu.o

# C Compiler Flags
CFLAGS=

# CC Compiler Flags
CCFLAGS=
CXXFLAGS=

# Fortran Compiler Flags
FFLAGS=

# Link Libraries and Options
LDLIBSOPTIONS=

# Build Targets
.build-conf: ${BUILD_SUBPROJECTS}
	${MAKE}  -f nbproject/Makefile-Release.mk dist/Release/${PLATFORM}/wxextension

dist/Release/${PLATFORM}/wxextension: ${OBJECTFILES}
	${MKDIR} -p dist/Release/${PLATFORM}
	${LINK.cc} -o dist/Release/${PLATFORM}/wxextension ${OBJECTFILES} ${LDLIBSOPTIONS} 

${OBJECTDIR}/trunk/extension/src/dir.o: trunk/extension/src/dir.cpp 
	${MKDIR} -p ${OBJECTDIR}/trunk/extension/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/trunk/extension/src/dir.o trunk/extension/src/dir.cpp

${OBJECTDIR}/trunk/extension/src/app.o: trunk/extension/src/app.cpp 
	${MKDIR} -p ${OBJECTDIR}/trunk/extension/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/trunk/extension/src/app.o trunk/extension/src/app.cpp

${OBJECTDIR}/trunk/extension/test/base/test.o: trunk/extension/test/base/test.cpp 
	${MKDIR} -p ${OBJECTDIR}/trunk/extension/test/base
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/trunk/extension/test/base/test.o trunk/extension/test/base/test.cpp

${OBJECTDIR}/trunk/syncped/src/app.o: trunk/syncped/src/app.cpp 
	${MKDIR} -p ${OBJECTDIR}/trunk/syncped/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/trunk/syncped/src/app.o trunk/syncped/src/app.cpp

${OBJECTDIR}/trunk/extension/src/dialog.o: trunk/extension/src/dialog.cpp 
	${MKDIR} -p ${OBJECTDIR}/trunk/extension/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/trunk/extension/src/dialog.o trunk/extension/src/dialog.cpp

${OBJECTDIR}/trunk/extension/src/textfile.o: trunk/extension/src/textfile.cpp 
	${MKDIR} -p ${OBJECTDIR}/trunk/extension/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/trunk/extension/src/textfile.o trunk/extension/src/textfile.cpp

${OBJECTDIR}/trunk/extension/src/shell.o: trunk/extension/src/shell.cpp 
	${MKDIR} -p ${OBJECTDIR}/trunk/extension/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/trunk/extension/src/shell.o trunk/extension/src/shell.cpp

${OBJECTDIR}/trunk/extension/src/file.o: trunk/extension/src/file.cpp 
	${MKDIR} -p ${OBJECTDIR}/trunk/extension/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/trunk/extension/src/file.o trunk/extension/src/file.cpp

${OBJECTDIR}/trunk/extension/src/lexer.o: trunk/extension/src/lexer.cpp 
	${MKDIR} -p ${OBJECTDIR}/trunk/extension/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/trunk/extension/src/lexer.o trunk/extension/src/lexer.cpp

${OBJECTDIR}/trunk/extension/src/statistics.o: trunk/extension/src/statistics.cpp 
	${MKDIR} -p ${OBJECTDIR}/trunk/extension/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/trunk/extension/src/statistics.o trunk/extension/src/statistics.cpp

${OBJECTDIR}/trunk/extension/src/configitem.o: trunk/extension/src/configitem.cpp 
	${MKDIR} -p ${OBJECTDIR}/trunk/extension/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/trunk/extension/src/configitem.o trunk/extension/src/configitem.cpp

${OBJECTDIR}/trunk/extension/src/tool.o: trunk/extension/src/tool.cpp 
	${MKDIR} -p ${OBJECTDIR}/trunk/extension/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/trunk/extension/src/tool.o trunk/extension/src/tool.cpp

${OBJECTDIR}/trunk/extension/test/report/test.o: trunk/extension/test/report/test.cpp 
	${MKDIR} -p ${OBJECTDIR}/trunk/extension/test/report
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/trunk/extension/test/report/test.o trunk/extension/test/report/test.cpp

${OBJECTDIR}/trunk/extension/src/otl.o: trunk/extension/src/otl.cpp 
	${MKDIR} -p ${OBJECTDIR}/trunk/extension/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/trunk/extension/src/otl.o trunk/extension/src/otl.cpp

${OBJECTDIR}/trunk/extension/src/notebook.o: trunk/extension/src/notebook.cpp 
	${MKDIR} -p ${OBJECTDIR}/trunk/extension/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/trunk/extension/src/notebook.o trunk/extension/src/notebook.cpp

${OBJECTDIR}/trunk/extension/sample/report/app.o: trunk/extension/sample/report/app.cpp 
	${MKDIR} -p ${OBJECTDIR}/trunk/extension/sample/report
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/trunk/extension/sample/report/app.o trunk/extension/sample/report/app.cpp

${OBJECTDIR}/trunk/extension/src/lexers.o: trunk/extension/src/lexers.cpp 
	${MKDIR} -p ${OBJECTDIR}/trunk/extension/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/trunk/extension/src/lexers.o trunk/extension/src/lexers.cpp

${OBJECTDIR}/trunk/extension/test/app/main.o: trunk/extension/test/app/main.cpp 
	${MKDIR} -p ${OBJECTDIR}/trunk/extension/test/app
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/trunk/extension/test/app/main.o trunk/extension/test/app/main.cpp

${OBJECTDIR}/trunk/extension/src/art.o: trunk/extension/src/art.cpp 
	${MKDIR} -p ${OBJECTDIR}/trunk/extension/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/trunk/extension/src/art.o trunk/extension/src/art.cpp

${OBJECTDIR}/trunk/syncped/src/support.o: trunk/syncped/src/support.cpp 
	${MKDIR} -p ${OBJECTDIR}/trunk/syncped/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/trunk/syncped/src/support.o trunk/syncped/src/support.cpp

${OBJECTDIR}/trunk/extension/src/frame.o: trunk/extension/src/frame.cpp 
	${MKDIR} -p ${OBJECTDIR}/trunk/extension/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/trunk/extension/src/frame.o trunk/extension/src/frame.cpp

${OBJECTDIR}/trunk/extension/src/report/textfile.o: trunk/extension/src/report/textfile.cpp 
	${MKDIR} -p ${OBJECTDIR}/trunk/extension/src/report
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/trunk/extension/src/report/textfile.o trunk/extension/src/report/textfile.cpp

${OBJECTDIR}/trunk/extension/src/renderer.o: trunk/extension/src/renderer.cpp 
	${MKDIR} -p ${OBJECTDIR}/trunk/extension/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/trunk/extension/src/renderer.o trunk/extension/src/renderer.cpp

${OBJECTDIR}/trunk/extension/src/listview.o: trunk/extension/src/listview.cpp 
	${MKDIR} -p ${OBJECTDIR}/trunk/extension/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/trunk/extension/src/listview.o trunk/extension/src/listview.cpp

${OBJECTDIR}/trunk/extension/src/stcdlg.o: trunk/extension/src/stcdlg.cpp 
	${MKDIR} -p ${OBJECTDIR}/trunk/extension/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/trunk/extension/src/stcdlg.o trunk/extension/src/stcdlg.cpp

${OBJECTDIR}/trunk/extension/src/log.o: trunk/extension/src/log.cpp 
	${MKDIR} -p ${OBJECTDIR}/trunk/extension/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/trunk/extension/src/log.o trunk/extension/src/log.cpp

${OBJECTDIR}/trunk/extension/src/util.o: trunk/extension/src/util.cpp 
	${MKDIR} -p ${OBJECTDIR}/trunk/extension/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/trunk/extension/src/util.o trunk/extension/src/util.cpp

${OBJECTDIR}/trunk/extension/src/vi.o: trunk/extension/src/vi.cpp 
	${MKDIR} -p ${OBJECTDIR}/trunk/extension/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/trunk/extension/src/vi.o trunk/extension/src/vi.cpp

${OBJECTDIR}/trunk/extension/src/report/stc.o: trunk/extension/src/report/stc.cpp 
	${MKDIR} -p ${OBJECTDIR}/trunk/extension/src/report
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/trunk/extension/src/report/stc.o trunk/extension/src/report/stc.cpp

${OBJECTDIR}/trunk/extension/src/report/listview.o: trunk/extension/src/report/listview.cpp 
	${MKDIR} -p ${OBJECTDIR}/trunk/extension/src/report
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/trunk/extension/src/report/listview.o trunk/extension/src/report/listview.cpp

${OBJECTDIR}/trunk/extension/test/report/main.o: trunk/extension/test/report/main.cpp 
	${MKDIR} -p ${OBJECTDIR}/trunk/extension/test/report
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/trunk/extension/test/report/main.o trunk/extension/test/report/main.cpp

${OBJECTDIR}/trunk/extension/src/grid.o: trunk/extension/src/grid.cpp 
	${MKDIR} -p ${OBJECTDIR}/trunk/extension/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/trunk/extension/src/grid.o trunk/extension/src/grid.cpp

${OBJECTDIR}/trunk/extension/src/report/dir.o: trunk/extension/src/report/dir.cpp 
	${MKDIR} -p ${OBJECTDIR}/trunk/extension/src/report
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/trunk/extension/src/report/dir.o trunk/extension/src/report/dir.cpp

${OBJECTDIR}/trunk/syncsocketserver/src/app.o: trunk/syncsocketserver/src/app.cpp 
	${MKDIR} -p ${OBJECTDIR}/trunk/syncsocketserver/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/trunk/syncsocketserver/src/app.o trunk/syncsocketserver/src/app.cpp

${OBJECTDIR}/trunk/extension/src/configdlg.o: trunk/extension/src/configdlg.cpp 
	${MKDIR} -p ${OBJECTDIR}/trunk/extension/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/trunk/extension/src/configdlg.o trunk/extension/src/configdlg.cpp

${OBJECTDIR}/trunk/syncodbcquery/src/app.o: trunk/syncodbcquery/src/app.cpp 
	${MKDIR} -p ${OBJECTDIR}/trunk/syncodbcquery/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/trunk/syncodbcquery/src/app.o trunk/syncodbcquery/src/app.cpp

${OBJECTDIR}/trunk/extension/test/base/main.o: trunk/extension/test/base/main.cpp 
	${MKDIR} -p ${OBJECTDIR}/trunk/extension/test/base
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/trunk/extension/test/base/main.o trunk/extension/test/base/main.cpp

${OBJECTDIR}/trunk/extension/src/report/util.o: trunk/extension/src/report/util.cpp 
	${MKDIR} -p ${OBJECTDIR}/trunk/extension/src/report
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/trunk/extension/src/report/util.o trunk/extension/src/report/util.cpp

${OBJECTDIR}/trunk/extension/src/frd.o: trunk/extension/src/frd.cpp 
	${MKDIR} -p ${OBJECTDIR}/trunk/extension/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/trunk/extension/src/frd.o trunk/extension/src/frd.cpp

${OBJECTDIR}/trunk/extension/src/filedlg.o: trunk/extension/src/filedlg.cpp 
	${MKDIR} -p ${OBJECTDIR}/trunk/extension/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/trunk/extension/src/filedlg.o trunk/extension/src/filedlg.cpp

${OBJECTDIR}/trunk/extension/src/report/listitem.o: trunk/extension/src/report/listitem.cpp 
	${MKDIR} -p ${OBJECTDIR}/trunk/extension/src/report
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/trunk/extension/src/report/listitem.o trunk/extension/src/report/listitem.cpp

${OBJECTDIR}/trunk/extension/src/header.o: trunk/extension/src/header.cpp 
	${MKDIR} -p ${OBJECTDIR}/trunk/extension/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/trunk/extension/src/header.o trunk/extension/src/header.cpp

${OBJECTDIR}/trunk/extension/src/stc.o: trunk/extension/src/stc.cpp 
	${MKDIR} -p ${OBJECTDIR}/trunk/extension/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/trunk/extension/src/stc.o trunk/extension/src/stc.cpp

${OBJECTDIR}/trunk/extension/sample/app.o: trunk/extension/sample/app.cpp 
	${MKDIR} -p ${OBJECTDIR}/trunk/extension/sample
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/trunk/extension/sample/app.o trunk/extension/sample/app.cpp

${OBJECTDIR}/trunk/extension/src/printing.o: trunk/extension/src/printing.cpp 
	${MKDIR} -p ${OBJECTDIR}/trunk/extension/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/trunk/extension/src/printing.o trunk/extension/src/printing.cpp

${OBJECTDIR}/trunk/syncped/src/frame.o: trunk/syncped/src/frame.cpp 
	${MKDIR} -p ${OBJECTDIR}/trunk/syncped/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/trunk/syncped/src/frame.o trunk/syncped/src/frame.cpp

${OBJECTDIR}/trunk/extension/src/report/process.o: trunk/extension/src/report/process.cpp 
	${MKDIR} -p ${OBJECTDIR}/trunk/extension/src/report
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/trunk/extension/src/report/process.o trunk/extension/src/report/process.cpp

${OBJECTDIR}/trunk/extension/src/svn.o: trunk/extension/src/svn.cpp 
	${MKDIR} -p ${OBJECTDIR}/trunk/extension/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/trunk/extension/src/svn.o trunk/extension/src/svn.cpp

${OBJECTDIR}/trunk/extension/test/app/test.o: trunk/extension/test/app/test.cpp 
	${MKDIR} -p ${OBJECTDIR}/trunk/extension/test/app
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/trunk/extension/test/app/test.o trunk/extension/test/app/test.cpp

${OBJECTDIR}/trunk/extension/src/report/frame.o: trunk/extension/src/report/frame.cpp 
	${MKDIR} -p ${OBJECTDIR}/trunk/extension/src/report
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/trunk/extension/src/report/frame.o trunk/extension/src/report/frame.cpp

${OBJECTDIR}/trunk/extension/src/menu.o: trunk/extension/src/menu.cpp 
	${MKDIR} -p ${OBJECTDIR}/trunk/extension/src
	${RM} $@.d
	$(COMPILE.cc) -O2 -MMD -MP -MF $@.d -o ${OBJECTDIR}/trunk/extension/src/menu.o trunk/extension/src/menu.cpp

# Subprojects
.build-subprojects:

# Clean Targets
.clean-conf:
	${RM} -r build/Release
	${RM} dist/Release/${PLATFORM}/wxextension

# Subprojects
.clean-subprojects:

# Enable dependency checking
.dep.inc: .depcheck-impl

include .dep.inc
