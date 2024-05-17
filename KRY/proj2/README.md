**Author**: Adam Zvara<br>
**ID**: 232242<br>
**Login**: xzvara01   <br>
**Date**: 04/2024     <br>

# How to run

Compile the binary:
```
make
```

The result binary is stored in projects root directory as `kry`. The program implements:
1. generating SHA256 hash of given input (`-c`) - prints out hash(input) to stdout
```bash
$ echo -ne "zprava" | ./kry -c
d8305a064cd0f827df85ae5a7732bf25d578b746b8434871704e98cde3208ddf
```

2. generating MAC from password passed in `k` parameter (`-s`) - prints out hash(password || input) to stdout
```bash
$ echo -ne "zprava" | ./kry -s -k heslo
23158796a45a9392951d9a72dffd6a539b14a07832390b937b94a80ddb6dc18e
```

3. validate MAC passed in `-m`, which should correspond to MAC of input and password in `-k` (`-v`) -
if the MAC is correct, program returns 0, otherwise 1
```bash
$ echo -ne "zprava" | ./kry -v -k heslo -m 23158796a45a9392951d9a72dffd6a539b14a07832390b937b94a80ddb6dc18e ; echo $?
0
$ echo -ne "zprava" | ./kry -v -k heslo -m INVALID_MAC ; echo $?
1
```

4. length extension attack on known secret length in `-n`, known MAC in `-m` and known input, which will append
new message in `-a` to the original message (`-e`) - prints out MAC of modified message and the message itself (with padding)
```bash
$ echo -ne "zprava" | ./kry -e -n 5 -a ==message -m 23158796a45a9392951d9a72dffd6a539b14a07832390b937b94a80ddb6dc18e
a3b205a7ebb070c26910e1028322e99b35e846d5db399aae295082ddecf3edd3
zprava\x80\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x00\x58==message
```

The project also comes with a set of tests in `_my_tests.py` (testing with `unittest` library), which were used during the implementation.
Before running the tests, make sure you have the `length-extension-tool` library installed:
```
pip install length-extension-tool
```

After installing the library, you can run the tests from the root project directory (**make sure that the binary is compiled in the root project directory**):
```
python3 -m unittest _my_tests
```

# Implementation details

Program structure is very simple. First, command line arguments are parsed with `getopt` and stored in internal `prog_args` structure. Since the program is written in C,
the handling of input (either the actual input from user or message extension in length extension attack) is realized with `input` structure, which is inflatable buffer
with various functions (like reading input, inserting characters/sequences at the beginning/end ...). The message is read character by character and loaded into the `input` structure.
After the whole message has been read, the action specified in `prog_args` determines the function called next. The `checksum` functions implements calculating SHA256 hash, which
is printed to stdout. The `genmac` functions uses the `checksum` function, but appends key to the input. The `valmac` function simply calculates MAC and compares it with the MAC
provided in `-m` argument. The `extattack` function is a little bit more complicated .. it first converts message to be appended from command line argument into `input` structure,
and it passes it into sha256, with the initial hash values from the provided hash. After that, it uses the original message (with padding) to print out the modified message.
