import bacon

# Load the music file, set stream=True since this is a long file that should be
# streamed from disk
music = bacon.Sound('res/PowerChorus2.ogg', stream=True)

# Create an instance of the music and start playing it
music_voice = bacon.Voice(music, loop=True)
music_voice.play()

class Game(bacon.Game):
	def on_tick(self):
		bacon.clear(0, 0, 0, 1)

		# Change the pan of the music based on the mouse position
		music_voice.pan = bacon.mouse.x / bacon.window.width * 2 - 1

	def on_key(self, key, pressed):
		# Pressing spacebar pauses/resumes the music
		if key == bacon.Keys.space and pressed:
			music_voice.playing = not music_voice.playing

bacon.run(Game())