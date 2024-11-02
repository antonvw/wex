*** Comments ***
Copyright: (c) 2023 Anton van Wezenbeek


*** Settings ***
Documentation       Testcases for wex project

Resource            wex-keywords.resource

Suite Setup         Suite Setup
Suite Teardown      Suite Teardown
Test Setup          Test Setup


*** Test Cases ***
project
	Input	:f
	Project Mode
	Output Contains	${file-project}
