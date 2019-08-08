#version 120

varying vec2 v_TexCoord;

uniform sampler2D u_Texture;

uniform vec4 u_Mult;
uniform vec4 u_Tint;

void main() {
    // regulat tint/mult filter
    gl_FragColor = texture2D(u_Texture, v_TexCoord) * u_Mult + u_Tint;

    // greyscale filter
    //    vec4 col = texture2D(u_Texture, v_TexCoord);
    //    float grey = (col.r + col.g + col.b) / 3;
    //    gl_FragColor = vec4(grey, grey, grey, col.a);
}
