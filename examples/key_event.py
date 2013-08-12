import bacon

class Game(bacon.Game):
    def on_tick(self):
        bacon.clear(0, 0, 0, 1)

    def on_key(self, key, pressed):
        print('bacon.Keys.%s was %s' % (bacon.Keys.tostring(key), 'pressed' if pressed else 'released'))

bacon.run(Game())