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

#if VS
float CoeffZ = -1.0 / (1 << 1);

vec4 fixZB(vec4 p) {
	float nz = 1 - exp(CoeffZ * (p.z / p.w));
	p.z = nz * p.w;
	return p;
}
#endif

#if !VS && PP
vec3 makeWPos(vec3 cam, vec3 v, float d)
{
	return cam + d * v;
}
#endif

#if PP

//================================================== 
// post process
//================================================== 

VAR vec3 fragCamPos;
VAR vec3 fragViewdir;
VAR vec2 fragTexCoord;

#if VS

in vec3 vertexPosition;

uniform mat4 matModel;
uniform mat4 mvp;
uniform vec3 camInfo;

void main()
{
	vec3 camPos = camInfo.xyz;
	vec3 worldPos = (matModel * vec4(vertexPosition, 0.0)).xyz;

	vec4 p = mvp * vec4(vertexPosition, 0.0);
	p.z = p.w;
	gl_Position = p;

	fragCamPos = camPos;
	fragViewdir = normalize(worldPos);
	fragTexCoord = 0.5 * (vertexPosition.xy + 1);
}

#else //!VS ____________________________________________________________________________________________________ 

out vec4 finalColor;

uniform sampler2D texture0;

#define CLOUD 1
#include "common.glsl"

vec3 vignetting(vec3 c, vec2 uv)
{
	uv = abs(uv * 2 - 1);
	float pv = 6;
	float bv = 0.5;
	float r = (1 - bv) + bv * (1 - pow(uv.x, pv)) * (1 - pow(uv.y, pv));
	return r * c;
}

void main()
{
	vec4 bg = texture(texture0, fragTexCoord);
	float depth = texture(depthMap, fragTexCoord).r;
	vec3 lightDir = normalize(vec3(0.5, -1.0, 0.3));
	vec3 vd = normalize(fragViewdir);
	if (depth > 0.0) {
		finalColor.xyz = domeSkyColor(lightDir, 1.0, fragCamPos, vd);
	} else {
		depth = -depth;
		vec3 wpos = makeWPos(fragCamPos, vd, depth);
		float fog = 1 - exp(-0.001 * depth);
		finalColor.xyz = sceneSkyColor(lightDir, bg.xyz, fog, fragCamPos, vd, wpos);
	}
	finalColor = vec4(vignetting(finalColor.xyz, fragTexCoord), 1);
}

#endif

#else

//================================================== 
// 3D pass
//================================================== 

VAR vec3 fragWorldPos;
VAR vec3 fragCamPos;
VAR vec3 fragNormal;
VAR vec2 fragTexCoord;
VAR vec4 fragColor;

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
uniform vec3 camInfo;

void main()
{
	vec3 camPos = camInfo;

#if INSTANCING
	mat4 tr = instanceTransform;
	mat4 mvpi = mvp * tr;
	vec3 worldPos = (matModel * tr * vec4(vertexPosition, 1.0)).xyz;
	gl_Position = fixZB(mvpi * vec4(vertexPosition, 1.0));
#else
	vec4 bwp = matModel * vec4(vertexPosition, 1.0);
	vec3 worldPos = bwp.xyz / bwp.w;
	gl_Position = fixZB(mvp * vec4(vertexPosition, 1.0));
#endif

	fragTexCoord = vertexTexCoord;
	fragColor = vertexColor;
	fragNormal = vertexNormal;
	fragCamPos = camPos;
	fragWorldPos = camPos + worldPos;
}

#else //!VS ____________________________________________________________________________________________________ 

#if MODE != 1
#define WATER 1
#endif

#include "common.glsl"

layout(location = 0) out vec4 finalColor;
layout(location = 1) out vec4 finalDepth;

uniform sampler2D texture0;
uniform sampler2D cloudDensity;

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
#if MODE == 3
	if (texelColor.a == 0)
		discard;
#endif

	float viewDepth = length(fragWorldPos - fragCamPos);
	vec3 viewDir = normalize(fragWorldPos - fragCamPos);
	vec3 sunColor = vec3(1.0, 1.0, 1.0);
	vec3 shadowColor = vec3(0.4, 0.4, 0.4);
	vec3 lightDir = normalize(vec3(0.5, -0.2, 0.3));
	vec3 diffuseLight = light(fragNormal, viewDir, lightDir, sunColor, shadowColor);

	vec4 baseColor = texelColor * colDiffuse * fragColor;
#if MODE == 1
	if (baseColor.a < 0.1)
		discard;
#endif

#if MODE == 1
	finalColor = baseColor;
	finalColor.xyz *= diffuseLight;
#endif
#if MODE == 2
	int idx = int(baseColor.a * 255.0 + 0.5);
	if (idx == 0x5 * 16) {
		vec4 details = texture(cloudDensity, fragWorldPos.xz / 256);
		finalColor.xyz = baseColor.xyz * vec3(0.8 + 0.4 * details.y);
		finalColor.xyz *= diffuseLight;
	} else if (idx == 0xA * 16) {
#if 0
		finalColor.xyz = getWorldSea(fragCamPos, viewDir, -lightDir);
#else
		vec4 details = texture(cloudDensity, fragWorldPos.xz / 256);
		finalColor.xyz = baseColor.xyz * vec3(0.8 + 0.4 * details.y);
		finalColor.xyz *= diffuseLight;
#endif
	} else {
		finalColor = vec4(baseColor.xyz, 1);
		finalColor.xyz *= diffuseLight;
	}
#endif
#if MODE == 3
	finalColor = vec4(baseColor.xyz, 1);
	finalColor.xyz *= diffuseLight;
#endif

#if MODE == 2
	finalColor.a = 1;
#endif

	finalDepth = vec4(-viewDepth, 0, 0, 1);
}

#endif

#endif

