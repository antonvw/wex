// Scintilla source code edit control
/** @file LexAda.cxx
 ** Lexer for Ada 95
 ** Converted to lexer object and added further folding features/properties by "Udo Lechner" <dlchnr(at)gmx(dot)net>
 ** Fixed warnings by Anton van Wezenbeek  
 **/
// Copyright 2002 by Sergey Koshcheyev <sergey.k@seznam.cz>
// The License.txt file describes the conditions under which this software may be distributed.

#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <stdarg.h>
#include <assert.h>
#include <ctype.h>

#ifdef _MSC_VER
#pragma warning(disable: 4786)
#endif

#include <string>
#include <map>

#include "ILexer.h"
#include "Scintilla.h"
#include "SciLexer.h"

#include "WordList.h"
#include "LexAccessor.h"
#include "StyleContext.h"
#include "CharacterSet.h"
#include "LexerModule.h"
#include "OptionSet.h"

#ifdef SCI_NAMESPACE
using namespace Scintilla;
#endif

enum tClassifiedKW {endKW=-1, unfitKW=0, nullKW=1, recordKW=2, ifKW, loopKW, caseKW, selectKW};

// An individual named option for use in an OptionSet

// Options used for LexerAda
struct OptionsAda { 
	bool fold {false};
	bool foldSyntaxBased {true};
	bool foldCommentMultiline {false};
	bool foldCommentExplicit {false};
	std::string foldExplicitStart {false};
	std::string foldExplicitEnd {false};
	bool foldExplicitAnywhere {false};
	bool foldCompact {true};
};

static const char * const adaWordListDesc[] = {
	"Keywords",
	0
};

struct OptionSetAda : public OptionSet<OptionsAda> {
	OptionSetAda() {

		DefineProperty("fold", &OptionsAda::fold);

		DefineProperty("fold.ada.syntax.based", &OptionsAda::foldSyntaxBased,
			"Set this property to 0 to disable syntax based folding.");

		DefineProperty("fold.ada.comment.multiline", &OptionsAda::foldCommentMultiline,
			"Set this property to 1 to enable folding successive single line comments, "
			"all starting on line's begin.");

		DefineProperty("fold.ada.comment.explicit", &OptionsAda::foldCommentExplicit,
			"This option enables folding explicit fold points when using the Ada lexer. "
			"Explicit fold points allows adding extra folding by placing a --{ comment at the start and a --} "
			"at the end of a section that should fold.");

		DefineProperty("fold.ada.explicit.start", &OptionsAda::foldExplicitStart,
			"The string to use for explicit fold start points, replacing the standard --{.");

		DefineProperty("fold.ada.explicit.end", &OptionsAda::foldExplicitEnd,
			"The string to use for explicit fold end points, replacing the standard --}.");

		DefineProperty("fold.ada.explicit.anywhere", &OptionsAda::foldExplicitAnywhere,
			"Set this property to 1 to enable explicit fold points anywhere, not just in line comments.");

		DefineProperty("fold.compact", &OptionsAda::foldCompact);
		DefineWordListSets(adaWordListDesc);
	}
};

/*
 * Interface
 */

class LexerAda : public ILexer {
	WordList keywords;
	OptionsAda options;
	OptionSetAda osAda;
public:
	LexerAda() {
	}
	~LexerAda() {
	}
	void SCI_METHOD Release() {
		delete this;
	}
	int SCI_METHOD Version() const {
		return lvOriginal;
	}
	const char * SCI_METHOD PropertyNames() {
		return osAda.PropertyNames();
	}
	int SCI_METHOD PropertyType(const char *name) {
		return osAda.PropertyType(name);
	}
	const char * SCI_METHOD DescribeProperty(const char *name) {
		return osAda.DescribeProperty(name);
	}
	int SCI_METHOD PropertySet(const char *key, const char *val);
	const char * SCI_METHOD DescribeWordListSets() {
		return osAda.DescribeWordListSets();
	}
	int SCI_METHOD WordListSet(int n, const char *wl);
	void SCI_METHOD Lex(unsigned int startPos, int length, int initStyle, IDocument *pAccess);
	void SCI_METHOD Fold(unsigned int startPos, int length, int initStyle, IDocument *pAccess);

	void * SCI_METHOD PrivateCall(int, void *) {
		return 0;
	}

	static ILexer *LexerFactoryAda() {
		return new LexerAda();
	}
};

LexerModule lmAda(SCLEX_ADA, LexerAda::LexerFactoryAda, "ada", adaWordListDesc);

/*
 * Implementation
 */

// colourise functions that have apostropheStartsAttribute as a parameter set it according to whether
// an apostrophe encountered after processing the current token will start an attribute or
// a character literal.
static void ColouriseCharacter(StyleContext& sc, bool& apostropheStartsAttribute);
static void ColouriseComment(StyleContext& sc, bool& apostropheStartsAttribute);
static void ColouriseContext(StyleContext& sc, char chEnd, int stateEOL);
static void ColouriseDelimiter(StyleContext& sc, bool& apostropheStartsAttribute);
static void ColouriseLabel(StyleContext& sc, WordList& keywords, bool& apostropheStartsAttribute);
static void ColouriseNumber(StyleContext& sc, bool& apostropheStartsAttribute);
static void ColouriseString(StyleContext& sc, bool& apostropheStartsAttribute);
static void ColouriseWhiteSpace(StyleContext& sc, bool& apostropheStartsAttribute);
static void ColouriseWord(StyleContext& sc, WordList& keywords, bool& apostropheStartsAttribute);

// other functions
static inline bool IsDelimiterCharacter(int ch);
static inline bool IsNumberStartCharacter(int ch);
static inline bool IsNumberCharacter(int ch);
static inline bool IsSeparatorOrDelimiterCharacter(int ch);
static bool IsValidIdentifier(const std::string& identifier);
static bool IsValidNumber(const std::string& number);
static inline bool IsWordStartCharacter(int ch);
static inline bool IsWordCharacter(int ch);
static tClassifiedKW classifyKeyword(char const *word);
static inline bool IsCompleteCommentLine(int line, LexAccessor &styler);
static inline int LowerCase(int c);

int SCI_METHOD LexerAda::PropertySet(const char *key, const char *val) {
	if (osAda.PropertySet(&options, key, val)) {
		return 0;
	}
	return -1;
}

int SCI_METHOD LexerAda::WordListSet(int n, const char *wl) {
	WordList *wordListN = 0;
	switch (n) {
	case 0:
		wordListN = &keywords;
		break;
	}
	int firstModification = -1;
	if (wordListN) {
		WordList wlNew;
		wlNew.Set(wl);
		if (*wordListN != wlNew) {
			wordListN->Set(wl);
			firstModification = 0;
		}
	}
	return firstModification;
}

//
// LexerAda::Lex
//

void SCI_METHOD LexerAda::Lex(unsigned int startPos, int length, int initStyle, IDocument *pAccess) {
	LexAccessor styler(pAccess);

	StyleContext sc(startPos, length, initStyle, styler);

	int lineCurrent = styler.GetLine(startPos);
	bool apostropheStartsAttribute = (styler.GetLineState(lineCurrent) & 1) != 0;

	while (sc.More()) {
		if (sc.atLineEnd) {
			// Go to the next line
			sc.Forward();
			lineCurrent++;

			// Remember the line state for future incremental lexing
			styler.SetLineState(lineCurrent, apostropheStartsAttribute);

			// Don't continue any styles on the next line
			sc.SetState(SCE_ADA_DEFAULT);
		}

		// Comments
		if (sc.Match('-', '-')) {
			ColouriseComment(sc, apostropheStartsAttribute);

		// Strings
		} else if (sc.Match('"')) {
			ColouriseString(sc, apostropheStartsAttribute);

		// Characters
		} else if (sc.Match('\'') && !apostropheStartsAttribute) {
			ColouriseCharacter(sc, apostropheStartsAttribute);

		// Labels
		} else if (sc.Match('<', '<')) {
			ColouriseLabel(sc, keywords, apostropheStartsAttribute);

		// Whitespace
		} else if (IsASpace(sc.ch)) {
			ColouriseWhiteSpace(sc, apostropheStartsAttribute);

		// Delimiters
		} else if (IsDelimiterCharacter(sc.ch)) {
			ColouriseDelimiter(sc, apostropheStartsAttribute);

		// Numbers
		} else if (IsADigit(sc.ch) || sc.ch == '#') {
			ColouriseNumber(sc, apostropheStartsAttribute);

		// Keywords or identifiers
		} else {
			ColouriseWord(sc, keywords, apostropheStartsAttribute);
		}
	}

	sc.Complete();
}

//
// LexerAda::Fold  -  folder is derived from LexCPP folder!
//

void SCI_METHOD LexerAda::Fold(unsigned int startPos, int length, int initStyle, IDocument* pAccess) {

	if (!options.fold)
		return;

	LexAccessor styler(pAccess);

	unsigned int endPos = startPos + length;
	int visibleChars = 0;
	int lineCurrent = styler.GetLine(startPos);
	int style = initStyle;

	// Backtrack to previous line in case need to fix its fold status
	if (startPos > 0) {
		if (lineCurrent > 0) {
			if (IsCompleteCommentLine(lineCurrent - 1, styler)) {
				lineCurrent--;
				startPos = styler.LineStart(lineCurrent);
				style = startPos > 0 ? styler.StyleAt(startPos - 1) : SCE_ADA_DEFAULT;
			}
		}
	}

	int levelCurrent = SC_FOLDLEVELBASE;
	if (lineCurrent > 0)
		levelCurrent = styler.LevelAt(lineCurrent-1) >> 16;
	int levelMinCurrent = levelCurrent;
	int levelNext = levelCurrent;
	char chNext = styler[startPos];
	int styleNext = styler.StyleAt(startPos);

	char keyword[20];
	int wordlen = 0;
	tClassifiedKW key = unfitKW, lastkey = unfitKW;
	const bool userDefinedFoldMarkers = !options.foldExplicitStart.empty() && !options.foldExplicitEnd.empty();
	for (unsigned int i = startPos; i < endPos; i++) {
		char ch = chNext;
		chNext = styler.SafeGetCharAt(i + 1);
		//int stylePrev = style;
		style = styleNext;
		styleNext = styler.StyleAt(i + 1);
		bool atEOL = (ch == '\r' && chNext != '\n') || (ch == '\n');
		if (options.foldCommentMultiline && atEOL && IsCompleteCommentLine(lineCurrent, styler)) {
			if (!IsCompleteCommentLine(lineCurrent - 1, styler)
			    && IsCompleteCommentLine(lineCurrent + 1, styler))
				levelNext++;
			else if (IsCompleteCommentLine(lineCurrent - 1, styler)
			         && !IsCompleteCommentLine(lineCurrent+1, styler))
				levelNext--;
		}
		if (options.foldCommentExplicit && ((style == SCE_ADA_COMMENTLINE) || options.foldExplicitAnywhere)) {
			if (userDefinedFoldMarkers) {
				if (styler.Match(i, options.foldExplicitStart.c_str())) {
					levelNext++;
				} else if (styler.Match(i, options.foldExplicitEnd.c_str())) {
					levelNext--;
				}
			} else {
				if ((ch == '-') && (chNext == '-')) {
					char chNext2 = styler.SafeGetCharAt(i + 2);
					if (chNext2 == '{') {
						levelNext++;
					} else if (chNext2 == '}') {
						levelNext--;
					}
				}
			}
		}
		if (options.foldSyntaxBased) {
			if (style == SCE_ADA_WORD) {
				keyword[wordlen++] = static_cast<char>(LowerCase(ch));
				if (wordlen == 20) {                   // prevent overflow
					keyword[0] = '\0';
					wordlen = 1;
				}
				if (styleNext != SCE_ADA_WORD) {       // reading identifier ready
					keyword[wordlen] = '\0';
					wordlen = 0;
					lastkey = key;
					key = classifyKeyword(keyword);
					if (key > nullKW) {
						if (!((lastkey == nullKW) && (key == recordKW))) {  // skip "record" after "null"
							if (lastkey == endKW) {
								levelNext--;
							} else  {
								levelNext++;
							}
						}
					}
				}
			} else if (style != SCE_ADA_DEFAULT) {
				key = unfitKW;
			}
		}
		if (!IsASpace(ch))
			visibleChars++;
		if (atEOL || (i == endPos-1)) {
			int levelUse = levelCurrent;
			int lev = levelUse | levelNext << 16;
			if (visibleChars == 0 && options.foldCompact)
				lev |= SC_FOLDLEVELWHITEFLAG;
			if (levelUse < levelNext)
				lev |= SC_FOLDLEVELHEADERFLAG;
			if (lev != styler.LevelAt(lineCurrent)) {
				styler.SetLevel(lineCurrent, lev);
			}
			lineCurrent++;
			levelCurrent = levelNext;
			levelMinCurrent = levelCurrent;
			if (atEOL && (i == static_cast<unsigned int>(styler.Length()-1))) {
				// There is an empty line at end of file so give it same level and empty
				styler.SetLevel(lineCurrent, (levelCurrent | levelCurrent << 16) | SC_FOLDLEVELWHITEFLAG);
			}
			visibleChars = 0;
		}
	}

}

// colourise functions

static void ColouriseCharacter(StyleContext& sc, bool& apostropheStartsAttribute) {
	apostropheStartsAttribute = true;

	sc.SetState(SCE_ADA_CHARACTER);

	// Skip the apostrophe and one more character (so that '' is shown as non-terminated and '''
	// is handled correctly)
	sc.Forward();
	sc.Forward();

	ColouriseContext(sc, '\'', SCE_ADA_CHARACTEREOL);
}

static void ColouriseContext(StyleContext& sc, char chEnd, int stateEOL) {
	while (!sc.atLineEnd && !sc.Match(chEnd)) {
		sc.Forward();
	}

	if (!sc.atLineEnd) {
		sc.ForwardSetState(SCE_ADA_DEFAULT);
	} else {
		sc.ChangeState(stateEOL);
	}
}

static void ColouriseComment(StyleContext& sc, bool& /*apostropheStartsAttribute*/) {
	// Apostrophe meaning is not changed, but the parameter is present for uniformity

	sc.SetState(SCE_ADA_COMMENTLINE);

	while (!sc.atLineEnd) {
		sc.Forward();
	}
}

static void ColouriseDelimiter(StyleContext& sc, bool& apostropheStartsAttribute) {
	apostropheStartsAttribute = sc.Match (')');
	sc.SetState(SCE_ADA_DELIMITER);
	sc.ForwardSetState(SCE_ADA_DEFAULT);
}

static void ColouriseLabel(StyleContext& sc, WordList& keywords, bool& apostropheStartsAttribute) {
	apostropheStartsAttribute = false;

	sc.SetState(SCE_ADA_LABEL);

	// Skip "<<"
	sc.Forward();
	sc.Forward();

	std::string identifier;

	while (!sc.atLineEnd && !IsSeparatorOrDelimiterCharacter(sc.ch)) {
		identifier += static_cast<char>(tolower(sc.ch));
		sc.Forward();
	}

	// Skip ">>"
	if (sc.Match('>', '>')) {
		sc.Forward();
		sc.Forward();
	} else {
		sc.ChangeState(SCE_ADA_ILLEGAL);
	}

	// If the name is an invalid identifier or a keyword, then make it invalid label
	if (!IsValidIdentifier(identifier) || keywords.InList(identifier.c_str())) {
		sc.ChangeState(SCE_ADA_ILLEGAL);
	}

	sc.SetState(SCE_ADA_DEFAULT);

}

static void ColouriseNumber(StyleContext& sc, bool& apostropheStartsAttribute) {
	apostropheStartsAttribute = true;

	std::string number;
	sc.SetState(SCE_ADA_NUMBER);

	// Get all characters up to a delimiter or a separator, including points, but excluding
	// double points (ranges).
	while (!IsSeparatorOrDelimiterCharacter(sc.ch) || (sc.ch == '.' && sc.chNext != '.')) {
		number += static_cast<char>(sc.ch);
		sc.Forward();
	}

	// Special case: exponent with sign
	if ((sc.chPrev == 'e' || sc.chPrev == 'E') &&
	        (sc.ch == '+' || sc.ch == '-')) {
		number += static_cast<char>(sc.ch);
		sc.Forward ();

		while (!IsSeparatorOrDelimiterCharacter(sc.ch)) {
			number += static_cast<char>(sc.ch);
			sc.Forward();
		}
	}

	if (!IsValidNumber(number)) {
		sc.ChangeState(SCE_ADA_ILLEGAL);
	}

	sc.SetState(SCE_ADA_DEFAULT);
}

static void ColouriseString(StyleContext& sc, bool& apostropheStartsAttribute) {
	apostropheStartsAttribute = true;

	sc.SetState(SCE_ADA_STRING);
	sc.Forward();

	ColouriseContext(sc, '"', SCE_ADA_STRINGEOL);
}

static void ColouriseWhiteSpace(StyleContext& sc, bool& /*apostropheStartsAttribute*/) {
	// Apostrophe meaning is not changed, but the parameter is present for uniformity
	sc.SetState(SCE_ADA_DEFAULT);
	sc.ForwardSetState(SCE_ADA_DEFAULT);
}

static void ColouriseWord(StyleContext& sc, WordList& keywords, bool& apostropheStartsAttribute) {
	apostropheStartsAttribute = true;
	sc.SetState(SCE_ADA_IDENTIFIER);

	std::string word;

	while (!sc.atLineEnd && !IsSeparatorOrDelimiterCharacter(sc.ch)) {
		word += static_cast<char>(tolower(sc.ch));
		sc.Forward();
	}

	if (!IsValidIdentifier(word)) {
		sc.ChangeState(SCE_ADA_ILLEGAL);

	} else if (keywords.InList(word.c_str())) {
		sc.ChangeState(SCE_ADA_WORD);

		if (word != "all") {
			apostropheStartsAttribute = false;
		}
	}

	sc.SetState(SCE_ADA_DEFAULT);
}

// other functions

static inline bool IsDelimiterCharacter(int ch) {
	switch (ch) {
	case '&':
	case '\'':
	case '(':
	case ')':
	case '*':
	case '+':
	case ',':
	case '-':
	case '.':
	case '/':
	case ':':
	case ';':
	case '<':
	case '=':
	case '>':
	case '|':
		return true;
	default:
		return false;
	}
}

static inline bool IsNumberCharacter(int ch) {
	return IsNumberStartCharacter(ch) ||
	       ch == '_' ||
	       ch == '.' ||
	       ch == '#' ||
	       (ch >= 'a' && ch <= 'f') ||
	       (ch >= 'A' && ch <= 'F');
}

static inline bool IsNumberStartCharacter(int ch) {
	return IsADigit(ch);
}

static inline bool IsSeparatorOrDelimiterCharacter(int ch) {
	return IsASpace(ch) || IsDelimiterCharacter(ch);
}

static bool IsValidIdentifier(const std::string& identifier) {
	// First character can't be '_', so initialize the flag to true
	bool lastWasUnderscore = true;

	size_t length = identifier.length();

	// Zero-length identifiers are not valid (these can occur inside labels)
	if (length == 0) {
		return false;
	}

	// Check for valid character at the start
	if (!IsWordStartCharacter(identifier[0])) {
		return false;
	}

	// Check for only valid characters and no double underscores
	for (size_t i = 0; i < length; i++) {
		if (!IsWordCharacter(identifier[i]) ||
		        (identifier[i] == '_' && lastWasUnderscore)) {
			return false;
		}
		lastWasUnderscore = identifier[i] == '_';
	}

	// Check for underscore at the end
	if (lastWasUnderscore == true) {
		return false;
	}

	// All checks passed
	return true;
}

static bool IsValidNumber(const std::string& number) {
	size_t hashPos = number.find("#");
	bool seenDot = false;

	size_t i = 0;
	size_t length = number.length();

	if (length == 0)
		return false; // Just in case

	// Decimal number
	if (hashPos == std::string::npos) {
		bool canBeSpecial = false;

		for (; i < length; i++) {
			if (number[i] == '_') {
				if (!canBeSpecial) {
					return false;
				}
				canBeSpecial = false;
			} else if (number[i] == '.') {
				if (!canBeSpecial || seenDot) {
					return false;
				}
				canBeSpecial = false;
				seenDot = true;
			} else if (IsADigit(number[i])) {
				canBeSpecial = true;
			} else {
				break;
			}
		}

		if (!canBeSpecial)
			return false;
	} else {
		// Based number
		bool canBeSpecial = false;
		int base = 0;

		// Parse base
		for (; i < length; i++) {
			int ch = number[i];
			if (ch == '_') {
				if (!canBeSpecial)
					return false;
				canBeSpecial = false;
			} else if (IsADigit(ch)) {
				base = base * 10 + (ch - '0');
				if (base > 16)
					return false;
				canBeSpecial = true;
			} else if (ch == '#' && canBeSpecial) {
				break;
			} else {
				return false;
			}
		}

		if (base < 2)
			return false;
		if (i == length)
			return false;

		i++; // Skip over '#'

		// Parse number
		canBeSpecial = false;

		for (; i < length; i++) {
			int ch = tolower(number[i]);

			if (ch == '_') {
				if (!canBeSpecial) {
					return false;
				}
				canBeSpecial = false;

			} else if (ch == '.') {
				if (!canBeSpecial || seenDot) {
					return false;
				}
				canBeSpecial = false;
				seenDot = true;

			} else if (IsADigit(ch)) {
				if (ch - '0' >= base) {
					return false;
				}
				canBeSpecial = true;

			} else if (ch >= 'a' && ch <= 'f') {
				if (ch - 'a' + 10 >= base) {
					return false;
				}
				canBeSpecial = true;

			} else if (ch == '#' && canBeSpecial) {
				break;

			} else {
				return false;
			}
		}

		if (i == length) {
			return false;
		}

		i++;
	}

	// Exponent (optional)
	if (i < length) {
		if (number[i] != 'e' && number[i] != 'E')
			return false;

		i++; // move past 'E'

		if (i == length) {
			return false;
		}

		if (number[i] == '+')
			i++;
		else if (number[i] == '-') {
			if (seenDot) {
				i++;
			} else {
				return false; // Integer literals should not have negative exponents
			}
		}

		if (i == length) {
			return false;
		}

		bool canBeSpecial = false;

		for (; i < length; i++) {
			if (number[i] == '_') {
				if (!canBeSpecial) {
					return false;
				}
				canBeSpecial = false;
			} else if (IsADigit(number[i])) {
				canBeSpecial = true;
			} else {
				return false;
			}
		}

		if (!canBeSpecial)
			return false;
	}

	// if i == length, number was parsed successfully.
	return i == length;
}

static inline bool IsWordCharacter(int ch) {
	return IsWordStartCharacter(ch) || IsADigit(ch);
}

static inline bool IsWordStartCharacter(int ch) {
	return (isascii(ch) && isalpha(ch)) || ch == '_';
}

static tClassifiedKW classifyKeyword(char const *keyword) {
tClassifiedKW key = unfitKW;

	// classifyKeyword will be fed only with detected keywords -
	// it's sufficient to check only some letters to classify them
	// http://www.adaic.org/resources/add_content/standards/05rm/html/RM-2-9.html
	if (keyword[0] < 'l') {
		if        (keyword[0] == 'e') {
			if ((keyword[1] == 'n') && (keyword[2] == 'd')) {
				key = endKW;
			}
		} else if (keyword[0] == 'i') {
			if (keyword[1] == 'f') {
				key =  ifKW;
			}
		} else if (keyword[0] == 'c') {
			if (keyword[1] == 'a') {
				key =  caseKW;
			}
		}
	} else {
		if        (keyword[0] == 'r') {
			if ((keyword[1] == 'e') && (keyword[2] == 'c')) {
				key =  recordKW;
			}
		} else if (keyword[0] == 'l') {
			if (keyword[1] == 'o') {
				key =  loopKW;
			}
		} else if (keyword[0] == 'n') {
			if (keyword[1] == 'u') {
				key =  nullKW;
			}
		} else if (keyword[0] == 's') {
			if ((keyword[1] == 'e') && (keyword[2] == 'l')) {
				key =  selectKW;
			}
		}
	}
	return key;
}

static inline bool IsCompleteCommentLine(int line, LexAccessor &styler) {
	int pos = styler.LineStart(line);
	char ch = styler[pos];
	char chNext = styler.SafeGetCharAt(pos + 1);
	int style = styler.StyleAt(pos);
	if (ch == '-' && chNext == '-' && style == SCE_ADA_COMMENTLINE) {
		return line >= 0;
	}
	return false;
}

static inline int LowerCase(int c) {
	if (c >= 'A' && c <= 'Z')
		return 'a' + c - 'A';
	return c;
}
