// SPHERE FRAGMENT SHADER: MOTION BLUR
// This shader implements 2.5D motion blur (image space blur).
// Use the velocity variable to blur the sphere along the given direction.

varying vec3 light_dir;
varying float this_point_size;
varying vec2 velocity;
varying float vel_length;
//varying float scale;

void main (void)
{
	vec3 final_color;
	float alpha;
	vec4 in_color = gl_Color;
	vec2 xy;
	vec2 velocity_pp = vec2(velocity.y, -velocity.x);
	//float vel_length = 0.0;
	float scale = 1.0 + vel_length;
	xy.x = (gl_TexCoord[0].s - 0.5) * 2.0 * scale;
	xy.y = (gl_TexCoord[0].t - 0.5) * -2.0 * scale;
	
	float r = length(xy);
	float h = abs(dot(xy, velocity_pp)); // projected "height" along blur
	if ( (r > 1.00 + vel_length) || h > 1.0)
		discard; // outside the sphere and blur trail.
			
	float w = abs(dot(xy, velocity)); // projected "width" along blur
	float W = sqrt(1.0 -  h*h);
	alpha = 1.0 - (clamp((w - W), 0.0, 1.0)*(1.0/max(1e-7, vel_length))); // smooth alpha attenuation
			
	/* // Old attempt at temporal average, looks nice but very slow
	int samples = 2;
	if (r > 0.50 - vel_length*0.5)
	{
		//samples = clamp(0.125*this_point_size*vel_length, 2, 16);
		//samples = 3;
	}	
	vec2 bigxy_next = xy*2.0 + 0.5*velocity*vel_length;	
	float accum_light = 0, accum_alpha = 0;		
	vec3 accum_normal = vec3(0, 0, 0);
	float vel_step = vel_length / (samples - 1.0);
	for (int i = 0; i < samples; i++)
	{		
		float zsqr = length(bigxy_next);
		if (zsqr < 1.0)	
		{
			zsqr = zsqr*zsqr;
			float z = sqrt(1.0 - zsqr);		
			accum_normal += normalize(vec3(bigxy_next, z));
			accum_alpha += 1.0;
		}
		bigxy_next = bigxy_next - (velocity*vel_step);	
	}
	*/
	
	//xy = xy * (min(r, 0.9) / r);	
	float z = sqrt(1.0 - min(1.0 - (vel_length*0.5), r*r));
	float lighting = max(0.0, dot(normalize(vec3(xy, z)), light_dir));
	final_color = in_color.xyz * lighting;
	

	gl_FragColor = vec4( final_color, alpha);
	//gl_FragDepth = (backdepth-(radius*z)+1.0)/2.0;
}
