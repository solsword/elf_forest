#version 130

//GLfloat model[16]; 
//glGetFloatv(GL_MODELVIEW_MATRIX, model); 

//GLfloat projection[16]; 
//glGetFloatv(GL_MODELVIEW_MATRIX, model); 

void main() {
  //gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
  gl_Position = ftransform();
  gl_FrontColor = gl_Color;
  gl_BackColor = gl_Color;
  gl_TexCoord[0] = gl_TextureMatrix[0] * gl_MultiTexCoord0;
}
