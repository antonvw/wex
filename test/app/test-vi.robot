*** Settings ***
Documentation	Testcases for wex vi
Test Setup	Test Setup
Suite Setup	Suite Setup
Suite Teardown	Suite Teardown
Resource	wex-keywords.resource


*** Test Cases ***
browse
	Input	:a|wxWidgets.org has some text
	...	:1
	...	ll
	...	U
	Appl

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
	Contents Does Not Contain	some text

find-not
	Input	:a|x
	...	:a|y
	...	:a|z
	...	/zz
	...	:.=
	Appl
	Output Contains	4

find-ok
	Input	:a|x
	...	:a|y
	...	:a|z
	...	/z
	...	:.=
	Appl
	Output Contains	3

info
	Input	:a|line has text
	...	
	Appl
	Output Contains	1
	Output Contains	%
	Output Contains	level

macro
	Input	@Template-test@
	Appl
	Contents Does Not Contain	@Created@
	Contents Does Not Contain	@Date@
	Contents Does Not Contain	'Date'
	Contents Does Not Contain	@Datetime@
	Contents Does Not Contain	@Process@
	Contents Does Not Contain	@Year@

macro-record
	Input	:a|10 1123
	...	:1
	...	qb
	...	w
	...	"x
	...	yw
	...	b
	...	cw
	...	=x+100
	...	
	...	0
	...	j
	...	q
	...	:1
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
	#Contents Contains	line other text\nline other text
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
	Input	:1
	...	v
	...	G
	...	y
	Appl
	Output Contains	10
	Output Contains	35
	Output Contains	fewer
	Output Does Not Contain	9

mode-visual-yank-range
	Input Many	:a|line has text	50
	Input	:1
	...	v
	...	G
	...	y
	Appl
	Output Contains	50
	Output Contains	yanked

navigate
	Input Many	:a|line has text	50
	Input	:1
	...	jjjjjjj
	...	:.=
	Appl
	Output Contains	8

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
	Output Contains	100
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


*** Comments ***
Copyright: (c) 2020-2022 Anton van Wezenbeek
