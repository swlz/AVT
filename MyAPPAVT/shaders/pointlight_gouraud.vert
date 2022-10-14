#version 330

#define NUMBER_POINT_LIGHTS 6
#define NUMBER_SPOT_LIGHTS 2

struct DirectionalLight {
    vec4 direction;
}; 

struct PointLight {    
    vec4 position;
};

struct SpotLight {    
    vec4 position;
	vec3 direction;
	float cutoff;
};

uniform sampler2D texmap0;
uniform sampler2D texmap1;
uniform sampler2D texmap2;

uniform PointLight pointLights[NUMBER_POINT_LIGHTS];
uniform SpotLight spotLights[NUMBER_SPOT_LIGHTS];
uniform DirectionalLight dirLight;

uniform mat4 m_pvm;
uniform mat4 m_viewModel;
uniform mat3 m_normal;

uniform vec4 l_pos;

in vec4 position;
in vec4 normal;    //por causa do gerador de geometria


struct Materials {
	vec4 diffuse;
	vec4 ambient;
	vec4 specular;
	vec4 emissive;
	float shininess;
	int texCount;
};

uniform Materials mat;

out Data {
	vec4 color;
} DataOut;

vec4 CalcDirLight(DirectionalLight dirLight, vec3 n, vec3 e)
{
	vec4 spec = vec4(0.0);
	vec3 l = normalize(vec3(-dirLight.direction));

	float intensity = max(dot(n,l), 0.0);

	
	if (intensity > 0.0) {

		vec3 h = normalize(l + e);
		float intSpec = max(dot(h,n), 0.0);
		spec = mat.specular * pow(intSpec, mat.shininess);
	}
	
	return max(intensity * mat.diffuse + spec, mat.ambient);
}  

vec4 CalcPointLight(vec3 l, vec3 n, vec3 e)
{
	vec4 spec = vec4(0.0);

	float intensity = max(dot(n,l), 0.0);

	
	if (intensity > 0.0) {

		vec3 h = normalize(l + e);
		float intSpec = max(dot(h,n), 0.0);
		spec = mat.specular * pow(intSpec, mat.shininess);
	}

	
	return max(intensity * mat.diffuse + spec, mat.ambient);
} 


vec4 calcSpotLight(vec3 l, vec3 s, float cutoff, vec3 n, vec3 e) {
	
	vec4 spec = vec4(0.0);

	float intensity = max(dot(n,l), 0.0);

	if (dot(s,l) > cutoff) {
        float intensity = max(dot(n,l), 0.0);
 
        if (intensity > 0.0) {
            vec3 h = normalize(l + e);
			float intSpec = max(dot(h,n), 0.0);
			spec = mat.specular * pow(intSpec, mat.shininess);
        }
    }

	
	return max(intensity * mat.diffuse + spec, mat.ambient);
}

void main () {

	vec4 pos = m_viewModel * position;


	vec3 normal = normalize(m_normal * normal.xyz);
	vec3 lightDir = vec3(l_pos - pos);
	vec3 eye = vec3(-pos);

	vec4 spec = vec4(0.0);

	vec3 n = normalize(normal);
	vec3 l = normalize(lightDir);
	vec3 e = normalize(eye);

	float intensity = max(dot(n,l), 0.0);

	
	if (intensity > 0.0) {

		vec3 h = normalize(l + e);
		float intSpec = max(dot(h,n), 0.0);
		spec = mat.specular * pow(intSpec, mat.shininess);
	}

	vec3 pl = vec3(pointLights[0].position - pos);
	vec3 pl2 = normalize(pl);
	vec4 col = CalcPointLight(pl2, n,  e);
	
	DataOut.color = max(intensity * mat.diffuse + spec, mat.ambient);

	gl_Position = m_pvm * position;	
}