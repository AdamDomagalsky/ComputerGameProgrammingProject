#version 330 core

in vec2 TexCoord0;
in vec3 interpNormal;
in vec3 Tangent;
out vec4 glFragColor;

uniform vec3 uCameraPosition;

#define PI 3.1415926f
#define EPSILON 10e-5f
#define saturate(value) clamp(value, 0.0f, 1.0f);

float BRDF_Lambert(float NdotL)
{
    return NdotL;
}

float BRDF_D_GGX(float NdotH, float Roughness)
{
    float Roughness2 = Roughness * Roughness;
	float NdotH2 = NdotH * NdotH;
    float r1 = (NdotH2 - 1.0) / (Roughness2 * NdotH2);
	float r2 = 1.0 / (PI * (Roughness2 * NdotH2) * Roughness2);
	return exp(r1) * r2;
}


float BRDF_F_FresnelSchlick(float NdotV, float F0)
{
    return (F0 + (1.0f - F0) * (pow(1.0f - max(NdotV,0.0f),5.0f)));;
}

float BRDF_G_SmithGGXCorrelated(float NdotL, float NdotV, float Roughness)
{
    float Roughness2 = Roughness * Roughness;
    float GV = NdotL * sqrt((-NdotV * Roughness2 + NdotV) * NdotV + Roughness2);
    float GL = NdotV * sqrt((-NdotL * Roughness2 + NdotL) * NdotL + Roughness2);

    return 0.5f / (GV + GL + EPSILON);
}

float BRDF_Specular(float NdotV, float NdotL, float NdotH, float LdotH, float Roughness, float F0)
{
    float D = BRDF_D_GGX(NdotH, Roughness);
    float F = BRDF_F_FresnelSchlick(LdotH, F0);
    float G = BRDF_G_SmithGGXCorrelated(NdotL, NdotV, Roughness);
    return (D * F * G) / PI;
}

void main()
{
    vec3 normal = normalize(interpNormal);

	vec3 Position = vec3(0.5f, 0.25f, 0.55f);
    vec4 BaseColor = vec4(0.65f, 1.0f, 0.5f, 0.65f);
    vec4 SpecularColor = vec4(0.35f, 0.75f, 0.15f, 0.85f);

    vec3 LightDirection = normalize(vec3(1.5, -0.5, 3) - Position);
    vec3 ViewDirection = normalize(uCameraPosition - Position);
    vec3 HalfVector = normalize(ViewDirection + LightDirection);
    float Roughness = 0.05f;

    float RefractiveIndex = 0.85f;
    float F0 = pow(((1.0f - RefractiveIndex) / (1.0f + RefractiveIndex)), 2);

    float NdotL = saturate(dot(LightDirection, normal));
    float NdotV = abs(dot(ViewDirection, normal)) + EPSILON; 
    float LdotH = saturate(dot(LightDirection, HalfVector));
    float NdotH = saturate(dot(normal, HalfVector));

    float DiffuseFactor = BRDF_Lambert(NdotL);
    float SpecularFactor = 0.15f;
    if(DiffuseFactor > 0.0f)
    {
        SpecularFactor = BRDF_Specular(NdotV, NdotL, NdotH, LdotH, Roughness, F0);
    }
    glFragColor = BaseColor * DiffuseFactor + SpecularColor * SpecularFactor;
}