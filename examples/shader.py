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
    uniform float brightness;
    uniform float contrast;

    varying vec2 v_TexCoord0;
    varying vec4 v_Color;

    void main()
    {
        // Standard vertex color and texture
        vec4 color = v_Color * texture2D(g_Texture0, v_TexCoord0);

        // Brightness / contrast
        color = vec4(brightness + 0.5) + (color - vec4(0.5)) * vec4(contrast);

        gl_FragColor = color;
    }
    """)
brightness = shader.uniforms['brightness']
contrast = shader.uniforms['contrast']

kitten = bacon.Image('res/kitten.png')

class Game(bacon.Game):
    def on_tick(self):
        bacon.clear(0, 0, 0, 1)
        bacon.set_shader(shader)
        brightness.value = bacon.mouse.y / float(bacon.window.width)
        contrast.value = bacon.mouse.x / float(bacon.window.width)
        bacon.draw_image(kitten, 0, 0)

bacon.run(Game())