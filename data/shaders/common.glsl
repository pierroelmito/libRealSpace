
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

#if 1

vec4 traceClouds(float hb, float ht, vec3 from, vec3 to, vec3 dir)
{
	float res = 0.0;
	float c = 1 - exp(-abs((ht - hb) / dir.y));
	for (int i = 0; i < 32; i++) {
		float r0 = float(i) / 15.0;
		vec3 pos = mix(to, from, r0);
		float r1 = (pos.y - hb) / (ht - hb);
		float th = 0.1 + 0.8 * pow(r1 * 2 - 1, 2);
		float d = smoothstep(th - 0.2, th + 0.2, cloudTest(pos.xz));
		res += c * d / 16.0;
	}
	return vec4(min(1, res), 0, 0, 0);
	//return 0.5 * vec4(cloudTest(from.xz), 0, 0, 1) + 0.5 * vec4(cloudTest(to.xz), 0, 0, 1);
}

vec4 makeSkyColor(vec4 sky, vec3 campos, vec3 dir, vec3 wpos) {
	float ht = cloudH + 8.0;
	float hb = cloudH - 8.0;
	if (dir.y < 0.0 && (campos.y < hb || wpos.y > ht))
		return sky;
	if (dir.y > 0.0 && (campos.y > ht || wpos.y < hb))
		return sky;
	vec3 from, to;
	if (dir.y < 0) {
		from = campos - (dir * ((campos.y - ht) / dir.y));
		to = campos - (dir * ((campos.y - hb) / dir.y));
	} else {
		from = campos - (dir * ((campos.y - hb) / dir.y));
		to = campos - (dir * ((campos.y - ht) / dir.y));
	}
	//if ((campos.y - to.y) * (campos.y - from.y) < 0.0)
	//	from = campos;
	if ((wpos.y - to.y) * (wpos.y - from.y) < 0.0)
		to = wpos;
	return vec4(vec3(1, 1, 0), traceClouds(hb, ht, from, to, dir).x);
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
