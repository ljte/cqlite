import random
import subprocess
import typing as t
import unittest


CQLITE_BINARY = "./bin/cqlite"


class TestCqlite(unittest.TestCase):
    @staticmethod
    def run_cqlite(commands: t.Sequence[str]) -> list[str]:
        cqlite = subprocess.Popen(args=[],
                                  executable=CQLITE_BINARY,
                                  stdin=subprocess.PIPE,
                                  stdout=subprocess.PIPE,
                                  stderr=subprocess.PIPE)
        with cqlite as p:
            input = ('\n'.join(commands) + '\n').encode()
            stdout, stderr = p.communicate(input)
            if stderr:
                raise ValueError(stderr)
            return stdout.decode().replace('>>> ', '').splitlines()

    def test_insert_and_select(self):
        id = random.randint(0, (2**16 - 1))
        cmds = (
            f"insert {id} new_user new_user@mail.com",
            "select",
            ".exit",
        )
        output = self.run_cqlite(cmds)

        expected = [
            "INSERTED 1",
            f"({id}, new_user, new_user@mail.com)",
        ]
        self.assertListEqual(output, expected)

    def test_error_table_full(self):
        cmds = [
            f"insert {i} user{i} user{i}@mail.com"
            for i in range(1201)
        ]

        with self.assertRaises(ValueError, msg="Error: table full"):
            self.run_cqlite(cmds)

    def test_long_input(self):
        username = 'a' * 32
        email = 'a' * 255
        cmds = [
            f"insert 5 {username} {email}",
            "select",
            ".exit",
        ]

        output = self.run_cqlite(cmds)

        expected = [
            "INSERTED 1",
            f"(5, {username}, {email})",
        ]
        self.assertListEqual(output, expected)

    def test_too_long_string(self):
        username = 'a' * 35
        email = 'a' * 255
        cmds = [
            f"insert 5 {username} {email}",
            "select",
            ".exit",
        ]

        with self.assertRaises(ValueError, msg="Input too long"):
            self.run_cqlite(cmds)

    def test_negative_id(self):
        cmds = ["insert -1 test test@mail.com"]

        with self.assertRaises(ValueError, msg="Id must be positive"):
            self.run_cqlite(cmds)
