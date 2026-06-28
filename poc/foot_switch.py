"""Emits foot-switch events from keyboard (space = press)."""
from pynput import keyboard

class FootSwitchMonitor:
    def __init__(self):
        self._pressed = False
        self._listener = keyboard.Listener(
            on_press=self._on_press,
            on_release=self._on_release
        )

    def start(self):
        self._listener.start()

    def stop(self):
        self._listener.stop()

    @property
    def pressed(self) -> bool:
        return self._pressed

    def _on_press(self, key):
        if key == keyboard.Key.space:
            self._pressed = True

    def _on_release(self, key):
        if key == keyboard.Key.space:
            self._pressed = False

if __name__ == "__main__":
    import time
    mon = FootSwitchMonitor()
    mon.start()
    try:
        while True:
            print("foot-down" if mon.pressed else "foot-up")
            time.sleep(0.05)
    except KeyboardInterrupt:
        mon.stop()
