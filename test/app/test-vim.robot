*** Comments ***
Copyright: (c) 2022-2025 Anton van Wezenbeek


*** Settings ***
Documentation       Testcases for wex vim

Resource            wex-keywords.resource

Suite Setup         Suite Setup
Suite Teardown      Suite Teardown
Test Setup          Test Setup


*** Test Cases ***
g-hash
	Input Many	:a|hash	10
	Input Many	:1	1
	Input	g#
	Appl
	Output Contains	hit top

g-lower
	Input Many	:a|UPPERCASE	10
	Input Many	:1	1
	Input	guw
	Appl
	Contents Contains	uppercase

g-star
	Input Many	:a|star	10
	Input Many	:?star?	1
	Input Many	g*	1
	Input	g*
	Appl
	Output Contains	hit bottom

g-toggle
	Input Many	:a|toGGle	10
	Input Many	:1	1
	Input	g~w
	Appl
	Contents Contains	TOggLE

g-upper
	Input Many	:a|lowercase	10
	Input Many	:1	1
	Input	gUw
	Appl
	Contents Contains	LOWERCASE
