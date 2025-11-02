
// common functions...

uniform sampler2D cloudDensity;
uniform sampler2D depthMap;

vec3 rgb(float r, float g, float b)
{
	return vec3(r, g, b);
}

vec3 colSun = rgb(1.0, 1.0, 1.0);
vec3 colTop = rgb(0.5, 0.7, 1.0);
vec3 colFar = rgb(0.3, 0.5, 0.8);
float cloudH = 250.0;
float dH = 30.0;
float coeffUV = 7251.0;

vec3 unitToColor(vec3 v) {
	return normalize(v) * 0.5 + 0.5;
}

float sampleCloud1(vec2 uv)
{
	vec2 p = uv;
	vec4 t0 = texture(cloudDensity, p);
	return t0.r;
}

float cloudTest1(vec2 uv)
{
	float r = sampleCloud1(uv / coeffUV);
	return r;
}

float sampleCloud2(vec3 uv)
{
	float c = 10.0;
	vec3 p = uv;
	vec4 t0 = texture(cloudDensity, p.xz);
	vec4 t1 = texture(cloudDensity, 4 * c * p.xy) * texture(cloudDensity, 3 * c * p.yz);
	vec4 t2 = texture(cloudDensity, 3 * c * p.yx) * texture(cloudDensity, 4 * c * p.zy);
	return (t0.r * (1 - 0.2 * t1.g)) * (1 - 0.2 * t2.g);
}

float cloudTest2(vec3 uv)
{
	float r = sampleCloud2(uv / coeffUV);
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
	float stride = 0.09;
	float c = 1 - exp(-0.011 * length(to - from));
	vec3 col0 = sky;
	vec3 col1 = mix(sky, colSun, exp(-0.0001 * length(from - cam)));

	int steps = 22;
	for (int i = 0; i < steps; i++) {
		float r0 = float(i) / float(steps - 1);
		vec3 pos = mix(to, from, r0);
		float cloud = cloudTest2(pos);
		float r1 = (pos.y - hb) / (ht - hb);
		float th = 0.5 + 0.8 * (pow(r1 * 2 - 1, 2) - 0.5);
		float d = c *  smoothstep(th - stride, th + stride, cloud);
		base = mix(base, mix(col0, col1, r1), d);
	}

	return base;
}

vec3 traceClouds3(vec3 sky, vec3 base, float hb, float ht, vec3 cam, vec3 from, vec3 to, vec3 dir)
{
	vec3 pos = from;
	vec3 delta = to - from;
	vec3 dp = delta / abs(delta.y);
	for (int i = 0; i < 16; i++) {
		float c = 1 - cloudTest1(pos.xz);
		pos = pos + c * dp / 16;
	}
	return traceClouds(sky, base, hb, ht, cam, pos, mix(from, to, 0.7), dir);
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
	return traceClouds3(sky, base, hb, ht, campos, from, to, dir);
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
	return traceClouds3(sky, sky, hb, ht, campos, from, to, dir);
}

