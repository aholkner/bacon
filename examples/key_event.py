import bacon

class Game(bacon.Game):
    def on_tick(self):
        bacon.clear(0, 0, 0, 1)

    def on_key(self, key, pressed):
        print('key %d was %s' % (key, 'pressed' if pressed else 'released'))

bacon.run(Game())