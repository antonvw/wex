*** Comments ***
Copyright: (c) 2020-2025 Anton van Wezenbeek


*** Settings ***
Documentation       Testcases for wex ex

Resource            wex-keywords.resource

Suite Setup         Suite Setup
Suite Teardown      Suite Teardown
Test Setup          Test Setup


*** Test Cases ***
addressing
	Input	:a|line has text
	...	:a|line has a tiger
	...	:a|line has a tiger
	...	:a|line has a tiger
	...	:a|line has a tiger
	...	:a|line has a tiger
	...	:a|line has a tiger
	...	:a|line has simon and simon and garfunkel
	...	:a|line has no match
	...	:/text/,/u/ya
	Appl
	Output Contains	15

edit
	Input	:e other.txt
	...	:f
	Appl
	# the :e command is handled by event, so other.txt not yet active
	Output Contains	${file-startup}

empty
	Input	:1000
	...	:.=
	Appl
	Output Contains	1

global
	Input Many	:a|line has text	20
	Input	:a|last line
	...	:g/has/
	Appl
	Output Contains	20 matches

info
	Input	:a|line has text
	...	:f
	Appl
	Output Contains	${file-startup}
	Output Contains	1
	Output Contains	%
	Output Contains	level

inverse
	Input Many	:a|line has text	20
	Input	:a|last line
	...	:g!/has/
	Appl
	Output Contains	1 matches

mdi
	Input	:a|line has text
	...	:e other.txt
	...	:e more.txt
	# We cannot test mdi, because of events.
	Appl

process
	Input	:!pwd
	Appl
	Output Contains	wex

saveas
	${size}=	Get File Size	test-ex.robot
	Input Many	:e test-ex.robot	1
	Input Many	:w copy.txt	1
	Appl	2000
	Sleep	1
	File Should Exist	copy.txt
	${size_copy}=	Get File Size	copy.txt
	Should Be Equal	${size}	${size_copy}
	Remove File	copy.txt

set
	Input	:set all
	Appl
	Output Contains	ec=

set-bool
	Input	:set nosws
	...	:set sws ?
	Appl
	Output Contains	nosws

set-info
	Input	:set ts ?
	Appl
	Output Contains	tabstop=

set-verbosity
	Input	:set ve?
	Appl
	Output Contains	ve=${level}

stream
	Input	:a|line has text
	Input	:a|line has text
	...	:f
	Ex Mode

substitute
	Input	:a|line has text
	...	:a|line has a tiger
	...	:a|line has simon and simon and garfunkel
	...	:%s/simon/nick/
	Appl
	Output Contains	1
	Output Contains	simon

substitute-eol
	Input	:a|line has text
	...	:a|line has a tiger
	...	:a|line has simon and simon and garfunkel
	...	:%s/$/EOL/
	Appl
	Contents Contains	textEOL
	Contents Contains	tigerEOL
	Contents Contains	garfunkelEOL

substitute-global
	Input	:a|line has text
	...	:a|line has a tiger
	...	:a|line has simon and simon and garfunkel
	...	:%s/simon/nick/g
	Appl
	Output Contains	2
	Output Contains	simon
	Contents Does Not Contain	simon

visual
	Input No Write	:vi
	Appl
