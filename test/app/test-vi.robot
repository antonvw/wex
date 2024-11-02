*** Comments ***
Copyright: (c) 2020-2024 Anton van Wezenbeek


*** Settings ***
Documentation       Testcases for wex vi

Library             DateTime
Resource            wex-keywords.resource

Suite Setup         Suite Setup
Suite Teardown      Suite Teardown
Test Setup          Test Setup


*** Test Cases ***
browse
	Input	:a|wxWidgets.org has some text
	...	:1
	...	ll
	...	:set ve=0
	...	U
	...	:set ve=${level}
	Appl
	Std Output Contains	duckduckgo

calculate
	Input	=9+9+9+9-2+(3*3)
	...	=2<4
	...	=128/2
	...	=127&7
	Appl
	Output Contains	43
	Output Contains	32
	Output Contains	64
	Output Contains	7

delete
	Input Many	:a|line	100
	Input	:1
	...	59dd
	Appl
	Output Contains	59
	Output Contains	fewer

delete-d
	Input	:a|line has some text
	...	:1
	...	ww
	...	D
	Appl
	Contents Contains	line has
	Contents Does Not Contain	some text

escape
	Input Many	:a|line has text	50
	Input	:1
	...	v
	...	10j
	...	:!wc -l
	...	
	Appl
	Contents Contains	5

find-not
	Input	:a|x
	...	:a|y
	...	:a|z
	...	/zz
	...	:.=
	Appl
	Output Contains	7

find-ok
	Input	:a|x
	...	:a|y
	...	:a|z
	...	/z
	...	:.=
	Appl
	Output Contains	5

info
	Input	:a|line has text
	...	
	Appl
	Output Contains	1
	Output Contains	%
	Output Contains	level

macro
	${date}=	Get Current Date	result_format=%Y-%m-%d
	Input	@Template-test@
	Appl
	Contents Does Not Contain	@Created@
	Contents Does Not Contain	@Date@
	Contents Does Not Contain	'Date'
	Contents Does Not Contain	@Datetime@
	Contents Does Not Contain	@Process@
	Contents Does Not Contain	@Year@
	Contents Contains	date:${SPACE*5}${date}
	Contents Contains	fullname:${SPACE}${file-startup}

macro-record
	Input	:a|10 1123
	...	:a|10 1500
	...	:1
	...	qb
	...	w
	...	"x
	...	yw
	...	b
	...	ce=x + 100
	...	
	...	0
	...	j
	...	q
	...	:2
	...	@b
	Appl
	Contents Contains	1223

marker
	Input Many	:a|line has text	50
	Input	:10
	...	mx
	...	:1
	...	'x
	...	:.=
	Appl
	Output Contains	10

mode-block
	Input Many	:a|line has text	50
	Input	:1
	...	w
	...	K
	...	10j
	...	w
	...	d
	Appl
	Output Contains	11

mode-block-ce
	Input Many	:a|line has text	50
	Input	:1
	...	w
	...	K
	...	10j
	...	ce
	...	other
	...	
	...	
	Appl
	# stc does not offer rect insert (needs upgrade)
	# Contents Contains	line other text\nline other text
	Contents Contains	line other text

mode-insert
	Input	:a|one line
	...	ijjjjjjj
	Appl
	Contents Contains	jjjjjjj

mode-visual
	Input Many	:a|line has text	50
	Input	:1
	...	v
	...	10j
	...	d
	...	:1
	...	v
	...	35j
	...	d
	...	:1
	...	v
	...	G
	...	y
	Appl
	Output Contains	10
	Output Contains	35
	Output Contains	fewer

mode-visual-yank-range
	Input Many	:a|line has text	50
	Input	:1
	...	v
	...	G
	...	y
	Appl
	Output Contains	100
	Output Contains	yanked

navigate
	Input Many	:a|line has text	50
	Input	:1
	...	jjjjjjj
	...	:.=
	Appl
	Output Contains	8

rfw
	Input	:a|* Test Cases *
	Input Many	:a|testcase	1000
	Input	:set sy=rfw
	Appl

substitute-eol
	Input	:a|line has text
	...	:a|line has a tiger
	...	:a|line has simon and simon and garfunkel
	...	:%s/t.*e$/TIGER/g
	...	:%s/g.*unkel$/NICK/g
	Appl
	Contents Does Not Contain	TIGER
	Contents Does Not Contain	gar
	Contents Contains	NICK
	Contents Contains	tiger

substitute-undo
	Input	:a|line has text
	...	:a|line has a tiger
	...	:a|line has simon and simon and garfunkel
	...	:%s/simon/nick/g
	...	u
	Appl
	Output Contains	2
	Output Contains	simon

undo
	Input Many	:a|line	10
	Input	:1
	...	u
	Appl
	Contents Does Not Contain	line

yank
	Input Many	:a|line	100
	Input	:1
	...	59yy
	Appl
	Output Contains	59
	Output Contains	yanked

yank-range
	Input Many	:a|line	100
	Input	:1
	...	yG
	Appl
	Output Contains	200
	Output Contains	yanked

yank-register
	Input Many	:a|line	10
	Input	:1
	...	"x
	...	yw
	...	:%d
	...	i
	...	xx
	...	
	Appl
	Contents Contains	lineline
