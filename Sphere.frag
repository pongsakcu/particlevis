//    SPHERE FRAGMENT SHADER: BASIC
//   "Floodlight" lighting scheme, uses radial distance, very fast.

void main (void)
{							
	vec2 xy;
	xy.x = (gl_TexCoord[0].s - 0.5) * 2.0;
	xy.y = (gl_TexCoord[0].t - 0.5) * -2.0;
	
	float r = length(xy);
	if (r > 1.00)
		discard;
		
	vec3 normal = vec3(xy, sqrt(1.0 - r*r));		
	vec4 in_color = gl_Color;
	//vec3 final_color = (in_color.xyz * (1.0 - r+0.1));
	vec3 final_color = (in_color.xyz * (1.0 - r*r));

	gl_FragColor = vec4(final_color, in_color.a);
}