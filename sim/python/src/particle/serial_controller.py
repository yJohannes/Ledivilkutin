import serial
import serial.serialutil

from config import BAUDRATE, PORT
from utils import scale_int16_to_int8
from vec2 import Vec2


class SerialController:
    def __init__(self, port: str = PORT, baudrate: int = BAUDRATE):
        while True:
            print("Attemping to connect to " + PORT + "...")

            try:
                self.connection = serial.Serial(PORT, BAUDRATE, timeout=1)
                break
            except serial.serialutil.SerialException as e:
                print(e)

        # self.connection = serial.Serial(port, baudrate, timeout=1)
        self.buffer = b""

    def read_acceleration(self) -> Vec2 | None:
        if self.connection.in_waiting:
            self.buffer += self.connection.read(
                self.connection.in_waiting
            )

        latest = None

        while b"\n" in self.buffer:
            raw_line, self.buffer = self.buffer.split(b"\n", 1)
            line = raw_line.decode("ascii", errors="ignore").strip()

            if not line or line == "0":
                continue

            try:
                raw_ax, raw_ay = map(int, line.split(","))
            except ValueError:
                continue

            raw_ax *= -1

            ax = scale_int16_to_int8(raw_ax) >> 2
            ay = scale_int16_to_int8(raw_ay) >> 2

            print(f"a_raw:    [{raw_ax:>6}, {raw_ay:>6}]")
            print(f"a_scaled: [{ax:>6}, {ay:>6}]")

            latest = Vec2(ax, ay)

        return latest

    def close(self) -> None:
        self.connection.close()
