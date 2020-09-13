# Name:      test-syncped.robot
# Purpose:   Testcase file for testing syncped
# Author:    Anton van Wezenbeek
# Copyright: (c) 2020 Anton van Wezenbeek

*** Settings ***

Test Setup	Test Setup
Suite Setup	Suite Setup
Suite Teardown	Suite Teardown
Library	OperatingSystem
Library	Process

*** Variables ***

# Normally syncped exits after each test, override this
# variable on the commandline using '-v quit:0' to remain active
${quit}	1

${file-config}	test.json
${file-contents}	contents.txt
${file-input}	input.txt
${file-output}	output.txt
${file-startup}	empty.txt

${syncped}

*** Keywords ***

Suite Setup
	Find Syncped
	Variable Should Exist	${syncped}

Test Setup
	Create File	${file-input}
	Create File	${file-output}

Input
	[Arguments]	@{text}
	FOR	${cmd}	IN	@{text}
		Append To File	${file-input}	${cmd}
		Append To File	${file-input}	\n
	END
	
	Append To File	${file-input}	:w ${file-contents}\n
	Run Keyword If	${quit} == 1	Append To File	${file-input}	:q!

Input Many
	[Arguments]	${line}	${count}
	FOR    ${index}	IN RANGE	${count}
		Append To File	${file-input}	${line}
		Append To File	${file-input}	\n
	END
	
Find Syncped
	${result}=	Run Process	
	...	find	./
	...	-name	syncped
	...	-type	f
	Set Suite Variable	${syncped}	${result.stdout}

Syncped
	[Documentation]	Runs syncped with suitable arguments
	Run Process
	...	${syncped}
	...	-j 	${file-config}
	...	-s 	${file-input}
	...	-X 	${file-output}
	...	${file-startup}

Contents Contains
	[Arguments]	${text}
	${result} =	Get File	${file-contents}
	Should Contain	${result}	${text}

Contents Does Not Contain
	[Arguments]	${text}
	${result} =	Get File	${file-contents}
	Should Not Contain	${result}	${text}

Output Contains
	[Arguments]	${text}
	${result} =	Get File	${file-output}
	Should Contain	${result}	${text}

Output Does Not Contain
	[Arguments]	${text}
	${result} =	Get File	${file-output}
	Should Not Contain	${result}	${text}
	
Suite Teardown
	Remove File	${file-contents}
	Remove File	${file-input}
	Remove File	${file-output}

*** Test Cases ***

tc-help
	[Documentation]	Check whether we can startup correctly
	${result}=	Run Process	${syncped}	-h
	# required by OpenGroup
	Should Contain	${result.stdout}		-c
	Should Contain	${result.stdout}		-R
	Should Contain	${result.stdout}		-s
	Should Contain	${result.stdout}		-t
	# our own
	Should Contain	${result.stdout}		-j
	Should Contain	${result.stdout}		-X
	Should Contain	${result.stdout}		version

tc-empty
	Input	:1000
	...	:.=
	Syncped
	Output Contains	1
	
# ex tests

tc-ex-edit
	Input	:e other.txt
	...	:f
	Syncped
	# the :e command is handled by event, so other.txt not yet active
	Output Contains	${file-startup}

tc-ex-info
	Input	:a|line has text
	...	:f
	Syncped
	Output Contains	${file-startup}
	Output Contains	1
	Output Contains	%
	Output Contains	level

tc-ex-mdi
	Input	:a|line has text
	...	:e other.txt
	...	:e more.txt
	# We cannot test mdi, because of events.
	Syncped

tc-ex-process
	Input	:!pwd
	Syncped
	Output Contains	build

tc-ex-set
	Input	:set all *
	Syncped
	Output Contains	2

tc-ex-set-bool
	Input	:set nosws *
	...	:set sws ? *
	Syncped
	Output Contains	nosws
	
tc-ex-set-info
	Input	:set ts ? *
	Syncped
	Output Contains	2
	
tc-ex-substitute
	Input	:a|line has text
	...	:a|line has a tiger
	...	:a|line has simon and simon and garfunkel
	...	:%s/simon/nick/
	Syncped
	Output Contains	1
	Output Contains	simon

tc-ex-substitute-global
	Input	:a|line has text
	...	:a|line has a tiger
	...	:a|line has simon and simon and garfunkel
	...	:%s/simon/nick/g
	Syncped
	Output Contains	2
	Output Contains	simon
	Contents Does Not Contain	simon

# vi tests

tc-vi-delete
	Input Many	:a|line	100
	Input	:1
	...	59dd
	Syncped
	Output Contains	59
	Output Contains	fewer

tc-vi-delete-D
	Input	:a|line has some text
	...	:1
	...	ww
	...	D
	Syncped
	Contents Does Not Contain	some text

tc-vi-calculate
	Input	:a|x
	...	=9+9+9+9
	Syncped
	Output Contains	36
	
tc-vi-find-not
	Input	:a|x
	...	:a|y
	...	:a|z
	...	/zz
	...	:.=
	Syncped
	Output Contains	4

tc-vi-find-ok
	Input	:a|x
	...	:a|y
	...	:a|z
	...	/z
	...	:.=
	Syncped
	Output Contains	3

tc-vi-info
	Input	:a|line has text
	...	
	Syncped
	Output Contains	1
	Output Contains	%
	Output Contains	level

tc-vi-marker
	Input Many	:a|line has text	50
	Input 	:10
	...	mx
	...	:1
	...	'x
	...	:.=
	Syncped
	Output Contains	10

tc-vi-mode-block
	Input Many	:a|line has text	50
	Input 	:1
	...	w
	...	K
	...	10j
	...	w
	...	d
	Syncped
	Output Does Not Contain	10

tc-vi-mode-insert
	Input 	:a|one line
	...	ijjjjjjj
	Syncped
	Contents Contains	jjjjjjj

tc-vi-mode-visual
	Input Many	:a|line has text	50
	Input 	:1
	...	v
	...	10j
	...	d
	... 	:1
	...	v
	...	35j
	...	d
	Syncped
	Output Contains	10
	Output Contains	35
	Output Contains	fewer
	Output Does Not Contain	9

tc-vi-navigate
	Input Many	:a|line has text	50
	Input 	:1
	...	jjjjjjj
	...	:.=
	Syncped
	Output Contains	8

tc-vi-yank
	Input Many	:a|line	100
	Input	:1
	...	59yy
	Syncped
	Output Contains	59
	Output Contains	yanked
