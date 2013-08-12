import bacon

class Game(bacon.Game):
    def on_tick(self):
        bacon.clear(0, 0, 0, 1)

    def on_mouse_button(self, button, pressed):
        print('bacon.MouseButtons.%s was %s' % (bacon.MouseButtons.tostring(button), 'pressed' if pressed else 'released'))

bacon.run(Game())