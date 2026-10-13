#!/usr/bin/env python3
"""Manage T114 Pager canned messages over the USB companion protocol.

Examples:
  py tools/canned_cli.py COM4 list
  py tools/canned_cli.py COM4 add "Bin in 10 Minuten da"
  py tools/canned_cli.py COM4 del 2
  py tools/canned_cli.py COM4 reload
"""

import argparse
import serial
import struct
import sys
import time

CMD_CANNED = 66
RESP_OK = 0
RESP_ERR = 1
RESP_CANNED_INFO = 29


def write_frame(ser, payload: bytes):
    ser.write(b"<" + struct.pack("<H", len(payload)) + payload)
    ser.flush()


def read_exact(ser, n: int) -> bytes:
    data = bytearray()
    deadline = time.time() + 3.0
    while len(data) < n and time.time() < deadline:
        chunk = ser.read(n - len(data))
        if chunk:
            data.extend(chunk)
    if len(data) != n:
        raise TimeoutError("Timeout waiting for T114 response")
    return bytes(data)


def read_frame(ser) -> bytes:
    deadline = time.time() + 3.0
    while time.time() < deadline:
        b = ser.read(1)
        if b == b">":
            ln = struct.unpack("<H", read_exact(ser, 2))[0]
            return read_exact(ser, ln)
    raise TimeoutError("No companion response from T114")


def transact(ser, payload: bytes) -> bytes:
    ser.reset_input_buffer()
    write_frame(ser, payload)
    return read_frame(ser)


def check_ok(resp: bytes):
    if not resp:
        raise RuntimeError("Empty response")
    if resp[0] == RESP_OK:
        return
    if resp[0] == RESP_ERR:
        code = resp[1] if len(resp) > 1 else -1
        raise RuntimeError(f"T114 returned error code {code}")
    raise RuntimeError(f"Unexpected response code {resp[0]}")


def cmd_list(ser):
    first = transact(ser, bytes([CMD_CANNED, 0, 0]))
    if not first or first[0] != RESP_CANNED_INFO or len(first) < 3:
        raise RuntimeError("Unexpected list response")

    count = first[2]
    print(f"/canned.txt - {count} message(s):")
    if count == 0:
        return

    def show(resp):
        idx = resp[1]
        msg = resp[3:].split(b"\0", 1)[0].decode("utf-8", errors="replace")
        print(f"[{idx}] {msg}")

    show(first)
    for idx in range(1, count):
        resp = transact(ser, bytes([CMD_CANNED, 0, idx]))
        if not resp or resp[0] != RESP_CANNED_INFO:
            raise RuntimeError(f"Unexpected response for index {idx}")
        show(resp)


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("port", help="Serial port, e.g. COM4")
    ap.add_argument("command", choices=["list", "add", "del", "reload"])
    ap.add_argument("value", nargs="?")
    ap.add_argument("--baud", type=int, default=115200)
    args = ap.parse_args()

    with serial.Serial(args.port, args.baud, timeout=0.2) as ser:
        time.sleep(0.4)

        if args.command == "list":
            cmd_list(ser)
        elif args.command == "add":
            if not args.value:
                ap.error("add requires message text")
            raw = args.value.encode("utf-8")
            if len(raw) > 40:
                ap.error("message is longer than 40 UTF-8 bytes")
            check_ok(transact(ser, bytes([CMD_CANNED, 1]) + raw))
            print(f'Added: "{args.value}"')
        elif args.command == "del":
            if args.value is None:
                ap.error("del requires an index")
            idx = int(args.value)
            if not 0 <= idx <= 255:
                ap.error("index out of range")
            check_ok(transact(ser, bytes([CMD_CANNED, 2, idx])))
            print(f"Deleted [{idx}]")
        elif args.command == "reload":
            check_ok(transact(ser, bytes([CMD_CANNED, 3])))
            print("Reloaded /canned.txt")


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        sys.exit(1)
