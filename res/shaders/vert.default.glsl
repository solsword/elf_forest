#version 130
// vim:syn=c

//GLfloat model[16]; 
//glGetFloatv(GL_MODELVIEW_MATRIX, model); 

//GLfloat projection[16]; 
//glGetFloatv(GL_PROJECTION_MATRIX, projection); 

void main() {
  //gl_Position = projection * model * gl_Vertex;
  gl_Position = ftransform();
  gl_FrontColor = gl_Color;
  gl_BackColor = gl_Color;
  gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
}
