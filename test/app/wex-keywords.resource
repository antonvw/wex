*** Comments ***
Copyright: (c) 2020-2023 Anton van Wezenbeek


*** Settings ***
Documentation	Robot framework Keywords for testing wex
Library	OperatingSystem
Library	Process
Library	String


*** Variables ***
# Normally the appl exits after each test, override this
# variable on the commandline using '-v quit:0' to remain active
${quit}	1

# Normally the appl runs in quiet mode, only errors are logged,
# override this variable using '-v level:0' to run in verbose mode
# (only shown in the log file).
${level}	4

${APPL}	${EMPTY}

${file-contents}	contents.txt
${file-input}	input.txt
${file-output}	output.txt
${file-startup}	empty.txt
${file-stdout}	stdout.txt


*** Keywords ***
Suite Setup
	Find Appl
	Variable Should Exist	${APPL}
	${version}=	Evaluate	robot.__version__
	${result}=	Get Substring	${version}	0	1
	${result}=	Convert To Integer	${result}
	Should Be True	${result} >= 6

Suite Teardown
	IF	${level} > 0
		Remove File	${file-stdout}
		Remove File	${file-contents}
		Remove File	${file-input}
		Remove File	${file-output}
	END

Test Setup
	Create File	${file-input}
	Create File	${file-output}

Appl
	[Documentation]	Runs appl with suitable arguments
	[Arguments]	${quit}=0
	Run Process
	...	${APPL}
	...	-q	${quit}
	...	-s	${file-input}
	...	-V	${level}
	...	-X	${file-output}
	...	${file-startup}
	...	stdout=${file-stdout}

Contents Contains
	[Documentation]	Verifies that contents should contain text
	[Arguments]	${text}
	${result}=	Get File	${file-contents}
	Should Contain	${result}	${text}

Contents Does Not Contain
	[Documentation]	Verifies that contents does not contain text
	[Arguments]	${text}
	${result}=	Get File	${file-contents}
	Should Not Contain	${result}	${text}

Ex Mode
	[Documentation]	Runs appl in  ex mode
	[Arguments]	${quit}=0
	Run Process
	...	${APPL}
	...	-q	${quit}
	...	-s	${file-input}
	...	-V	${level}
	...	-X	${file-output}
	...	--ex
	...	${file-startup}
	...	stdout=${file-stdout}

Find Appl
	[Documentation]	Set the APPL variable to correct binary
	[Tags]	:private
	${result}=	Run Process
	...	find	../../
	...	-name	wex-sample
	...	-type	f
	Set Suite Variable	${APPL}	${result.stdout}

Input
	[Documentation]	Appends text to input
	[Arguments]	@{text}
	FOR	${cmd}	IN	@{text}
		Append To File	${file-input}	${cmd}
		Append To File	${file-input}	\n
	END

	Append To File	${file-input}	:w ${file-contents}\n

	IF	${quit} == 1	Append To File	${file-input}	:q!

Input Many
	[Arguments]	${line}	${count}
	[Documentation]	As Input, but extra argument for repeat count,
	...	and does not end write output and does not end with quit
	...	Normally, invoke this as often a you like, finish with
	...	one other Input keyword.
	FOR	${index}	IN RANGE	${count}
		Append To File	${file-input}	${line}
		Append To File	${file-input}	\n
	END

Input No Write
	[Documentation]	As Input, but does not write the file
	[Arguments]	@{text}
	FOR	${cmd}	IN	@{text}
		Append To File	${file-input}	${cmd}
		Append To File	${file-input}	\n
	END

	IF	${quit} == 1	Append To File	${file-input}	:q!

Output Contains
	[Documentation]	Verifies output contains text
	[Arguments]	${text}
	${result}=	Get File	${file-output}
	Should Contain	${result}	${text}

Output Does Not Contain
	[Documentation]	Verifies output does not contain text
	[Arguments]	${text}
	${result}=	Get File	${file-output}
	Should Not Contain	${result}	${text}

Std Output Contains
	[Documentation]	Verifies stdout does not contain text
	[Arguments]	${text}
	${result}=	Get File	${file-stdout}
	Should Contain	${result}	${text}
