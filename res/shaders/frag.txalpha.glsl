#version 130
// vim:syn=c

uniform sampler2D texture;

void main() {
  vec4 texture_color = texture2D(texture, gl_TexCoord[0].st);
  vec4 color = gl_Color;
  color[3] = texture_color[3];
  gl_FragColor = color;
}
