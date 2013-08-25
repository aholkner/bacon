import bacon

font = bacon.Font('res/DejaVuSans.ttf', 48)

# The offscreen texture must be created in a separate atlas to the glyphs
# which will be drawn into it.  By specifying atlas=0, we disable atlasing
# for the image entirely.
offscreen = bacon.Image(width = 128, height = 128, atlas=0)

class Game(bacon.Game):
    def draw_scene(self):
        # Draw some overlapping letters
        bacon.draw_string(font, 'ABC', 0, 50)
        bacon.draw_string(font, 'abc', 20, 50)

    def on_tick(self):
        bacon.clear(0.3, 0.3, 0.3, 1)

        # Here the scene is drawn directly to the framebuffer,
        # and where the letters overlap there is double-blending.
        bacon.set_color(0.5, 0.5, 0.5, 0.5)
        self.draw_scene()

        # This time we render the scene to an image first
        bacon.set_frame_buffer(offscreen)
        bacon.clear(0, 0, 0, 0)
        bacon.set_color(1, 1, 1, 1)
        self.draw_scene()

        # Then render the offscreen image back to the framebuffer,
        # There is no double-blending this time because we precomposited
        # the image.
        bacon.set_frame_buffer(None)
        bacon.set_color(0.5, 0.5, 0.5, 0.5)
        bacon.draw_image(offscreen, 0, 100)

bacon.run(Game())