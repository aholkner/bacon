import bacon

bacon.window.resizable = True
bacon.window.target = bacon.Image(width=512, height=512, atlas=0)

font = bacon.Font(None, 16)

class Game(bacon.Game):
    def on_tick(self):
        bacon.clear(0.2, 0.2, 0.2, 1.0)
        bacon.draw_string(font, 
            'Window target dimensions are fixed to 512x512 but window is resizable; press "f" for fullscreen', 
            x=0, y=0,
            width=512, height=512,
            align=bacon.Alignment.center,
            vertical_align=bacon.VerticalAlignment.center)

    def on_key(self, key, pressed):
        if key == bacon.Keys.f and pressed:
            bacon.window.fullscreen = not bacon.window.fullscreen

bacon.run(Game())