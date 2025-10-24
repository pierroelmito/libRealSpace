
// common functions...

vec3 unitToColor(vec3 v) {
	return normalize(v) * 0.5 + 0.5;
}

vec3 skyColor(float z, vec3 campos, vec3 dir) {
	float t = 0.5 * (dir.y + 1.0);
	return mix(vec3(1.0, 1.0, 1.0), vec3(0.5, 0.7, 1.0), t);
}
