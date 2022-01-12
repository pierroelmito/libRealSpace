
@ctype mat4 hmm_mat4
@ctype vec4 hmm_vec4
@ctype vec3 hmm_vec3
@ctype vec2 hmm_vec2

// -----------------------------------------------------------
// common
// -----------------------------------------------------------

@block fog
vec3 computeFog(vec3 v, float depth)
{
	float intensity = exp(thickNess * depth);
	return mix(fogColor, v.rgb, intensity * intensity);
}
@end

@block lighting
vec3 computeLight(vec3 albedo, vec3 n, vec3 l, vec3 eyeDir, float specParam)
{
	vec3 h = normalize(l - eyeDir);
	float ndotl = dot(l, -n);
	float ndoth = max(0, dot(h, -n));
	ndoth = ndoth * ndoth * ndoth * ndoth * ndoth * ndoth * ndoth * ndoth;
	float diffuse = max(ndotl, 0.2);
	vec3 result = diffuse * albedo.xyz + specParam * ndoth.xxx;
	//result = 0.001 * result + specParam * ndoth.xxx;
	return result;
}
@end

// -----------------------------------------------------------
// fullscreen bitmap
// -----------------------------------------------------------

@vs bitmap_vs

layout(location=0) in vec4 position;

out vec2 uv;

void main() {
	uv = 0.5 * (vec2(1, -1) * position.xy + 1);
	gl_Position = position;
}

@end
@fs bitmap_fs

uniform sampler2D fs_bitmap;

in vec2 uv;

out vec4 frag_color;

void main() {
	vec4 tc = texture(fs_bitmap, uv);
	frag_color = vec4(tc.xyz, 1);
}

@end
@program bitmap bitmap_vs bitmap_fs

// -----------------------------------------------------------
// fullscreen sky
// -----------------------------------------------------------

@vs sky_vs

uniform sky_vs_params {
	mat4 view;
	mat4 proj;
	vec3 plightdir;
};

layout(location=0) in vec4 position;

out vec3 eyedir;
out vec3 lightdir;

void main() {
	vec3 x = vec3(view[0][0], view[1][0], view[2][0]);
	vec3 y = vec3(view[0][1], view[1][1], view[2][1]);
	vec3 z = vec3(view[0][2], view[1][2], view[2][2]);
	eyedir = z + ((-position.x / proj[0][0]) * x) + ((-position.y / proj[1][1]) * y);
	lightdir = plightdir;
	gl_Position = position;
}

@end
@fs sky_fs

uniform sampler2D skydome;

uniform sky_fs_params {
	vec3 colUp;
	vec3 colBot;
	vec3 colLight;
};

in vec3 eyedir;
in vec3 lightdir;

out vec4 frag_color;

void main() {
	vec3 e = normalize(eyedir + vec3(0, -0.02, 0));
	float ey = 1 + e.y;
	vec2 tmp = (e.xz / (1 - e.y * e.y)) * ey;
	//vec2 tmp = e.xz;
	vec4 dome = texture(skydome, 0.5 * (0.99f * tmp + 1));
	float edotl = 0.5 * (1 + dot(-lightdir, e));
	edotl = edotl * edotl * edotl * edotl * edotl * edotl;
	edotl = 0.5 * (edotl + dome.a);
	//float h = 0.5 * (1 - e.y);
	float h = abs(e.y);
	vec3 hcolor = mix(colBot, colUp, h);
	frag_color = vec4(mix(hcolor, colLight, edotl), 1);
	frag_color.xyz = mix(frag_color.xyz, colLight, dome.a);
	//frag_color.xyz = 0.001f * frag_color.xyz + dome.aaa;
}

@end
@program sky sky_vs sky_fs

// -----------------------------------------------------------
// model rendering
// -----------------------------------------------------------

@vs model_vs

uniform model_vs_global_params {
	mat4 view;
	mat4 proj;
	vec3 pcampos;
	vec3 lightDir;
};

uniform model_vs_instance_params {
	mat4 world;
};

layout(location=0) in vec4 position;
layout(location=1) in vec4 normal;
layout(location=2) in vec4 texcoord;
layout(location=3) in vec4 vcolor;

out vec4 color;
out vec3 n;
out vec3 l;
out vec3 worldpos;
out vec3 campos;
out vec2 uv;
out float depth;

void main() {
	color = vcolor;
	n = normalize((world * vec4(normal.xyz, 0)).xyz);
	l = lightDir;
	uv = texcoord.xy;
	vec4 tmp = (view * world) * position;
	depth = tmp.z / tmp.w;
	worldpos = (world * position).xyz;
	campos = pcampos;
	gl_Position = (proj * view * world) * position;
}

@end
@fs model_fs

uniform model_fog_params {
	vec3 fogColor;
	float thickNess;
};

@include_block fog
@include_block lighting

uniform sampler2D model_bitmap;

in vec4 color;
in vec3 n;
in vec3 l;
in vec3 worldpos;
in vec3 campos;
in vec2 uv;

in float depth;

out vec4 frag_color;

void main() {
	vec4 tc = texture(model_bitmap, uv);
	if (tc.a * color.a < 0.5)
		discard;

	frag_color = vec4(color.xyz * tc.xyz, 1);

	frag_color.xyz = computeLight(frag_color.xyz, -normalize(n), l, normalize(worldpos - campos), 0.3);
	frag_color.xyz = computeFog(frag_color.xyz, depth);
}

@end
@program model model_vs model_fs

// -----------------------------------------------------------
// ground rendering
// -----------------------------------------------------------

@vs ground_vs

uniform ground_vs_params {
	mat4 view;
	mat4 proj;
	mat4 world;
	vec3 pcampos;
	vec3 plightdir;
	float gtime;
};

layout(location=0) in vec4 position;
layout(location=1) in vec4 normal;
layout(location=2) in vec4 texcoord;
layout(location=3) in vec4 vcolor;

out vec4 color;
out vec3 worldpos;
out vec3 campos;
out vec3 lightdir;
out vec3 n;
out vec3 uv;
out float depth;

void main() {
	color = vcolor;
	n = normalize((world * vec4(normal.xyz, 0)).xyz);
	uv = vec3(texcoord.xy, gtime);
	vec4 tmp = (view * world) * position;
	depth = tmp.z / tmp.w;
	worldpos = (world * position).xyz;
	campos = pcampos;
	lightdir = plightdir;
	gl_Position = (proj * view * world) * position;
}

@end
@fs ground_fs

uniform ground_fog_params {
	vec3 fogColor;
	float thickNess;
};

@include_block fog
@include_block lighting

uniform sampler2D ground_bitmap;
uniform sampler2D water;

in vec4 color;
in vec3 worldpos;
in vec3 campos;
in vec3 lightdir;
in vec3 n;
in vec3 uv;
in float depth;

out vec4 frag_color;

vec4 sampleWater(float cf, float sc, vec2 dir)
{
	vec2 uv = sc * (0.002 * worldpos.xz + cf * 0.05 * dir);
	vec4 wc = texture(water, uv);
	return wc;
}

float computeWater(vec3 wpos, float cf, float sc)
{
	vec4 wc0 = sampleWater(cf, sc, vec2(1, 0.1));
	vec4 wc1 = sampleWater(cf, sc, vec2(0, 0.76));
	vec4 wc2 = sampleWater(cf, sc, vec2(0, -0.65));
	vec4 wc3 = sampleWater(cf, sc, vec2(-0.2, -0.05));
	vec4 wc = (wc0 + wc1 + wc2 + wc3) / 4;
	float ws = wc.r * wc.r * wc.r * wc.r;
	//return ws > 0.06 ? 1.0 : 0.0;
	return ws;
}

void main() {
	vec4 tc = texture(ground_bitmap, uv.xy);
	if (tc.a == 0.0)
		discard;

	float spec = 0.1;

	//int idx = int(color.a * tc.a * 255.0);
	int idx = int(color.a * 255.0);
	if (idx == 0x7) {
		// ground!!
		frag_color = tc * vec4(color.rgb, 1);
	} else if (idx == 0x5) {
		// grass!!
		vec4 gc0 = 0.1 * (2 * texture(water, 0.005 * worldpos.xz) - 1);
		vec4 gc1 = 0.1 * (2 * texture(water, 0.006 * worldpos.zx) - 1);
		vec4 gc2 = 0.2 * (2 * texture(water, 0.0007 * worldpos.xz) - 1);
		vec4 gc3 = 0.2 * (2 * texture(water, 0.0008 * worldpos.zx) - 1);
		vec4 gc = gc0 + gc1  + gc2 + gc3;
		//frag_color = computeFog(vec4(tc.xyz * color.xyz + gc.xyz, 1) + gc.xyz, depth);
		frag_color = tc * vec4(color.rgb + 0.5 * gc.rgb, 1);
	} else if (idx == 0xA) {
		// water!!
		float ws = 0.9 * computeWater(worldpos, 0.3 * uv.z, 1.0) + 0.6 * computeWater(worldpos, 0.1 * uv.z, 5.0);
		vec3 wcolor1 = color.rgb * 1.2f;
		vec3 wcolor0 = color.rgb * 0.8f;
		float tmp = 6 * ws;
		spec = tmp * tmp * tmp;
		frag_color = tc * vec4(mix(wcolor0, wcolor1, 10 * ws), 1);
	} else {
		frag_color = tc * vec4(color.rgb, 1);
	}

	// lighting + fog
	//vec3 normal = normalize(n);
	//float ndotl = dot(lightdir, -normal);
	//float diffuse = max(ndotl, 0.5);
	frag_color.xyz = computeLight(frag_color.xyz, normalize(n), lightdir, normalize(worldpos - campos), spec);
	frag_color.xyz = computeFog(frag_color.xyz, depth);
}

@end
@program ground ground_vs ground_fs
