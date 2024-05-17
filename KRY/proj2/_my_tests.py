import unittest
import string
import HashTools
import os
from subprocess import run
from hashlib import sha256
from random import SystemRandom

# Get path to binary
binary = os.getcwd().split("/")[-1]
if binary == "test":
    binary = "../kry"
else:
    binary = "./kry"

# Check, if binary exists
if not os.path.exists(binary):
    print("Binary not found, please compile it first.")
    exit(1)

class TestOverall(unittest.TestCase):
    # Some parts to identify error messages and check them, edit to match your output
    usage = "Usage: kry"
    err_noaction = "Error: No action specified"
    err_mulactions = "Error: Multiple actions specified"
    err_incorrectparams = "requires"

    def _run_program(self, input, args):
        return run(binary + ' ' + args, input=input, capture_output=True, text=True, shell=True)

    def _run_success(self, input = "", args = ""):
        x = self._run_program(input, args)
        self.assertEqual(x.returncode, 0)
        return x.stdout

    def _generate_input(self, length):
        return ''.join(SystemRandom().choice(string.ascii_uppercase + string.digits) for _ in range(length))

    def _generate_key(self, length=32):
        key_length = SystemRandom().randint(1, length)
        return ''.join(SystemRandom().choice(string.ascii_uppercase + string.digits) for _ in range(key_length))

    def _run_fail(self, ret_msg, args):
        x = self._run_program("", args)
        self.assertEqual(x.returncode, 1)
        self.assertEqual(True, ret_msg in x.stderr)

    def msg_bytes2str(self, msg, append_len, input_len):
        new_msg = []
        for i in range(0, input_len):
            new_msg.append(chr(msg[i]))
        for j in range(input_len, len(msg) - append_len):
            new_msg.append("\\x{0:02x}".format(msg[j]))
        for k in range(len(msg) - append_len, len(msg)):
            new_msg.append(chr(msg[k]))
        return "".join(new_msg)

    def test_arg_parsing(self):
        # usage
        x = self._run_program("", "")
        self.assertEqual(x.returncode, 1)
        self.assertEqual(True, self.usage in x.stdout)

        # no action specified fail
        self._run_fail(self.err_noaction, "test")

        # multiple actions fail
        self._run_fail(self.err_mulactions, "-c -s")
        self._run_fail(self.err_mulactions, "-s -v -e")
        self._run_fail(self.err_mulactions, "-s -s")

        # incorect parameters
        self._run_fail(self.err_incorrectparams, "-s")
        self._run_fail(self.err_incorrectparams, "-s -k")
        self._run_fail(self.err_incorrectparams, "-v")
        self._run_fail(self.err_incorrectparams, "-v -k abc")
        self._run_fail(self.err_incorrectparams, "-e")
        self._run_fail(self.err_incorrectparams, "-e -m abc")
        self._run_fail(self.err_incorrectparams, "-e -m abc -n 1")
        self._run_fail(self.err_incorrectparams, "-e -m abc -n abc -a abc")

    def test_checksum(self):
        N = 2000
        for i in range(1, N):
            input = self._generate_input(i)
            hash = self._run_success(input, "-c")
            exp_hash = sha256(input.encode()).hexdigest()
            self.assertEqual(exp_hash, hash.strip())

    def test_checksum_long(self):
        input = self._generate_input(100000)
        hash = self._run_success(input, "-c")
        exp_hash = sha256(input.encode()).hexdigest()
        self.assertEqual(exp_hash, hash.strip())

    def test_checksum_longlong(self):
        input = self._generate_input(10000000)
        hash = self._run_success(input, "-c")
        exp_hash = sha256(input.encode()).hexdigest()
        self.assertEqual(exp_hash, hash.strip())

    def test_genmac(self):
        N = 2000
        for i in range(1, N):
            input = self._generate_input(i)
            key = self._generate_key()
            mac = self._run_success(input, "-s -k " + key)
            exp_mac = sha256((key + input).encode()).hexdigest()
            self.assertEqual(exp_mac, mac.strip())

    def test_genmac_long(self):
        input = self._generate_input(100000)
        key = self._generate_key()
        mac = self._run_success(input, "-s -k " + key)
        exp_mac = sha256((key + input).encode()).hexdigest()
        self.assertEqual(exp_mac, mac.strip())

    def test_verifymac(self):
        N = 2000
        for i in range(1, N):
            input = self._generate_input(i)
            key = self._generate_key()
            exp_mac = sha256((key + input).encode()).hexdigest()
            self._run_success(input, "-v -k " + key + " -m " + exp_mac)

    def test_verifymac_long(self):
        input = self._generate_input(100000)
        key = self._generate_key()
        exp_mac = sha256((key + input).encode()).hexdigest()
        self._run_success(input, "-v -k " + key + " -m " + exp_mac)

    def test_verifymac_fail(self):
        N = 20
        for i in range(1, N):
            input = self._generate_input(i*100)
            key = self._generate_key()
            exp_mac = sha256((key + input).encode()).hexdigest()
            self._run_fail("", "-v -k " + key + " -m " + exp_mac + "a")

    def test_length_extension(self):
        N = 2000
        for i in range(1, N):
            input = self._generate_input(i)
            secret = self._generate_key(SystemRandom().randint(1, 60))
            append_data = self._generate_input(SystemRandom().randint(1, 60))

            original_sig = HashTools.new(algorithm='sha256', raw=bytes(secret, "ascii")+bytes(input, "ascii")).hexdigest()
            magic = HashTools.new("sha256")
            new_data, new_sig = magic.extension(secret_length=len(secret), original_data=bytes(input, "ascii"), append_data=bytes(append_data, "ascii"), signature=original_sig)
            new_data = list(new_data)
            new_data = self.msg_bytes2str(new_data, len(append_data), len(input))

            out = self._run_success(input, f"-e -n {len(secret)} -m {original_sig} -a {append_data}")
            test_new_sig, test_new_msg = tuple(out.split("\n"))[0:2]

            self.assertEqual(new_data, test_new_msg)
            self.assertEqual(new_sig, test_new_sig)

if __name__ == '__main__':
    unittest.main()