"""Verify the managed native launch boundary with a local Base fixture."""

import os
from pathlib import Path
import select
import socket
import subprocess
import sys
import tempfile
import time


class BaseVerification:
    def __init__(self, executable, directory):
        self.executable = executable
        self.address = str(Path(directory) / "base.sock")
        self.env = {key: value for key, value in os.environ.items()
                    if not key.startswith("TOOL_BASE_") and key != "TOOL_LAUNCH_HOST"}

    def configuration(self):
        return {
            **self.env,
            "TOOL_BASE_SOCKET": self.address,
            "TOOL_BASE_PEER_UID": str(os.getuid()),
            "TOOL_BASE_TARGET": "base-1",
            "TOOL_BASE_ENVIRONMENT": "test",
            "TOOL_BASE_RELEASE_SHA256": "a" * 64,
            "TOOL_BASE_GENERATION": "17",
            "TOOL_BASE_FENCE": "23",
            "TOOL_BASE_COMMAND_ID": "attempt-1",
        }

    def command(self, group="launch", action="managed-start"):
        return [self.executable, group, action, "mobile", "0"]

    def assert_denied(self, result, error):
        assert result.returncode == 1, (result.returncode, result.stderr)
        assert error in result.stderr, result.stderr
        assert "http://127.0.0.1:" not in result.stdout, result.stdout
        assert "Liftoff:" not in result.stdout, result.stdout

    def denied_configuration(self, env, error, group="launch", action="managed-start"):
        result = subprocess.run(self.command(group, action), env=env, timeout=5,
                                capture_output=True, text=True, check=False)
        self.assert_denied(result, error)

    def exchange(self, listener, env, response, error=None):
        with subprocess.Popen(self.command(), env=env, stdout=subprocess.PIPE,
                              stderr=subprocess.PIPE) as process:
            try:
                connection, _ = listener.accept()
                with connection:
                    connection.settimeout(3)
                    packet = connection.recv(2048)
                    if response is None and error == "endpoint identity mismatch":
                        assert packet == b"", packet
                    else:
                        expected = ["tool-base/1", "authorize-launch", "attempt-1",
                                    "base-1", "test", socket.gethostname(), "a" * 64,
                                    "17", "23", "mobile", "0", "direct"]
                        assert packet == "\t".join(expected).encode(), packet
                    if response is not None:
                        connection.sendall(response)
                    if error is not None:
                        stdout, stderr = process.communicate(timeout=5)
                        result = subprocess.CompletedProcess(process.args, process.returncode,
                                                             stdout.decode(), stderr.decode())
                        self.assert_denied(result, error)
                        return
                self.assert_ready(process)
            finally:
                if process.poll() is None:
                    process.kill()

    def assert_ready(self, process):
        output = b""
        deadline = time.monotonic() + 5
        marker = b"http://127.0.0.1:"
        while marker not in output:
            remaining = deadline - time.monotonic()
            assert remaining > 0 and select.select([process.stdout], [], [], remaining)[0], output
            chunk = os.read(process.stdout.fileno(), 4096)
            assert chunk, output
            output += chunk
        process.terminate()
        assert process.wait(timeout=5) == 0

    def run(self):
        env = self.configuration()
        self.denied_configuration(self.env, "Missing Base configuration")
        self.denied_configuration(env, "Cannot connect to Base authorization endpoint")
        self.denied_configuration({**env, "TOOL_BASE_FENCE": "0"}, "Invalid Base numeric binding")
        for group in ("launch", "workspace", "server"):
            self.denied_configuration({**self.env, "TOOL_BASE_MANAGED": "1"},
                                      "Missing Base configuration", group, "start")
        with socket.socket(socket.AF_UNIX, socket.SOCK_SEQPACKET) as listener:
            listener.bind(self.address)
            listener.listen()
            listener.settimeout(5)
            self.exchange(listener, {**env, "TOOL_BASE_PEER_UID": str(os.getuid() + 1)},
                          None, "endpoint identity mismatch")
            self.exchange(listener, env, b"tool-base/1\tdeny\tattempt-1", "denied or invalid")
            self.exchange(listener, env, b"tool-base/1\tallow\tother", "denied or invalid")
            self.exchange(listener, env, None, "did not complete the exchange")
            self.exchange(listener, env, b"tool-base/1\tallow\tattempt-1")


with tempfile.TemporaryDirectory(prefix="tool-base-") as directory:
    BaseVerification(str(Path(sys.argv[1]).resolve() / "tool"), directory).run()
    print("Managed native Base launch checks passed")
