#version 430

in vec3 pos;
in vec2 inTexCoord;

out vec2 texCoord;

void main() {
    texCoord = inTexCoord;
    gl_Position = vec4(pos, 1);
}
