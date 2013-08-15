import logging
logging.basicConfig(level=logging.WARN)

import bacon

shader = bacon.Shader(vertex_source=
    """
    precision highp float;
    attribute vec3 a_Position;
    attribute vec2 a_TexCoord0;
    attribute vec4 a_Color;

    varying vec2 v_TexCoord0;
    varying vec4 v_Color;

    uniform mat4 g_Projection;

    void main()
    {
        gl_Position = g_Projection * vec4(a_Position, 1.0);
        v_TexCoord0 = a_TexCoord0;
        v_Color = a_Color;
    }
    """,

    fragment_source=
    """
    precision highp float;
    
    uniform sampler2D g_Texture0;
    uniform sampler2D mask;
    uniform float brightness;
    
    varying vec2 v_TexCoord0;
    varying vec4 v_Color;

    void main()
    {
        gl_FragColor = v_Color * texture2D(g_Texture0, v_TexCoord0) * texture2D(mask, v_TexCoord0);
    }
    """)

brightness = shader.uniforms['brightness']
shader.uniforms['mask'].value = bacon.Image('res/ball.png')
kitten = bacon.Image('res/kitten.png')

class Game(bacon.Game):
    def on_tick(self):
        bacon.clear(0, 0, 0, 1)
        bacon.set_shader(shader)
        #brightness.value = bacon.mouse.x / float(bacon.window.width)
        bacon.draw_image(kitten, 0, 0)

    def on_key(self, key, pressed):
        shader.uniforms['mask'].value = bacon.Image('res/PngSuite.png')
        


bacon.run(Game())