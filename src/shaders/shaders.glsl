
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
void main() {
	color = vcolor;
	n = (world * vec4(normal.xyz, 0)).xyz;
	l = lightDir;
	uv = texcoord.xy;
	gl_Position = (proj * view * world) * position;
}
@end

@fs model_fs
uniform sampler2D bitmap;
in vec4 color;
in vec3 n;
in vec3 l;
in vec2 uv;
out vec4 frag_color;
void main() {
	vec4 tc = texture(bitmap, uv);
	if (tc.a < 0.5)
		discard;
	float ndotl = 0.5 * (1 + dot(n, l));
	frag_color = vec4(ndotl.xxx, 1) * tc * color;
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
void main() {
	color = vcolor;
	uv = texcoord.xy;
	gl_Position = (proj * view * world) * position;
}
@end

@fs ground_fs
uniform sampler2D bitmap;
in vec4 color;
in vec2 uv;
out vec4 frag_color;
void main() {
	vec4 tc = texture(bitmap, uv);
	if (tc.a < 0.5)
		discard;
	frag_color = tc * vec4(color.rgb, 1);
}
@end

@program ground ground_vs ground_fs
