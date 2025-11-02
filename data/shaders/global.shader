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

float CoeffZ = -0.001;

vec4 fixZB(vec3 cam, vec3 wpos, vec4 p) {
    //float nz = 0.01 + 0.9 * (1 - exp(CoeffZ * p.z));
    //p.z = nz * p.w;
    return p;
}

vec3 makeWPos(vec3 cam, vec3 v0, vec3 v1, float d0, float d1)
{
    //float nz = 1 - exp(CoeffZ * (p.z));
	float z = log(1 - d1) / CoeffZ;
	return cam + (z * v0);
}

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

#include "common.glsl"

void main()
{
	vec4 bg = texture(texture0, fragTexCoord);
	float d = texture(depthMap, fragTexCoord).r;
	float depth = 2 * (d - 0.5);
	vec3 lightDir = normalize(vec3(0.5, -1.0, 0.3));
	vec3 vd = normalize(fragViewdir);
	if (d == 1.0) {
		finalColor = vec4(domeSkyColor(lightDir, 1.0, fragCamPos, vd), 1.0);
	} else {
		vec3 sky = baseSkyColor(vd, lightDir);
		vec3 wpos = makeWPos(fragCamPos, vd,  fragViewdir, d, depth);
		finalColor = vec4(bg.xyz, 1);
		//finalColor.xyz = mix(finalColor.xyz, sky.xyz, depth);
		//finalColor.xyz = mix(finalColor.xyz, fract(wpos.xyz / 32), 0.5);
		//finalColor.xyz = mix(finalColor.xyz, vec3(depth), 0.9);
	}
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

void main()
{
	vec3 camPos = vec3(inverse(matView)[3]);

	fragTexCoord = vertexTexCoord;
	fragColor = vertexColor;
	fragNormal = vertexNormal;
	fragCamPos = camPos;

#if INSTANCING
	mat4 tr = instanceTransform;
	mat4 mvpi = mvp * tr;
	vec3 worldPos = (matModel * tr * vec4(vertexPosition, 1.0)).xyz;
	gl_Position = fixZB(camPos, worldPos, mvpi * vec4(vertexPosition, 1.0));
#else
	vec3 worldPos = (matModel * vec4(vertexPosition, 1.0)).xyz;
	gl_Position = fixZB(camPos, worldPos, mvp * vec4(vertexPosition, 1.0));
#endif

	fragWorldPos = worldPos;
	fragFog = gl_Position.z / gl_Position.w;
}

#else //!VS

out vec4 finalColor;

uniform sampler2D texture0;
uniform vec4 colDiffuse;

vec3 light(vec3 normal, vec3 eyedir, vec3 lightDir, vec3 lightColor, vec3 ambientColor)
{
	vec3 halfDir = normalize(lightDir + eyedir);
	float diff = max(dot(normal, lightDir), 0.0);
	float spec = pow(max(dot(normal, halfDir), 0.0), 6.0);
	return mix(ambientColor, lightColor, diff) + lightColor * spec;
}

void main()
{
	vec4 texelColor = texture(texture0, fragTexCoord);
	finalColor = texelColor * colDiffuse * fragColor;
	if (finalColor.a < 0.1)
		discard;

	vec3 viewDir = normalize(fragWorldPos - fragCamPos);
	vec3 sunColor = vec3(1.0, 1.0, 1.0);
	vec3 shadowColor = vec3(0.4, 0.4, 0.4);
	vec3 lightDir = normalize(vec3(0.5, -1.0, 0.3));
	vec3 diffuseLight = light(fragNormal, viewDir, lightDir, sunColor, shadowColor);
	finalColor.xyz *= diffuseLight;
}

#endif

#endif

