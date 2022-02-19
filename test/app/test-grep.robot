*** Settings ***
Documentation	Testcases for wex grep
Test Setup	Test Setup
Suite Setup	Suite Setup
Suite Teardown	Suite Teardown
Resource	wex-keywords.resource


*** Test Cases ***
help
	Input	:grep -h
	Appl
	Output Contains	hidden
	Output Contains	recursive

text	[Documentation]	grep (without quit), and quit after some time
	${result}=	Run Process
	...	./count.sh
	Input Many	:grep rfw *.robot ./	1
	Appl	2000
	Sleep	1s
	Output Contains	Found 1 matches in ${result.stdout} file(s)


*** Comments ***
Copyright: (c) 2020-2022 Anton van Wezenbeek
