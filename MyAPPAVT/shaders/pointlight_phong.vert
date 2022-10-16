#version 330

uniform mat4 m_pvm;
uniform mat4 m_viewModel;
uniform mat3 m_normal;

uniform vec4 l_pos;

in vec4 position;
in vec4 normal;    //por causa do gerador de geometria
in vec2 in_texCoord;

out vec2 out_texCoord;

out Data {
	vec3 normal;
	vec3 eye;
	vec4 p;
} DataOut;

void main () {

	vec4 pos = m_viewModel * position;

	DataOut.normal = normalize(m_normal * normal.xyz);
	DataOut.eye = vec3(-pos);
	DataOut.p = pos;
	out_texCoord = in_texCoord;

	gl_Position = m_pvm * position;	
}