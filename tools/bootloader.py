#!/usr/bin/env python3
"""Reboot a T114 Pager into the nRF52840 UF2/DFU bootloader over USB.

Usage:
  py tools\\bootloader.py COM4
"""

import argparse
import serial
import struct
import sys
import time

CMD_ENTER_BOOTLOADER = 67
RESP_OK = 0
RESP_ERR = 1


def write_frame(ser, payload: bytes):
    ser.write(b"<" + struct.pack("<H", len(payload)) + payload)
    ser.flush()


def read_exact(ser, n: int, timeout: float = 2.0) -> bytes:
    data = bytearray()
    deadline = time.time() + timeout
    while len(data) < n and time.time() < deadline:
        chunk = ser.read(n - len(data))
        if chunk:
            data.extend(chunk)
    return bytes(data)


def read_frame(ser) -> bytes:
    deadline = time.time() + 2.0
    while time.time() < deadline:
        b = ser.read(1)
        if b == b">":
            hdr = read_exact(ser, 2)
            if len(hdr) != 2:
                break
            ln = struct.unpack("<H", hdr)[0]
            payload = read_exact(ser, ln)
            if len(payload) == ln:
                return payload
            break
    return b""


def main():
    ap = argparse.ArgumentParser()
    ap.add_argument("port", help="Application serial port, e.g. COM4")
    ap.add_argument("--baud", type=int, default=115200)
    args = ap.parse_args()

    with serial.Serial(args.port, args.baud, timeout=0.1) as ser:
        time.sleep(0.3)
        ser.reset_input_buffer()
        write_frame(ser, bytes([CMD_ENTER_BOOTLOADER]))
        resp = read_frame(ser)

        # The device may reset before Windows delivers the OK frame. Both cases
        # are acceptable as long as the serial port disappears shortly after.
        if resp:
            if resp[0] == RESP_OK:
                print("Bootloader request accepted.")
            elif resp[0] == RESP_ERR:
                code = resp[1] if len(resp) > 1 else -1
                raise RuntimeError(f"T114 returned error code {code}")
            else:
                raise RuntimeError(f"Unexpected response code {resp[0]}")
        else:
            print("No reply received; device may already be resetting.")

    print("T114 is rebooting into UF2/DFU mode.")
    print("Windows may assign a different COM port in bootloader mode.")


if __name__ == "__main__":
    try:
        main()
    except Exception as exc:
        print(f"ERROR: {exc}", file=sys.stderr)
        sys.exit(1)
