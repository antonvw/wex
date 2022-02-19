*** Settings ***
Documentation	Testcases for wex ex
Test Setup	Test Setup
Suite Setup	Suite Setup
Suite Teardown	Suite Teardown
Resource	wex-keywords.resource


*** Test Cases ***
empty
	Input	:1000
	...	:.=
	Appl
	Output Contains	1

edit
	Input	:e other.txt
	...	:f
	Appl
	# the :e command is handled by event, so other.txt not yet active
	Output Contains	${file-startup}

info
	Input	:a|line has text
	...	:f
	Appl
	Output Contains	${file-startup}
	Output Contains	1
	Output Contains	%
	Output Contains	level

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
	${size-copy}=	Get File Size	copy.txt
	Should Be Equal	${size}	${size-copy}
	Remove File	copy.txt

set
	Input	:set all *
	Appl
	Output Contains	ts=

set-bool
	Input	:set nosws *
	...	:set sws ? *
	Appl
	Output Contains	nosws

set-info
	Input	:set ts ? *
	Appl
	Output Contains	ts=

set-verbosity
	Input	:set ve?
	Appl
	Output Contains	ve=${level}

substitute
	Input	:a|line has text
	...	:a|line has a tiger
	...	:a|line has simon and simon and garfunkel
	...	:%s/simon/nick/
	Appl
	Output Contains	1
	Output Contains	simon

substitute-global
	Input	:a|line has text
	...	:a|line has a tiger
	...	:a|line has simon and simon and garfunkel
	...	:%s/simon/nick/g
	Appl
	Output Contains	2
	Output Contains	simon
	Contents Does Not Contain	simon


*** Comments ***
Copyright: (c) 2020-2022 Anton van Wezenbeek
