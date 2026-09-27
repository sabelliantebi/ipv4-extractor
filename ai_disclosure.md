# AI Disclosure

## 1. Tools I used

- **Claude** (claude.ai), model **Opus 5.5**. 
- **Code generation:** I sent my prompt and Claude wrote the full program (`main.cpp`).

- **Date:** September 27, 2026.

## 2. Prompts

### Prompt 1 (generate the code)

```
I need a C++ program that finds one IPv4 address inside a line of text.
Rules:
- A "token" is a group of characters next to each other that are only digits, '.' or ':'. Any other character is garbage and it separates tokens.
- Check every token completely. The token must be exactly A.B.C.D or A.B.C.D:PORT. If not, skip the whole token. Do not cut the token to find a valid part inside.
- Each octet: 1 to 3 digits, value 0-255, no leading zero (only "0" alone is ok).
- Port is optional: 1 to 5 digits, value 0-65535, same leading zero rule. If there is a colon, the port must be valid or the whole token is rejected.
- Return the first valid token in the line.
Function (must be exactly this):
bool extractIPv4(const std::string& str, unsigned long& outAddress, int& outPort);
On failure: outAddress = 0 and outPort = -1. If there is no port, outPort = -1.
Not allowed: atoi, stoi, strtol, sscanf, stringstream to convert numbers, inet_pton, inet_aton, regex. I must convert digits by hand (value = value*10 + (c - '0')). isdigit is ok.
main(): loop and ask "Enter a string (or 'END' to quit): ". If the input is exactly "END", print "Program terminated." and stop.
Output on success: Extracted IPv4 address: A.B.C.D (decimal value: N, port: P)
P is the number or the word none.
Output on failure: Invalid input: no valid IPv4 address found
Be careful with overflow: check the number of digits before you calculate the value.
```

Why I wrote it like this: I did not paste the handout. I tried to write the rules that are easy to get wrong: what a "token" is, no cutting the token to find a valid part, the leading zero rule, and overflow. I also listed the functions that are not allowed.

### Prompt 2 (follow-up after a failed test)

```
There is a bug in your code. For input "192.168.1.1" the program prints
"168.192.1.1", but the decimal value 3232235777 is correct. So the parsing
is fine and the problem is in main, where you convert the number back to
A.B.C.D. Please fix only this part and explain what was wrong.
```

## 3. What was AI-generated and what I did

| Part | Who |
|---|---|
| `main.cpp` (all functions: `isTokenChar`, `readNumber`, `checkToken`, `extractIPv4`, `main`) | AI generated. The final file is the exact AI version. |
| Prompts | Me, with help from Claude for the English |
| `tests.txt` (34 test inputs) | Me. I started from the handout sample run and added edge cases. Some ideas came from Claude. |
| `expected.txt` | Me. I checked every line by hand before I saved it. |
| Testing, finding the bug, checking the original code | Me |
| Line-by-line comments in `main.cpp` | Me |

**Modifications to the AI code:** I did not change any code logic. I added a comment to every line to explain what it does, to show I understand the code. After adding the comments I ran all tests again and the output is the same as `expected.txt`. See section 4 for the bug that happened when I copied the code.

## 4. Problem I found and how I fixed it

**Problem:** When I ran the tests the first time, the program printed the first two octets in the wrong order. For `192.168.1.1` it printed `168.192.1.1`. For `1.2.3.4` it printed `2.1.3.4`. The decimal value was correct (3232235777), so the parsing was fine. The bug was only in the printing part.

**How I found it:** I ran all my tests with `./ip < tests.txt > output.txt` and compared the output with the sample run. See `evidence/output_before_fix.txt`.

Something I learned here: `0.0.0.0`, `255.255.255.255` and `8.8.8.8` looked correct, but only because the first two octets are the same number. If I only tested with those, I would think the code was perfect. So tests need different values in every octet.

**What I did:**
1. I looked at `main` and found the lines. My file had `>> 16` first and `>> 24` second. The correct order is `>> 24`, `>> 16`, `>> 8`, then no shift.
2. I sent Prompt 2 to the AI. The AI said its code was already correct and my copy was different.
3. I did not just believe it. I went back to the original AI answer and checked. The AI was right: in its code the order was correct. In my file two lines were swapped. I think I moved a line by mistake when I pasted the code (in VSCode, Option + arrow moves a line). My file also had 163 lines and the AI file had 165, so it was not an exact copy.
4. **Fix:** I replaced my whole `main.cpp` with the exact AI version, so no other difference stays hidden. Then I compiled and ran all the tests again. All 34 tests pass.

Evidence: `EVIDENCE/repo_bug.png` (my file with the bug), `EVIDENCE/ai_original.png` (the AI original code), `EVIDENCE/output_before_fix.txt`, `EVIDENCE/first_output.txt`.

**Lesson:** When a test fails, I need to check my own copy too, not only the AI code. And when the AI says "my code is correct", I also need to check it myself.

## 5. Other checks I did

- I searched the code for forbidden functions with:
  `grep -nE "atoi|atol|stoi|stol|stoul|strtol|strtoul|strtod|sscanf|stringstream|inet_|regex" main.cpp`
  Nothing was found.
- I tested numbers that are too big (`99999999999.1.1.1`, port `9999999999999`). The program does not crash because it counts the digits before it calculates the value.
- I tested all the cases from the rules: 5 octets, 3 octets, empty octet (`1..2.3`), leading zero in octet and port (`01.2.3.4`, `1.2.3.04`, `1.2.3.4:080`, `1.2.3.4:00`), empty port (`1.2.3.4:`), two colons, port 65535 (ok) and 65536 (invalid), empty line, and `end` in lowercase (does not quit).

## 6. Decisions I made 

- **Two valid addresses in one line:** the program returns the first one. Example: `1.2.3.4 and 5.6.7.8` gives `1.2.3.4`.
- **`ip:10.0.0.1` is rejected.** The `:` is part of the token, so the token is `:10.0.0.1`. The rules say a stray colon next to the address makes it invalid. A person would say the address is there, but I followed the rules.
- **`-1.2.3.4` gives `1.2.3.4`,** because `-` is not a digit, `.` or `:`, so it is garbage.

## 7. How the code works 

- `extractIPv4` goes through the line. It skips garbage characters. When it finds a digit, `.` or `:`, it takes the whole group (the token) and sends it to `checkToken`.
- `checkToken` reads 4 numbers with `readNumber`, with a `.` between them. After the 4th number the token must end, or have `:` and a valid port that ends the token. If anything is wrong, the whole token is rejected and the search continues with the next token.
- `readNumber` counts the digits first (max 3 for an octet, 5 for a port), checks the leading zero, and after that converts by hand with `value = value * 10 + (c - '0')`.
- The 32-bit value is built with `result = result * 256 + octet`. In `main` I get each octet back with `>> 24`, `>> 16`, `>> 8` and `& 255`.

## 8. Verification statement

I understand every line of the submitted code. I did not copy it blindly: I tested it, found a problem, and checked where the problem came from. The code has been tested with the 34 cases in `tests.txt`, and the output is the same as `expected.txt`.

**Known limitations:**
- If the input ends without `END` (for example Ctrl+D), the program stops without printing `Program terminated.`
- If a line comes with a Windows line ending (`END\r`), it will not match `END`. On Mac this is not a problem.