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
	Materials materials;
}; 

struct PointLight {    
    vec4 position;
	Materials materials;
};

struct SpotLight {    
    vec4 position;
	vec3 direction;
	float cutoff;
	Materials materials;
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
		spec = dirLight.materials.specular * pow(intSpec, dirLight.materials.shininess);
	}
	//vec3 diffuse = intensity * texture(texmap0, texCoord).rgb;

	//if(dirLight.materials.ambient.a == 0.0) 
	//	discard;
	
	//return max(intensity * dirLight.materials.diffuse + spec,dirLight.materials.ambient.a);
	
	return max(intensity * dirLight.materials.diffuse + spec, dirLight.materials.ambient);
}  

vec4 CalcPointLight(vec3 l, vec3 n, vec3 e, Materials materials)
{
	vec4 spec = vec4(0.0);

	float intensity = max(dot(n,l), 0.0);

	
	if (intensity > 0.0) {

		vec3 h = normalize(l + e);
		float intSpec = max(dot(h,n), 0.0);
		spec = materials.specular * pow(intSpec, materials.shininess);
	}

	//if(materials.ambient.a == 0.0) discard;

	
	//return max(intensity * materials.diffuse + spec, materials.ambient.a);
	return max(intensity * materials.diffuse + spec, materials.ambient);
} 

vec4 calcSpotLight(vec3 l, vec3 s, float cutoff, vec3 n, vec3 e, Materials materials) {
	
	vec4 spec = vec4(0.0);

	float intensity = max(dot(n,l), 0.0);

	if (dot(s,l) > cutoff) {
        float intensity = max(dot(n,l), 0.0);
 
        if (intensity > 0.0) {
            vec3 h = normalize(l + e);
			float intSpec = max(dot(h,n), 0.0);
			spec = materials.specular * pow(intSpec, materials.shininess);
        }
    }

	//if(materials.ambient.a == 0.0) discard;
	
	//return max(intensity * materials.diffuse + spec, materials.ambient.a);
	return max(intensity * materials.diffuse + spec, materials.ambient);
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
		colorOut += calcSpotLight(l, s, cutoff, n, e, spotLights[i].materials);
	}

	for (int i = 0; i < NUMBER_POINT_LIGHTS; i++) {
		vec3 pl = vec3(pointLights[i].position - pos);
		vec3 pl2 = normalize(pl);
		colorOut += CalcPointLight(pl2, n, e, pointLights[i].materials)/6;
	}

}