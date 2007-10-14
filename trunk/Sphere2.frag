// SPHERE FRAGMENT SHADER: BLINN-PHONG LIGHTING
// This is the default scheme.  Evaluate the normal per-pixel, phong-shader style.
// The exponent is currently fixed.

varying vec3 light_dir;
varying vec3 light_dir_2;

varying vec3 light_half;

varying float backdepth;
varying float radius;

void main (void)
{
	vec2 xy;
	xy.x = (gl_TexCoord[0].s - 0.5) * 2.0;
	xy.y = (gl_TexCoord[0].t - 0.5) * -2.0;
	
	float r = length(xy);
	if (r > 1.00)
		discard;
		
	float z = sqrt(1.0 - r*r);
	vec3 normal = vec3(xy, z);

	float color = max(0.0, dot(normal, light_dir));
	float color_2 = max(0.0, dot(normal, light_dir_2));
	float spec = pow(max(0.0, dot(normal, light_half)), 50.0);
	vec3 final_color = gl_Color.rgb * (color+color_2*0.2+normal.z*0.1) + spec;
		
	gl_FragColor = vec4( final_color, gl_Color.a);
	gl_FragDepth = (backdepth-(radius*z)+1.0)/2.0;
}