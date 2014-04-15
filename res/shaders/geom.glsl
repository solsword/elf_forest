#version 130

layout (triangles) in;
layout (triangles, max_vertices=3) out;

void main() {
  gl_Position = gl_in[0].glPosition;
  EmitVertex();

  gl_Position = gl_in[1].glPosition;
  EmitVertex();

  gl_Position = gl_in[2].glPosition;
  EmitVertex();
  EndPrimitive();
}
