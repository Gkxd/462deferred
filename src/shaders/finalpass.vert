#version 330 core
// Pass-through vertex shader taken from this tutorial:
// http://www.opengl-tutorial.org/intermediate-tutorials/tutorial-16-shadow-mapping/

in vec3 vertexPosition_modelspace;

out vec2 UV;

void main(){
	gl_Position =  vec4(vertexPosition_modelspace,1);
	UV = (vertexPosition_modelspace.xy+vec2(1,1))/2.0;
}