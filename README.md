# Bacon

*Bacon* is a Python package for making 2D games that run on Windows and OS X.  It provides functions for drawing graphics, playing sounds, and receiving input from the mouse, keyboard and game controllers.

Here is a simple example of a complete Bacon game::

```python
import bacon

kitten = bacon.Image('res/kitten.png')

bacon.window.width = 512
bacon.window.height = 512

class Game(bacon.Game):
    def on_tick(self):
        bacon.clear(0, 0, 0, 1)
        bacon.draw_image(kitten, 0, 0)
        
bacon.run(Game())
```

See the [documentation](https://bacon.readthedocs.org/en/latest) for more examples.

## Getting started

Bacon requires:

* Python 2.7 (32 or 64-bit) or later, or
* Python 3.3 (32 or 64-bit) or later

and either

* Windows Vista or later, or
* Mac OS X 10.6 or later

[Download the latest release from PyPI](https://pypi.python.org/pypi/bacon) as a `.zip` archive -- including prebuilt binaries, source and example code.  Alternatively, install the latest version into your current environment with:

    $ pip install -U bacon

## Documentation

Documentation is hosted online at

* https://bacon.readthedocs.org/en/latest

## Get in touch

Bacon is developed by Alex Holkner (alex.holkner@gmail.com).  Drop me a line if you have any feedback or need support.
