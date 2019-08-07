#version 120

varying vec2 v_TexCoord;

uniform sampler2D u_Texture;

void main() {
    gl_FragColor = texture2D(u_Texture, v_TexCoord);
    //gl_FragColor = vec4(1.0, 0.0, 0.0, 1.0);
}
