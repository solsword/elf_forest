#version 130
// vim:syn=c

//GLfloat model[16]; 
//glGetFloatv(GL_MODELVIEW_MATRIX, model); 

//GLfloat projection[16]; 
//glGetFloatv(GL_PROJECTION_MATRIX, projection); 

// Logarithmic z-buffer interpolation correction:
out float flogz;

void main() {
  //gl_Position = projection * model * gl_Vertex;
  gl_Position = ftransform();
  gl_FrontColor = gl_Color;
  gl_BackColor = gl_Color;
  gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;

  // Logarithmic depth buffer from:
  // outerra.blogspot.com/2013/07/logarithmic-depth-buffer-optimizations.html
  float far = 1024.0; // TODO: make this a parameter.
  float Fcoef = 2.0 / log2(far + 1.0);
  gl_Position.z = log2(max(1e-6, 1.0 + gl_Position.w)) * Fcoef - 1.0;
  flogz = 1.0 + gl_Position.w;
}
