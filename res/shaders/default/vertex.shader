#version 120

attribute vec4 coord;
attribute vec2 texCoord;

varying vec2 v_TexCoord;

uniform mat4 u_MVP;

void main() {
    gl_Position = u_MVP * coord;
    v_TexCoord = texCoord;
}
