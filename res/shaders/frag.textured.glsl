#version 130
// vim:syn=c

uniform sampler2D texture;

void main() {
  vec4 texture_color = texture2D(texture, gl_TexCoord[0].st);
  gl_FragColor = texture_color;
}
