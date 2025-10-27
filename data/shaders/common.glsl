
// common functions...

uniform sampler2D cloudDensity;

vec3 rgb(float r, float g, float b)
{
	return vec3(r, g, b);
}

vec3 colSun = rgb(1.0, 1.0, 1.0);
vec3 colTop = rgb(0.5, 0.7, 1.0);
vec3 colFar = rgb(0.3, 0.5, 0.8);
float cloudH = 250.0;
float dH = 30.0;

vec3 unitToColor(vec3 v) {
	return normalize(v) * 0.5 + 0.5;
}

float sampleCloud(vec2 uv)
{
	vec2 p = uv;
	vec4 t0 = texture(cloudDensity, p);
	vec4 t1 = texture(cloudDensity, 170 * p);
	vec4 t2 = texture(cloudDensity, 230 * p);
	return (t0.r * (1 - 0.3 * t1.g)) * (1 - 0.3 * t2.g);
}

float cloudTest(vec2 uv)
{
	float r = sampleCloud(uv / 4251.0);
	return r;
}

vec3 baseSkyColor(vec3 dir, vec3 light) {
	float ty = 0.5 * (dir.y + 1.0);
	float tl = pow(0.5 * (dot(dir, -light) + 1.0), 4);
	vec3 cy = vec3(mix(colFar, colTop, ty));
	vec3 cl = vec3(mix(cy, colSun, tl));
	return cl;
}

vec3 traceClouds(vec3 sky, vec3 base, float hb, float ht, vec3 cam, vec3 from, vec3 to, vec3 dir)
{
	float stride = 0.05;
	float c = 1 - exp(-0.011 * length(to - from));
	vec3 col0 = sky;
	vec3 col1 = mix(sky, colSun, exp(-0.0001 * length(from - cam)));
	for (int i = 0; i < 40; i++) {
		float r0 = float(i) / 15.0;
		vec3 pos = mix(to, from, r0);
		float r1 = (pos.y - hb) / (ht - hb);
		float th = 0.1 + 0.8 * pow(r1 * 2 - 1, 2);
		float d = c *  smoothstep(th - stride, th + stride, cloudTest(pos.xz));
		base = mix(base, mix(col0, col1, r1), d);
	}
	return base;
}

vec3 makeSkyColor(vec3 sky, vec3 base, vec3 campos, vec3 dir, vec3 wpos) {
	float ht = cloudH + dH;
	float hb = cloudH - dH;
	if (dir.y < 0.0 && (campos.y < hb || wpos.y > ht))
		return base;
	if (dir.y > 0.0 && (campos.y > ht || wpos.y < hb))
		return base;
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
	if ((to.y - wpos.y) * (wpos.y - from.y) > 0.0)
		to = wpos;
	return traceClouds(sky, base, hb, ht, campos, from, to, dir);
}

vec3 sceneSkyColor(vec3 light, vec3 base, float z, vec3 campos, vec3 dir, vec3 wpos) {
	vec3 sky = baseSkyColor(dir, light);
	vec4 dr = vec4(sky, z);
	base = mix(base, dr.xyz, z);
	return makeSkyColor(sky, base, campos, dir, wpos);
}

vec3 domeSkyColor(vec3 light, float z, vec3 campos, vec3 dir) {
	vec3 sky = baseSkyColor(dir, light);
	float ht = cloudH + dH;
	float hb = cloudH - dH;
	if (dir.y < 0.0 && campos.y < hb)
		return sky;
	if (dir.y > 0.0 && campos.y > ht)
		return sky;
	vec3 from, to;
	if (dir.y < 0) {
		from = campos - (dir * ((campos.y - ht) / dir.y));
		to = campos - (dir * ((campos.y - hb) / dir.y));
	} else {
		from = campos - (dir * ((campos.y - hb) / dir.y));
		to = campos - (dir * ((campos.y - ht) / dir.y));
	}
	return traceClouds(sky, sky, hb, ht, campos, from, to, dir);
}

