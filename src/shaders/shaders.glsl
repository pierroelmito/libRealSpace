
// common

@block fog
vec4 computeFog(vec4 v, float depth)
{
	vec3 fogColor = vec3(1, 1, 1);
	float intensity = exp(0.00005 * depth);
	return vec4(mix(fogColor, v.rgb, intensity), v.a);
}
@end

// fullscreen bitmap

@vs bitmap_vs
layout(location=0) in vec4 position;
out vec2 uv;
void main() {
	uv = 0.5 * (vec2(1, -1) * position.xy + 1);
	gl_Position = position;
}
@end

@fs bitmap_fs
uniform sampler2D bitmap;
in vec2 uv;
out vec4 frag_color;
void main() {
	vec4 tc = texture(bitmap, uv);
	frag_color = vec4(tc.xyz, 1);
}
@end

@program bitmap bitmap_vs bitmap_fs

// fullscreen sky

@vs sky_vs
uniform sky_vs_params {
	mat4 view;
	mat4 proj;
};
layout(location=0) in vec4 position;
out vec3 eyedir;
void main() {
	vec3 x = vec3(view[0][0], view[1][0], view[2][0]);
	vec3 y = vec3(view[0][1], view[1][1], view[2][1]);
	vec3 z = vec3(view[0][2], view[1][2], view[2][2]);
	eyedir = z + ((-position.x / proj[0][0]) * x) + ((-position.y / proj[1][1]) * y);
	gl_Position = position;
}
@end

@fs sky_fs
in vec3 eyedir;
out vec4 frag_color;
void main() {
	vec3 e = normalize(eyedir);
	//frag_color = vec4(fract(10 * (0.5 * (1 + e))), 1);
	//frag_color = vec4(mix(vec3(0, 0, 1), vec3(0, 1, 1), fract(10 * e.y)), 1);
	frag_color = vec4(mix(vec3(0, 1, 1), vec3(0, 0, 1), -e.y), 1);
}
@end

@program sky sky_vs sky_fs

// model rendering

@vs model_vs
uniform model_vs_params {
	mat4 view;
	mat4 proj;
	mat4 world;
	vec3 camPos;
	vec3 lightDir;
};
layout(location=0) in vec4 position;
layout(location=1) in vec4 normal;
layout(location=2) in vec4 texcoord;
layout(location=3) in vec4 vcolor;
out vec4 color;
out vec3 n;
out vec3 l;
out vec2 uv;
out float depth;
void main() {
	color = vcolor;
	n = normalize((world * vec4(normal.xyz, 0)).xyz);
	l = lightDir;
	uv = texcoord.xy;
	vec4 tmp = (view * world) * position;
	depth = tmp.z / tmp.w;
	gl_Position = (proj * view * world) * position;
}
@end

@fs model_fs
@include_block fog

uniform sampler2D bitmap;
in vec4 color;
in vec3 n;
in vec3 l;
in vec2 uv;
in float depth;
out vec4 frag_color;
void main() {
	vec4 tc = texture(bitmap, uv);
	if (tc.a * color.a == 0.0)
		discard;
	float ndotl = 0.5 * (1 + dot(n, l));
	frag_color = computeFog(vec4(ndotl.xxx, 1) * tc * color, depth);
	//frag_color.xyz = 0.001 * frag_color.xyz + 0.5 * (n.xzy + 1);
}
@end

@program model model_vs model_fs

// ground rendering

@vs ground_vs
uniform ground_vs_params {
	mat4 view;
	mat4 proj;
	mat4 world;
};
layout(location=0) in vec4 position;
layout(location=1) in vec4 texcoord;
layout(location=2) in vec4 vcolor;
out vec4 color;
out vec2 uv;
out float depth;
void main() {
	color = vcolor;
	uv = texcoord.xy;
	vec4 tmp = (view * world) * position;
	depth = tmp.z / tmp.w;
	gl_Position = (proj * view * world) * position;
}
@end

@fs ground_fs
@include_block fog

uniform sampler2D bitmap;
in vec4 color;
in vec2 uv;
in float depth;
out vec4 frag_color;

void main() {
	vec4 tc = texture(bitmap, uv);
	if (tc.a == 0.0)
		discard;
	//int idx = int(color.a * tc.a * 255.0);
	int idx = int(color.a * 255.0);
	if (idx == 0x7) {
		frag_color = computeFog(tc * vec4(1, 0, 0, 1), depth); // ground!!
	} else if (idx == 0x5) {
		frag_color = computeFog(tc * vec4(0, 1, 0, 1), depth); // grass!!
	} else if (idx == 0xA) {
		frag_color = computeFog(tc * vec4(0, 0, 1, 1), depth); // water!!
	} else {
		frag_color = computeFog(tc * vec4(color.rgb, 1), depth);
	}
	frag_color = computeFog(tc * vec4(color.rgb, 1), depth);
}
@end

@program ground ground_vs ground_fs
