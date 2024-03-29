*** Comments ***
Copyright: (c) 2023 Anton van Wezenbeek


*** Settings ***
Documentation	Testcases for wex project
Test Setup	Test Setup
Suite Setup	Suite Setup
Suite Teardown	Suite Teardown
Resource	wex-keywords.resource


*** Test Cases ***
project
	Input	:f
	Project Mode
	Output Contains	${file-project}
