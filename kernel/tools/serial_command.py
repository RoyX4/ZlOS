#!/usr/bin/env python3
"""Connect to a QEMU serial socket, wait for readiness, then send a command."""

from __future__ import annotations

import argparse
import os
import socket
import sys
import tempfile
import threading
import time


def connect(path, deadline):
    client = socket.socket(socket.AF_UNIX, socket.SOCK_STREAM)
    while time.monotonic() < deadline:
        try:
            client.connect(path)
            client.settimeout(0.2)
            return client
        except (FileNotFoundError, ConnectionRefusedError):
            time.sleep(0.02)
    client.close()
    raise TimeoutError("serial socket did not become connectable")


def exchange(client, log_path, ready, command, expected, ceiling):
    deadline = time.monotonic() + ceiling
    transcript = ""
    sent = False
    with open(log_path, "w", encoding="latin-1") as log:
        while time.monotonic() < deadline:
            try:
                data = client.recv(65536)
            except socket.timeout:
                data = b""
            except OSError:
                break
            if data:
                text = data.decode("latin-1").replace("\r", "")
                transcript += text
                log.write(text)
                log.flush()
            if not sent and ready in transcript:
                client.sendall(command.encode("latin-1"))
                sent = True
            if sent and all(marker in transcript for marker in expected):
                return True
        return False


def selftest():
    left, right = socket.socketpair()
    with tempfile.TemporaryDirectory(prefix="zlos-serial-command-") as root:
        log = os.path.join(root, "serial.log")

        def guest():
            right.sendall(b"boot\r\nready.\r\n")
            command = right.recv(64)
            if command == b"fib 20\r":
                right.sendall(b"compositor: 3 windows\r\n6765\r\n")
            right.close()

        worker = threading.Thread(target=guest)
        worker.start()
        left.settimeout(0.2)
        if not exchange(left, log, "ready.", "fib 20\r",
                        ["compositor:", "6765"], 2):
            raise RuntimeError("selftest exchange failed")
        worker.join()
        left.close()
        text = open(log, encoding="latin-1").read()
        if "ready." not in text or "6765" not in text or "\r" in text:
            raise RuntimeError("selftest transcript is incomplete or unnormalized")
    print("serial-command selftest: delayed send and marker wait PASS")


def main(argv):
    parser = argparse.ArgumentParser()
    parser.add_argument("--socket")
    parser.add_argument("--log")
    parser.add_argument("--ready")
    parser.add_argument("--send")
    parser.add_argument("--expect", action="append", default=[])
    parser.add_argument("--ceiling", type=float, default=360)
    parser.add_argument("--selftest", action="store_true")
    args = parser.parse_args(argv)
    try:
        if args.selftest:
            selftest()
            return 0
        for name in ("socket", "log", "ready", "send"):
            if getattr(args, name) is None:
                parser.error(f"--{name} is required without --selftest")
        deadline = time.monotonic() + args.ceiling
        client = connect(args.socket, deadline)
        try:
            remaining = max(0.1, deadline - time.monotonic())
            passed = exchange(client, args.log, args.ready, args.send,
                              args.expect, remaining)
        finally:
            client.close()
        if not passed:
            print("serial-command: required markers did not arrive", file=sys.stderr)
            return 1
    except (OSError, RuntimeError, TimeoutError) as error:
        print(f"serial-command: FAIL: {error}", file=sys.stderr)
        return 1
    return 0


if __name__ == "__main__":
    raise SystemExit(main(sys.argv[1:]))
