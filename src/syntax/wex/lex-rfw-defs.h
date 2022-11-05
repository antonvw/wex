////////////////////////////////////////////////////////////////////////////////
// Name:      lex-rfw-defs.h
// Purpose:   Declaration of lex rfw defines
// Author:    Anton van Wezenbeek
// Copyright: (c) 2022 Anton van Wezenbeek
////////////////////////////////////////////////////////////////////////////////

#pragma once

#define RFW_CMD_BODY SCE_SH_HERE_Q + 1
#define RFW_CMD_START SCE_SH_HERE_Q + 2
#define RFW_CMD_WORD SCE_SH_HERE_Q + 3
#define RFW_CMD_TEST SCE_SH_HERE_Q + 4
#define RFW_CMD_ARITH SCE_SH_HERE_Q + 5
#define RFW_CMD_DELIM SCE_SH_HERE_Q + 6
#define RFW_CMD_SKW_PARTIAL SCE_SH_HERE_Q + 7
#define RFW_CMD_TESTCASE SCE_SH_HERE_Q + 8

// state constants for nested delimiter pairs, used by
// SCE_SH_STRING and SCE_SH_BACKTICKS processing
#define RFW_DELIM_LITERAL 0
#define RFW_DELIM_STRING 1
#define RFW_DELIM_CSTRING 2
#define RFW_DELIM_LSTRING 3
#define RFW_DELIM_COMMAND 4
#define RFW_DELIM_BACKTICK 5

#define RFW_BASE_ERROR 65
#define RFW_BASE_DECIMAL 66
#define RFW_BASE_HEX 67

// should be part of interface
#define SCE_SH_TESTCASE SCE_SH_HERE_DELIM // i.e. testcase in lexers.xml
#define SCE_SH_WORD2 SCE_SH_HERE_Q
