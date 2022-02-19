*** Settings ***
Documentation	Testcases for wex vim
Test Setup	Test Setup
Suite Setup	Suite Setup
Suite Teardown	Suite Teardown
Resource	wex-keywords.resource


*** Test Cases ***
g-upper
	Input Many	:a|lowercase	10
	Input Many	:1	1
	Input	gUw
	Appl
	Contents Contains	LOWERCASE

g-lower
	Input Many	:a|UPPERCASE	10
	Input Many	:1	1
	Input	guw
	Appl
	Contents Contains	uppercase

g-toggle
	Input Many	:a|toGGle	10
	Input Many	:1	1
	Input	g~w
	Appl
	Contents Contains	TOggLE

*** Comments ***
Copyright: (c) 2022 Anton van Wezenbeek
