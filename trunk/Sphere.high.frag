// SPHERE FRAGMENT SHADER: FULL BLINN-PHONG LIGHTING
// This shader uses two "pow" calls to add a second (small) specular highlight

varying vec3 light_dir;
varying vec3 light_dir_2;

varying vec3 light_half;
varying vec3 light_half_2;

varying float backdepth;
varying float radius;

void main (void)
{
	vec4 in_color = gl_Color;
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
	float spec = pow(max(0.0, dot(normal, light_half)), 40.0);
	float spec_2 = pow(max(0.0, dot(normal,  light_half_2)), 40.0);
	vec3 final_color = (in_color.rgb * (color+color_2*0.2+normal.z*0.2)) + spec + (spec_2 * 0.2);

	//gl_FragColor = vec4(color, color, color, 1.0);		
	//gl_FragColor = vec4( final_color * step(2.0/this_point_size, 1.0 - r), in_color.a);
	gl_FragColor = vec4(final_color, in_color.a);
	gl_FragDepth = (backdepth-(radius*z)+1.0)/2.0;
}