#version 120
//#version 330

//GLfloat model[16]; 
//glGetFloatv(GL_MODELVIEW_MATRIX, model); 

//GLfloat projection[16]; 
//glGetFloatv(GL_MODELVIEW_MATRIX, model); 

void main() {
  //gl_Position = gl_ModelViewProjectionMatrix * gl_Vertex;
  gl_Position = ftransform();
}
