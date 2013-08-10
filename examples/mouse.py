import bacon

font = bacon.Font('res/DejaVuSans.ttf', 12)

class Game(bacon.Game):
    def on_tick(self):
        bacon.clear(0, 0, 0, 1)
        bacon.draw_string(font, 'Mouse is at %d, %d' % (bacon.mouse.x, bacon.mouse.y), 5, 15)
        if bacon.mouse.left:
            bacon.draw_string(font, 'Left button down', 5, 35)
        if bacon.mouse.right:
            bacon.draw_string(font, 'Right button down', 5, 55)

bacon.run(Game())