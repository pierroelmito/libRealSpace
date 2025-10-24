#define MACRO(hash, name) hash name
MACRO(#,version 330)

#ifndef VS
#error "pipo"
#endif

#if VS
#define VAR out
#else
#define VAR in
#endif

#include "common.glsl"

#if PP

VAR vec3 fragCamPos;
VAR vec3 fragViewdir;
VAR vec2 fragTexCoord;

#if VS

in vec3 vertexPosition;

uniform mat4 matModel;
uniform mat4 matView;
uniform mat4 mvp;

void main()
{
	fragTexCoord = 0.5 * (vertexPosition.xy + 1);
	vec3 worldPos = (matModel * vec4(vertexPosition, 0.0)).xyz;
	fragViewdir = normalize(worldPos);
	vec4 p = mvp * vec4(vertexPosition, 0.0);
	p.z = p.w;
	gl_Position = p;
	vec3 camPos = vec3(inverse(matView)[3]);
	fragCamPos = camPos;
}

#else

out vec4 finalColor;

uniform sampler2D texture0;

void main()
{
	vec3 vd = normalize(fragViewdir);
	finalColor = domeSkyColor(1.0, fragCamPos, vd);
}

#endif

#else

VAR vec3 fragWorldPos;
VAR vec3 fragCamPos;
VAR vec3 fragNormal;
VAR vec2 fragTexCoord;
VAR vec4 fragColor;
VAR float fragFog;

#if VS

in vec3 vertexPosition;
in vec3 vertexNormal;
in vec2 vertexTexCoord;
in vec4 vertexColor;

#if INSTANCING
in mat4 instanceTransform;
#endif

uniform mat4 mvp;
uniform mat4 matModel;
uniform mat4 matView;

vec4 fixZB(vec4 p) {
    float nz = 1 - exp(-0.001 * (p.z));
    p.z = nz * p.w;
    return p;
}

void main()
{
	fragTexCoord = vertexTexCoord;
	fragColor = vertexColor;
	fragNormal = vertexNormal;
	vec3 camPos = vec3(inverse(matView)[3]);
#if INSTANCING
	mat4 tr = instanceTransform;
	mat4 mvpi = mvp * tr;
	vec3 worldPos = (matModel * tr * vec4(vertexPosition, 1.0)).xyz;
	gl_Position = fixZB(mvpi * vec4(vertexPosition, 1.0));
#else
	vec3 worldPos = (matModel * vec4(vertexPosition, 1.0)).xyz;
	gl_Position = fixZB(mvp * vec4(vertexPosition, 1.0));
#endif
	fragWorldPos = worldPos;
	fragCamPos = camPos;
	fragFog = gl_Position.z / gl_Position.w;
}

#else //!VS

out vec4 finalColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

//vec3 light(vec3 normal, vec3 lightDir, vec3 lightColor, vec3 ambientColor)
//{
//	normal = normalize(normal);
//	lightDir = normalize(lightDir);
//	float diff = max(dot(normal, lightDir), 0.0);
//	return mix(ambientColor, lightColor, diff);
//}

vec3 light(vec3 normal, vec3 eyedir, vec3 lightDir, vec3 lightColor, vec3 ambientColor)
{
	vec3 halfDir = normalize(lightDir + eyedir);
	float diff = max(dot(normal, lightDir), 0.0);
	float spec = pow(max(dot(normal, halfDir), 0.0), 16.0);
	return mix(ambientColor, lightColor, diff) + lightColor * spec;
}

void main()
{
	vec3 viewDir = normalize(fragWorldPos - fragCamPos);
	vec3 sunColor = vec3(1.0, 1.0, 1.0);
	vec3 shadowColor = vec3(0.4, 0.4, 0.4);
	vec3 lightDir = normalize(vec3(0.5, -1.0, 0.3));
	vec3 diffuseLight = light(fragNormal, viewDir, lightDir, sunColor, shadowColor);
	vec4 texelColor = texture(texture0, fragTexCoord);
	finalColor = texelColor * colDiffuse * fragColor;
	finalColor.xyz *= diffuseLight;
	if (finalColor.a < 0.1)
		discard;
	vec4 fogColor = sceneSkyColor(fragFog, fragCamPos, viewDir, fragWorldPos);
	finalColor.xyz = mix(finalColor.xyz, fogColor.xyz, fogColor.a);
}

#endif

#endif

