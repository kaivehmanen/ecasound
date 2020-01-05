"""Native Python ECI (ecasound control interface) implementation"""
from __future__ import print_function
import sys
import re
import subprocess
from select import select
import os
import signal
import time

AUTHORS = """Kai Vehmanen, Eric S. Tiedemann, Janne Halttunen"""

if sys.version.split()[0] < '2.7':
    print("ERROR: Python 2.7 or newer is required by ecacontrol.py",
          file=sys.stderr)
    sys.exit(-1)


class ECA_CONTROL_INTERFACE:
    def __init__(self, verbose=1):
        """Instantiate new ECI session

        verbose: set this false to get rid of startup-messages
        """
        self.type_override = {}
        self.verbose = verbose
        self._cmd = ""
        self._type = ""
        self._prompt = "ecasound ('h' for help)> "
        self._timeout = 1  # in seconds
        self._resp = {}
        self.initialize()

    def __call__(self, cmd, f=None):
        if f is not None:
            val = self.command_float_arg(cmd, f)
        else:
            cmds = cmd.split("\n")
            if len(cmds) > 1:
                v = []
                for c in cmds:
                    c = c.strip()
                    if c:
                        v.append(self.command(c))

                        if self.error():
                            raise Exception(v[-1])

                val = "\n".join(list(map(str, v)))
            else:
                val = self.command(cmd)

        if self.error():
            raise Exception(val)

        return val

    def _readline(self):
        """Return one line of ECA output"""
        return self.eca.stdout.readline().decode().strip()

    def _read_eca(self):
        """Return ECA output (or None if timeout)"""

        str_buf = ""
        timeout = time.time() + self._timeout
        while time.time() < timeout:
            in_buf = b""
            while select([self.eca.stdout], [], [], 0)[0]:
                in_buf += self.eca.stdout.read(1)
            str_buf += in_buf.decode()
            if str_buf.endswith("> "):
                return str_buf

    def _parse_response(self):
        r = ()
        if self.verbose > 2:
            print("c=" + self._cmd)

        s = self._read_eca()

        if s is None:
            r = ("e", "Connection to the processing engine was lost.\n")
        elif s:
            if self.verbose > 3:
                print("s=<", s, ">")
            m = expand_eiam_response(s)
            # print('expand_eiam_response', m)
            r = parse_eiam_response(s, m)

        if not r:
            r = ("e", "-")

        if self.verbose > 2:
            print("r=", r)

        self._type = r[0]

        if self._cmd in self.type_override:
            self._type = self.type_override[self._cmd]

        if self._type == "S":
            self._resp[self._type] = r[1].split(",")
        elif self._type == "Sn":
            self._resp[self._type] = r[1].split("\n")
        elif self._type == "f":
            self._resp[self._type] = float(r[1])
        elif self._type == "i":
            self._resp[self._type] = int(r[1])
        elif self._type == "li":
            self._resp[self._type] = int(r[1])
        else:
            self._resp[self._type] = r[1]

        return self._resp[self._type]

    def initialize(self):
        """Reserve resources"""

        try:
            ecasound_binary = os.environ["ECASOUND"]
        except KeyError:
            ecasound_binary = ""

        if ecasound_binary == "":
            ecasound_binary = "ecasound"

        self.eca = subprocess.Popen(
            [ecasound_binary, "-c", "-E", "int-output-mode-wellformed"],
            shell=False,
            bufsize=0,
            stdin=subprocess.PIPE,
            stdout=subprocess.PIPE,
            close_fds=True,
        )

        lines = self._readline() + "\n"
        version = self._readline()
        s = version.find("ecasound v")
        if float(version[s + 10: s + 13]) >= 2.2:
            lines = lines + version + "\n"
        else:
            raise RuntimeError("ecasound version 2.2+ required!")
        lines = lines + self._readline() + "\n"

        if self.verbose:
            print(lines)
            print(__doc__)
            print("by", AUTHORS)
            print("\n(to get rid of this message, pass zero to instance init)")

        print(self._parse_response())

        # self.command('debug 256')

    def cleanup(self):
        """Free all reserved resources"""

        self.eca.stdin.write("quit\n".encode())

        os.kill(self.eca.pid, signal.SIGTERM)

        signal.signal(signal.SIGALRM, handler)
        signal.alarm(2)

        try:
            return self.eca.wait()
        except:
            signal.alarm(0)
            os.kill(self.eca.pid, signal.SIGKILL)

    def command(self, cmd):
        """Issue an EIAM command"""

        cmd = cmd.strip()
        if cmd:
            self._cmd = cmd
            cmd += "\r"
            self.eca.stdin.write(cmd.encode())
            return self._parse_response()

    def command_float_arg(self, cmd, f=None):
        """Issue an EIAM command

        This function can be used instead of command(string),
        if the command in question requires exactly one numerical parameter.
        """

        cmd = cmd.strip()
        if cmd:
            self._cmd = cmd
            if f:
                self.eca.stdin.write(("%s %f\n" % (cmd, f)).encode())
            else:
                self.eca.stdin.write((cmd + "\n").encode())
            return self._parse_response()

    def error(self):
        """Return true if error has occured
        during the execution of last EIAM command"""

        if self._type == "e":
            return 1

    def last_error(self):
        """Return a string describing the last error"""

        if self.error():
            return self._resp.get("e")

        return ""

    def last_float(self):
        """Return the last floating-point return value"""
        return self._resp.get("f")

    def last_integer(self):
        """Return the last integer return value

        This function is also used to return boolean values."""
        return self._resp.get("i")

    def last_long_integer(self):
        """Return the last long integer return value

        Long integers are used to pass values like 'length_in_samples'
        and 'length_in_bytes'.  It's implementation specific whether there's
        any real difference between integers and long integers."""
        return self._resp.get("li")

    def last_string(self):
        """Return the last string return value"""
        return self._resp.get("s")

    def last_string_list(self):
        """Return the last collection of strings (one or more strings)"""
        return self._resp.get("S")

    def last_type(self):
        """Return the last type"""
        return self._type

    def current_event(self):
        """** not implemented **"""

    def events_available(self):
        """** not implemented **"""

    def next_event(self):
        """** not implemented **"""


def handler(*args):
    print("AARGH!")
    raise Exception("killing me not so softly")


EXPAND = re.compile("256 ([0-9]{1,5}) (.+)\r\n(.*)\r\n\r\n.*",
                    re.MULTILINE | re.S)


def expand_eiam_response(st):
    """Checks wheter 'str' is a valid EIAM response.

    @return Regex match object.
    """

    return EXPAND.search(st)


PARSE = re.compile("256 ([0-9]{1,5}) (.+)\r\n(.*)", re.MULTILINE | re.S)


def parse_eiam_response(st, m=None):
    """Parses a valid EIAM response.

    @param m Valid regex match object.
    @param str The whole EIAM response.

    @return tuple of return value type and value
    """

    if not m:
        m = PARSE.search(st)
        if not m:
            return ()

    if m and len(m.groups()) == 0:
        return ("e", "Matching groups failed")

    if m and len(m.groups()) == 3:
        if int(m.group(1)) != len(m.group(3)):
            print(
                "(pyeca) Response length error. Received ",
                len(m.group(3)),
                ", expected for ",
                m.group(1),
                ".",
            )
            return ("e", "Response length error.")

    if m:
        return (m.group(2), m.group(3))

    return ("e", "")


class CmdBase:
    def __init__(self, eci, cmd):
        self.eci = eci
        self.cmd = cmd.replace("_", "-")

    def __call__(self):
        return self.eci(self.cmd)


class StringArgument(CmdBase):
    def __call__(self, s):
        return self.eci("%s %s" % (self.cmd, s))


class EIAM:
    def __init__(self, verbose=0):
        self._eci = ECA_CONTROL_INTERFACE(verbose)
        self._cmds = self._eci("int-cmd-list")

        for c in self._cmds:
            c = c.replace("-", "_")
            if c.count("add") or c.count("select"):
                self.__dict__[c] = StringArgument(self._eci, c)
            else:
                self.__dict__[c] = CmdBase(self._eci, c)


def main():
    e = ECA_CONTROL_INTERFACE()
    print(e.command("c-add huppaa"))
    print(e.command("c-list"))

    print(
        e(
            """

    c-list
    c-status
    """
        )
    )

    print(e.cleanup())


if __name__ == "__main__":
    main()
