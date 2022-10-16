#version 330

out vec4 colorOut;

in vec4 texCoord;

struct Materials {
	vec4 diffuse;
	vec4 ambient;
	vec4 specular;
	vec4 emissive;
	float shininess;
	int texCount;
};

uniform Materials mat;

in Data {
	vec3 normal;
	vec3 eye;
	vec4 p;
} DataIn;

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
	
	//return max(intensity * mat.diffuse + spec, mat.ambient.a);
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

	
	//return max(intensity * mat.diffuse + spec, mat.ambient.a);
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

	
	//return max(intensity * mat.diffuse + spec, mat.ambient.a);
	return max(intensity * mat.diffuse + spec, mat.ambient);
}

void main() {

	vec4 spec = vec4(0.0);

	vec3 n = normalize(DataIn.normal);
	vec3 e = normalize(DataIn.eye);
	vec4 pos = normalize(DataIn.p);

	colorOut = CalcDirLight(dirLight, n, e);

	for (int i = 0; i < NUMBER_SPOT_LIGHTS; i++) {
		vec3 l = normalize(vec3(spotLights[i].position - pos));
		vec3 s = normalize(-spotLights[i].direction); 
		float cutoff = spotLights[i].cutoff;
		colorOut += calcSpotLight(l, s, cutoff, n, e);
	}

	for (int i = 0; i < NUMBER_POINT_LIGHTS; i++) {
		vec3 pl = vec3(pointLights[i].position - pos);
		vec3 pl2 = normalize(pl);
		colorOut += CalcPointLight(pl2, n, e)/6;
	}
}