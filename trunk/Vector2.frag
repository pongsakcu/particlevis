// SPHERE FRAGMENT SHADER: VELOCITY GLYPHS
// Old vector fragment shader, unused.

varying float backdepth;
varying float radius;

varying vec3 light_dir;
varying float this_point_size;
varying vec2 velocity;
//varying float vel_length;

void main (void)
{
	vec3 final_color;
	float alpha;
	vec4 in_color = gl_Color;
	vec2 xy;
	vec2 velocity_pp = vec2(velocity.y, -velocity.x);
	//float vel_length = 0.0;
	//float scale = 1.0 + vel_length;
	xy.x = (gl_TexCoord[0].s - 0.5) * 2.0;
	xy.y = (gl_TexCoord[0].t - 0.5) * -2.0;
	
	float r = length(xy);	
	if (r > 1.00)
		discard;	
	
	//float W = sqrt(1.0 - h*h);
	//alpha = 1.0 - (clamp((w - W), 0.0, 1.0)*(1.0/max(1e-7, vel_length)));

	float z = sqrt(1.0 - r*r);
	float lighting =  z*0.2 + max(0.0, dot(normalize(vec3(xy, z)), light_dir));
	final_color = in_color.xyz * lighting;

	float w = dot(xy, velocity);
	//float h = abs(dot(xy, velocity_pp)) * (w+1.0);
	float h = abs(dot(xy, velocity_pp)) + step(0, -w);
	
	// add 'arrowhead' glyph	
	final_color = final_color - (1.0-step(0.2, h+r*0.2))*0.72;
	//final_color = final_color - (1.0-step(0.2, h))*0.64;

	gl_FragColor = vec4( final_color, in_color.a );
	gl_FragDepth = (backdepth-(radius*z)+1.0)/2.0;
}
