#version 130

uniform sampler2D texture;

void main() {
  //gl_FragColor = vec4(1,1,0.5,1);
  vec4 texture_color = texture2D(texture, gl_TexCoord[0].st);
  vec4 color = gl_Color;
  //color[3] = texture_color[3];
  color = texture_color;
  gl_FragColor = color;
}
