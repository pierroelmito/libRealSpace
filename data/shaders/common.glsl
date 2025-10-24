
// common functions...

uniform sampler2D cloudDensity;

float cloudH = 15.0;

vec3 unitToColor(vec3 v) {
	return normalize(v) * 0.5 + 0.5;
}

float sampleCloud(vec2 uv)
{
	vec2 p = uv;
	vec4 texelColor = texture(cloudDensity, p);
	return texelColor.r;
}

float cloudTest(vec2 uv)
{
	float r = sampleCloud(uv / 1000.0);
	return r;
}

vec3 baseSkyColor(vec3 dir) {
	float t = 0.5 * (dir.y + 1.0);
	return vec3(mix(vec3(1.0, 1.0, 1.0), vec3(0.5, 0.7, 1.0), t));
}

#if 0

vec4 makeSkyColor(vec4 sky, vec3 campos, vec3 dir, vec3 wpos) {
	float ht = cloudH + 1.0;
	float hb = cloudH - 1.0;
	if (campos.y < hb && dir.y < 0.0)
		return sky;
	if (campos.y > ht && dir.y > 0.0)
		return sky;
	return vec4(1, 0, 0, 1);
}

vec4 sceneSkyColor(float z, vec3 campos, vec3 dir, vec3 wpos) {
	vec4 dr = vec4(baseSkyColor(dir), z);
	return makeSkyColor(dr, campos, dir, wpos);
}

vec4 domeSkyColor(float z, vec3 campos, vec3 dir) {
	vec4 dr = vec4(baseSkyColor(dir), z);
	return dr;
}

#else

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

#endif
