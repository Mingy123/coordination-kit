"""Emits foot-switch events from keyboard (2 = left, 4 = right)."""
from pynput import keyboard

class FootSwitchMonitor:
    def __init__(self):
        self._left = self._right = False
        self._listener = keyboard.Listener(
            on_press=self._on_press,
            on_release=self._on_release
        )

    def start(self): self._listener.start()
    def stop(self): self._listener.stop()

    @property
    def left(self) -> bool: return self._left
    @property
    def right(self) -> bool: return self._right

    def _on_press(self, key):
        try:
            k = key.char
            if k == '2': self._left = True
            elif k == '4': self._right = True
        except AttributeError:
            pass

    def _on_release(self, key):
        try:
            k = key.char
            if k == '2': self._left = False
            elif k == '4': self._right = False
        except AttributeError:
            pass

if __name__ == "__main__":
    import time
    mon = FootSwitchMonitor()
    mon.start()
    try:
        while True:
            print(f"L={'down' if mon.left else 'up'}  R={'down' if mon.right else 'up'}")
            time.sleep(0.05)
    except KeyboardInterrupt:
        mon.stop()
