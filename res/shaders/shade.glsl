vec4 apply_light(vec3 normal, float time) {
    vec3 light = vec3(.5*(cos(time)/2.0+.5), time, .8*(cos(time)/2.0+.5));
    float b = min(max(0.3, dot(normalize(light), normal) * 2.0), 1.0);
    return vec4(b, b, b, 1.0);
}