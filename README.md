# IPv4 Extractor (C++)

This program reads a line of text and finds one valid IPv4 address, with an optional port.
It was made with AI help. See ai_disclosure.md for the full process.

## How to run
g++ -std=c++17 -Wall -o ip main.cpp
./ip

## How to run all tests
./ip < tests.txt > output.txt
diff output.txt expected.txt
(no output from diff = all tests pass)

## Files
- main.cpp: the program
- tests.txt: 34 test inputs
- expected.txt: expected output for each test
- ai_disclosure.md: how I used AI, the prompts, the bug I found and the fix
- EVIDENCE/: screenshots and the output before the fix
