

layout (location = 0) in vec3 position;
layout (location = 3) in vec2 textureCoord;

out vec2 textureCoordinate;

void main(){
	gl_Position = vec4(position, 1.0);
	textureCoordinate = textureCoord;
}
