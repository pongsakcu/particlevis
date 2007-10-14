This is the CVS folder for ParticleVis.  By checking out all the source files here, you should have an up to date (but uncompiled) copy of PVis.

TODO

-Cullface disabled after inverted particles: disrupts transparency rendering
-Multiple file input?
-Color dialogs for marked particles etc.
-Implement renderers
	-vectorsets?  probably separate.
	-surface maps
-Testing testing testing


/* CODE DEBRIS */
	//glCopyTexSubImage2D(GL_TEXTURE_RECTANGLE_NV, 0, 0, 0, 0, 0, width, height);
	//glCopyTexSubImage2D(GL_TEXTURE_2D, 0, 0, 0, 0, 0, width, height);
	//glCopyTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT24, 0, 0, width, height, 0);
		
	//glUseProgramObjectARB(peel_program);
	//glBindTexture(GL_TEXTURE_RECTANGLE_NV, depth_tex);
	//glBindTexture(GL_TEXTURE_2D, depth_tex);

	/*else // last pass, render to screen!
	{				
		glUseProgramObjectARB(GL_NONE);
		frontTex->EnableTextureTarget();
		frontTex->BindDepth();
		glDisable(GL_DEPTH_TEST);
		glDisable(GL_CULL_FACE);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glColor4f(1, 1, 1, 1);
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		glOrtho(-1, 1, -1, 1, -1, 1);
		glMatrixMode(GL_MODELVIEW);
		glLoadIdentity();
		glNormal3f(0, 0, 1);
		glBegin(GL_QUADS);
			glTexCoord2f(0,0);
			glVertex3f(-1, -1, 0);
			glTexCoord2f(0,1);
			glVertex3f(-1,  1, 0);
			glTexCoord2f(1,1);
			glVertex3f( 1,  1, 0);
			glTexCoord2f(1,0);
			glVertex3f( 1, -1, 0);
		glEnd();
		frontTex->DisableTextureTarget();
		glEnable(GL_CULL_FACE);
	}*/
		//glEnable(GL_FRAGMENT_PROGRAM_ARB);	
		//glBindProgramARB(GL_FRAGMENT_PROGRAM_ARB, peel_program);
		//glEnable(GL_FRAGMENT_SHADER_ARB);


			/*Frame current = data_store->GetFrame(currentframe);
			triple * pos = current.pos;
			triple * d_pos = current.d_pos;
			triple * d_angle = current.d_angle;
			if (useVelocity)
				RenderVectors(POSVELOCITY, particle_count, pos, d_pos, vectorBuf);		
			if (useRotVelocity)		
				RenderVectors(ROTVELOCITY, particle_count, pos, d_angle, vectorBuf);
			// draw normal vectors
			if (Particle::normalVector)
				RenderVectors(NORMALVEC, particle_count, pos, d_pos, vectorBuf);
			// draw paths for marked particles
			for (int p = 0; p < particle_count; p++)
				particles[p]->DrawPath(currentframe, vectorBuf);*/
				

void ParticleSet::RenderVectors(const int type, const int count, triple * pos, triple * d_pos, GLVertexBuffer& vBuf)
{
	// this function and DrawPath both add their lines/colors to an external
	// array (mypath/mycolor) that is rendered with glDrawArrays

	using ::Particle;

	/*Frame current = data_store->GetFrame(currentframe);

	triple * pos = current.pos;
	triple * d_pos = current.d_pos;
	triple * d_angle = current.d_angle;*/
	triple * d_angle = d_pos; // semantics only!
	int render_count = 0;

	for (int p = 0; p < count; p++)
	{
		if (current_visibility[p])
		{		
			//if (current_particle->marked==MARK_HIDE_MODE) return;		

			triple delta(0, 0, 0);
			triple origin(0, 0, 0);
			triple color(0, 0, 0);
			double headlength = 0.925; // length of arrowhead
			double headwidth = 0.05; // width of arrowhead
			double vscale = Particle::transVelocityLength; // scaling factor, to enlarge or shrink vector size arbitrarily
			origin = *pos;		

			switch (type)
			{
				case POSVELOCITY:
					vscale = Particle::transVelocityLength;// / maxTransVelocity;
					delta = *d_pos;//deltapos[frame];
					break;
				case ROTVELOCITY:
					//vscale /= 100.0;
					vscale = Particle::rotVelocityLength;// / maxRotVelocity;
					headlength = 0.9;
					headwidth = 0.05;
					delta = *d_angle;//deltatheta[frame];
					break;
				case NORMALVEC:
					//vscale = 1.0;
					vscale = Particle::transVelocityLength;
					headlength = 1.0;
					headwidth = 0.085;
					delta = particles[p]->normal[currentframe];
					break;
				case FIELDVEC:
					delta = *d_pos;
					vscale = Particle::fieldVelocityLength;// / maxFieldVelocity;
					break;
			}		

			double power = sqrt((delta.x*delta.x) +
								(delta.y*delta.y) +
								(delta.z*delta.z));	// power = magnitude

			switch (type)
			{		
				case ROTVELOCITY:
					power = power / Particle::maxRVelocityMap;
					color = Particle::grad.ConvertToColor(power);
					break;
				case FIELDVEC:
					power = power / Particle::maxFieldVectorMap;
					color = Particle::grad.ConvertToColor(power);
					break;
				case POSVELOCITY:
					power = power / Particle::maxVelocityMap;
					color = Particle::grad.ConvertToColor(power);
					break;
				case NORMALVEC:
					color.x = 0;
					color.y = 0;
					color.z = 0;
					break;	
			}

			//if (power<=0)
			//	return;
			if (power > 1) power = 1;
			
			triple up(0, 0, 1);
			triple cross(0, 0, 0);
			if (delta.x == 0 && delta.y == 0)
			{
				cross.x = 0;
				cross.y = sqrt((delta.x*delta.x) +
								(delta.y*delta.y) +
								(delta.z*delta.z));
				cross.z = 0;
			}
			else
			{
				cross = crossProduct(delta, up);
				double vecLength = sqrt((delta.x*delta.x) +
								(delta.y*delta.y) +
								(delta.z*delta.z));
				double crossLength = sqrt((cross.x*cross.x) +
								(cross.y*cross.y) +
								(cross.z*cross.z));
				cross.x *= (vecLength / crossLength);
				cross.y *= (vecLength / crossLength);
				cross.z *= (vecLength / crossLength);
			}
				

			// draw vector-line+arrowhead

			vBuf.AddVertex(origin, color);
			// vector-line itself
			vBuf.AddVertex(vscale*delta + origin, color);

			vBuf.AddVertex(headlength*vscale*delta + headwidth*vscale*cross + origin, color, 1.0);
			// begin arrowhead 1

			if (type != NORMALVEC)
			{
				vBuf.AddVertex(vscale*delta + origin, color);
				// end arrowhead 1
				// begin arrowhead 2
				vBuf.AddVertex(vscale*delta + origin, color);
			}

			vBuf.AddVertex(headlength*vscale*delta + -headwidth*vscale*cross + origin, color);
			// end arrowhead 2

			if (type == ROTVELOCITY) // add additional arrowhead
			{
				vBuf.AddVertex(0.95*vscale*delta + origin, color);
				// arrowhead 3
				vBuf.AddVertex((headlength-0.05)*vscale*delta + headwidth*vscale*cross + origin, color);

				vBuf.AddVertex(0.95*vscale*delta + origin, color);
				// arrowhead 4
				vBuf.AddVertex((headlength-0.05)*vscale*delta + -headwidth*vscale*cross + origin, color, 1.0);
			}
		}
		// increment data pointers to next particle
		pos++;
		d_pos++;
		d_angle++;
	}
}


failed occlusion picking code:

	if (GLEW_ARB_occlusion_query)
	//if (false)
	{		
		GLint viewport[4];	
		glGetIntegerv(GL_VIEWPORT, viewport);
		int N = primary->GetMaxParticles();
		GLuint sampleCount = 0;
		GLOcclusionQuery::InitQueries(N);		
		glColorMask(GL_FALSE, GL_FALSE, GL_FALSE, GL_FALSE);
		glDepthMask(GL_TRUE);
		glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
		glColor4f(1, 1, 1, 1);
		
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPickMatrix((double)x, (double)(viewport[3]-y), 0.1, 0.1, viewport);
		cam->Draw();
		primary->DrawFrame(frame); // depth-only pass


		glDepthMask(GL_FALSE);	
		glDepthFunc(GL_EQUAL);
		ParticleSet::useQueries = true;
		glMatrixMode(GL_PROJECTION);
		glLoadIdentity();
		gluPickMatrix((double)x, (double)(viewport[3]-y), 0.1, 0.1, viewport);
		cam->Draw();
		primary->DrawFrame(frame); // query each particle

		for (int i = 0; i < N; i++)
		{
			sampleCount = 0;
			glGetQueryObjectuivARB(GLOcclusionQuery::queries[i], GL_QUERY_RESULT_ARB, &sampleCount);
			if (sampleCount > 0)
			{
				hit_id = i;
				break;
			}
		}
		
		ParticleSet::useQueries = false;
		
		glColorMask(GL_TRUE, GL_TRUE, GL_TRUE, GL_TRUE);
		glDepthMask(GL_TRUE);
		glDepthFunc(GL_LESS);		
	}
end occlusion picking code


	// Load in all the spray slices.
	// SPRAY: x0.1764564m x 0.2032m
	
	/*testslice = GLPlanarSlices(256, 256, 18);
	testslice.SetDummyData(false);
	testslice.LoadSlice("\\programming\\DEM_INPUT_SCO\\0p5in_water_condition1CO.txt", 0);
	testslice.LoadSlice("\\programming\\DEM_INPUT_SCO\\1in_water_condition1CO.txt", 0.0127*1);
	testslice.LoadSlice("\\programming\\DEM_INPUT_SCO\\1p5in_water_condition1CO.txt", 0.0127*2);
	testslice.LoadSlice("\\programming\\DEM_INPUT_SCO\\2in_water_condition1CO.txt", 0.0127*3);
	testslice.LoadSlice("\\programming\\DEM_INPUT_SCO\\2p5in_water_condition1CO.txt", 0.0127*4);
	testslice.LoadSlice("\\programming\\DEM_INPUT_SCO\\3in_water_condition1CO.txt", 0.0127*5);
	testslice.LoadSlice("\\programming\\DEM_INPUT_SCO\\3p5in_water_condition1CO.txt", 0.0127*6);
	testslice.LoadSlice("\\programming\\DEM_INPUT_SCO\\4in_water_condition1CO.txt", 0.0127*7);
	testslice.LoadSlice("\\programming\\DEM_INPUT_SCO\\4p5in_water_condition1CO.txt", 0.0127*8);
	testslice.LoadSlice("\\programming\\DEM_INPUT_SCO\\5in_water_condition1CO.txt", 0.0127*9);
	testslice.LoadSlice("\\programming\\DEM_INPUT_SCO\\5p5in_water_condition1CO.txt", 0.0127*10);
	testslice.LoadSlice("\\programming\\DEM_INPUT_SCO\\6in_water_condition1CO.txt", 0.0127*11);
	testslice.LoadSlice("\\programming\\DEM_INPUT_SCO\\6p5in_water_condition1CO.txt", 0.0127*12);
	testslice.LoadSlice("\\programming\\DEM_INPUT_SCO\\7in_water_condition1CO.txt", 0.0127*13);
	testslice.LoadSlice("\\programming\\DEM_INPUT_SCO\\7p5in_water_condition1CO.txt", 0.0127*14);
	testslice.LoadSlice("\\programming\\DEM_INPUT_SCO\\8in_water_condition1_2CO.txt", 0.0127*15);	
	testslice.SetOrigin(triple(0.09, -0.1, 0.0));
	testslice.SetRotation(triple(-45, -90, 0));
	testslice.SetScale(triple(1.0/5.667, 1.0/5.667, 1.0/4.3743));
	testslice.SetRange(0.15);*/	
	
	
///// OLD CODE FOR SPRAY SLICES

// Load file data into the valuecache
bool GLPlanarSlices::GetFiles()
{
	if (filenames.size() != 16)
		return false;
	char buffer[255];

	static double lower_X, upper_X, lower_Y, upper_Y;
	/*lower_X = -.100;
	upper_X = .100;
	lower_Y = -.100;
	upper_Y = .100;*/

	lower_X = -.0882282;
	upper_X = .0882282;
	lower_Y = -.0882282;
	upper_Y = .0882282;

	float temp_value[256][256];
	int temp_count[256][256];

	delete[] valuecache;
	valuecache = new float[XMAX * YMAX * ZMAX];
	if (use_dummy) return true;

	std::fill(valuecache, valuecache + (XMAX * YMAX * ZMAX), 0.f);

	max_value = 0;
	for (int i = 1; i < ZMAX-1; i++)
	{
		for (int x = 0; x < XMAX; x++)
			for (int y = 0; y < YMAX; y++)
			{
				temp_value[x][y] = 0;
				temp_count[x][y] = 0;
			}
		pointcache.clear();

		ifstream infile;		
		infile.open(filenames[i-1].c_str(), ios::in);
		if (infile.fail())
			return false;
		infile.getline(buffer, 255);
		while (infile)
		{
			double x, y, val;
			infile >> x >> y >> val;
			pointcache.push_back( triple(x*1e-3, y*1e-3, val) );
		}
		for (int j = 0; j < pointcache.size(); j++)
		{
			triple p = pointcache[j];
			int x, y;
			x = (int)(255.0*((p.x-lower_X) / (upper_X - lower_X)));
			y = (int)(255.0*((p.y-lower_Y) / (upper_Y - lower_Y)));

			if (x>=0 && x<256 && y>=0 && y<256)
			{
				temp_value[x][y] += p.z;
				temp_count[x][y]++;
			}
		}		
		for (int x = 0; x < XMAX; x++)
		{
			for (int y = 0; y < YMAX; y++)
			{
				temp_value[x][y] = (temp_value[x][y] / (float)temp_count[x][y]);
				if (temp_value[x][y] > max_value)
					max_value = temp_value[x][y];
			}
		}
		for (int x = 0; x < XMAX; x++)
		{
			for (int y = 0; y < YMAX; y++)
			{
					int offset = (XMAX * YMAX * i)+
							(XMAX * x) +
							 y;
					valuecache[offset] = temp_value[x][y];// / max_value;
			}
		}
		//max_value = 0;
	}
	//max_value = 1.0;

	
	ofstream sprayout;
	sprayout.open("spray.raw", ios::out | ios::binary);
	for (int z = 0; z < ZMAX; z++)
	{
		for (int y = 0; y < YMAX; y++)
		{
			for (int x = 0; x < XMAX; x++)
			{
				int offset = (XMAX * YMAX * z)+
					(XMAX * x) +
					 y;
				unsigned short raw_value = (unsigned short)(65535*(valuecache[offset] / max_value));
				sprayout.write((char*)&raw_value, 2);
			}
		}
	}
	sprayout.close();

	return true;
}
//bool LoadSlice(string filename, double _height);
//bool GetFiles();
	if (loaded_files == false && !use_dummy)
	{
		GetFiles();
		loaded_files = true;
	}
// Add a filename to the loadlist
bool GLPlanarSlices::LoadSlice(string filename, double _height)
{
	this->height = _height;
	filenames.push_back(filename);
	return true;
}