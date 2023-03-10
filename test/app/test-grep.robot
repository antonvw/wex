*** Comments ***
Copyright: (c) 2020-2023 Anton van Wezenbeek


*** Settings ***
Documentation	Testcases for wex grep (and set)
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

grep	[Documentation]	grep (without quit), and quit after some time
	${result}=	Run Process
	...	./count.sh
	Input Many	:grep rfw *.robot ./	1
	Appl	2000
	Sleep	1s
	Output Contains	Found 1 matches in ${result.stdout} file(s)

sed	[Documentation]	sed (without quit), and quit after some time
	${result}=	Run Process
	...	./count.sh
	Input Many	:sed XXX XXX *.robot ./	1
	Appl	2000
	Sleep	1s
	Output Contains	Replaced 2 matches in ${result.stdout} file(s)
