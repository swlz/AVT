#version 330

out vec4 colorOut;

in Data {
	vec4 color;
} DataIn;

void main() {
	colorOut = DataIn.color;
}