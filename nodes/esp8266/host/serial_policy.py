"""ESP01-013 Linux host-side serial settings for an already-connected CH340.
No automatic reset or Flash. Never import a hardware module at import time.
Call open_esp_uart ONLY under a separately approved physical test.
"""
from __future__ import annotations
from contextlib import contextmanager
from pathlib import Path
import termios
from typing import Any, Iterator

ESP_CH340_BY_ID="/dev/serial/by-id/usb-1a86_USB_Serial-if00-port0"

class SerialPolicyError(RuntimeError):
    pass

def clear_hupcl_on_open_fd(fd: int, *, termios_api: Any = termios) -> bool:
    """Disable hangup-on-last-close on this *open* tty (not system-wide).

    Returns True if changed. Never changes baudrate, line speed, DTR, RTS,
    GPIO or physical power. Does not open any device.
    """
    before=termios_api.tcgetattr(fd)
    if not (before[2] & termios_api.HUPCL):
        return False
    after=list(before)
    after[2] &= ~termios_api.HUPCL
    termios_api.tcsetattr(fd,termios_api.TCSANOW,after)
    if termios_api.tcgetattr(fd)[2] & termios_api.HUPCL:
        raise SerialPolicyError("HUPCL_REMAINED_ENABLED")
    return True

@contextmanager
def open_esp_uart(*, port: str=ESP_CH340_BY_ID,
                  serial_api: Any=None) -> Iterator[Any]:
    """Explicit opt-in physical acquisition; never called by tests implicitly.

    Caller owns authorization; no esptool, reset, stty subprocess or Flash.
    This is a bench host policy, NOT ESP8266 firmware or STM32 USART2 code.
    """
    path=Path(port)
    if port!=ESP_CH340_BY_ID or not path.is_symlink() or not path.exists():
        raise SerialPolicyError("EXPECTED_CH340_BY_ID_NOT_AVAILABLE")
    if serial_api is None:
        import serial as serial_api
    client=serial_api.Serial(port=None,baudrate=115200,timeout=.05,
                             write_timeout=1.0,exclusive=True)
    client.dtr=False
    client.rts=False
    client.port=port
    try:
        client.open()
        clear_hupcl_on_open_fd(client.fileno())
        yield client
    finally:
        client.close()
