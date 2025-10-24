
// common functions...

float cloudH = 15.0;

vec3 unitToColor(vec3 v) {
	return normalize(v) * 0.5 + 0.5;
}

float sampleCloud(vec2 uv)
{
	vec2 p = fract(uv);
	float l = length(p - vec2(0.5));
	return smoothstep(0.4, 0.1, l);
}

float cloudTest(vec2 uv)
{
	float r = sampleCloud(uv / 300.0);
	r *= (0.9 + 0.1 * exp(-sampleCloud(uv / 180.0)));
	r *= (0.9 + 0.1 * exp(-sampleCloud(uv / 110.0)));
	r *= (0.9 + 0.1 * exp(-sampleCloud(uv / 70.0)));
	r *= (0.9 + 0.1 * exp(-sampleCloud(uv / 10.0)));
	return r;
}

vec3 baseSkyColor(vec3 dir) {
	float t = 0.5 * (dir.y + 1.0);
	return vec3(mix(vec3(1.0, 1.0, 1.0), vec3(0.5, 0.7, 1.0), t));
}

vec4 sceneSkyColor(float z, vec3 campos, vec3 dir, vec3 wpos) {
	float h = cloudH;
	if (campos.y > h && wpos.y < h && dir.y < 0.0) {
		vec3 proj = campos - (dir * ((campos.y - h) / dir.y));
		float nz = 1 - exp(wpos.y - h);
		return vec4(baseSkyColor(dir), max(z, nz * cloudTest(proj.xz)));
	}
	if (campos.y < h && wpos.y > h && dir.y > 0.0) {
		vec3 proj = campos - (dir * ((campos.y - h) / dir.y));
		float nz = 1 - exp(h - wpos.y);
		return vec4(baseSkyColor(dir), max(z, nz * cloudTest(proj.xz)));
	}
	return vec4(baseSkyColor(dir), z);
}

vec4 domeSkyColor(float z, vec3 campos, vec3 dir) {
	float h = cloudH;
	if (campos.y > h && dir.y < 0.0) {
		vec3 proj = campos - (dir * ((campos.y - h) / dir.y));
		float nz = 1;
		return vec4(baseSkyColor(dir), max(z, nz * cloudTest(proj.xz)));
	}
	if (campos.y < h && dir.y > 0.0) {
		vec3 proj = campos - (dir * ((campos.y - h) / dir.y));
		float nz = 1;
		return vec4(baseSkyColor(dir), max(z, nz * cloudTest(proj.xz)));
	}
	return vec4(baseSkyColor(dir), z);
}

