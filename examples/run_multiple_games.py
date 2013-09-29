import bacon

font = bacon.Font(None, 12)

class Menu(bacon.Game):
    def on_tick(self):
        bacon.clear(0, 0.2, 0, 1)
        bacon.draw_string(font, 'This is the menu; click to start game', 
                          bacon.window.width / 2, bacon.window.height / 2,
                          align=bacon.Alignment.center,
                          vertical_align=bacon.VerticalAlignment.center)

    def on_mouse_button(self, button, pressed):
        if pressed:
            bacon.run(game)

class Game(bacon.Game):
    def on_tick(self):
        bacon.clear(0.2, 0, 0, 1)
        bacon.draw_string(font, 'This is the game; click to return to menu', 
                          bacon.window.width / 2, bacon.window.height / 2,
                          align=bacon.Alignment.center,
                          vertical_align=bacon.VerticalAlignment.center)

    def on_mouse_button(self, button, pressed):
        if pressed:
            bacon.run(menu)

game = Game()
menu = Menu()
bacon.run(menu)
