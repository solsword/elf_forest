#version 130
// vim:syn=c

uniform sampler2D texture;

in float flogz;

void main() {
  vec4 texture_color = texture2D(texture, gl_TexCoord[0].st);
  float shade = gl_Color.r;
  gl_FragColor = texture_color;
  gl_FragColor.r *= shade; // lighting
  gl_FragColor.g *= shade; // lighting
  gl_FragColor.b *= shade; // lighting

  // Logarithmic z-buffer interpolation correction:
  float far = 1024.0; // TODO: make this a parameter.
  float Fcoef_half = 1.0 / log2(far + 1.0);
  gl_FragDepth = log2(flogz) * Fcoef_half;
}
