*** Comments ***
Copyright: (c) 2022-2023 Anton van Wezenbeek


*** Settings ***
Documentation       Testcases for wex vim

Resource            wex-keywords.resource

Suite Setup         Suite Setup
Suite Teardown      Suite Teardown
Test Setup          Test Setup


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
