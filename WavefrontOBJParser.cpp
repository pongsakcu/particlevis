#include ".\wavefrontobjparser.h"
#include <iostream>
#include <fstream>
#include <sstream>

#include "stdafx.h"


WavefrontOBJParser::WavefrontOBJParser(void)
{
}

WavefrontOBJParser::~WavefrontOBJParser(void)
{
}

bool WavefrontOBJParser::ParseData(string filename) // read in file and load into internal data structures
{
	using namespace std;
	float x, y, z;
	int i = 0, j = 0, k = 0;
	char buffer[512];
	
	//stringstream tokenstream;
	string token;

	ifstream infile;
	infile.open(filename.c_str(), ios::in);
	if (!infile)
		return false;


	while (infile)
	{
		//line_stream.str("");
		//line_stream << infile; // read in one line of data
		//line_str = line_stream.str();

		infile >> token;
		
		if (token == "#")
		{
			// a comment, do nothing
		}

		if (token == "v")
		{
			infile >> x >> y >> z;
			triple vertex(x, y, z);
			points.push_back(vertex);
		}

		if (token == "vn")
		{
			infile >> x >> y >> z;
			triple normal(x, y, z);
			normalize(normal);
			normals.push_back(normal);
		}

		if (token == "vt")
		{
			infile >> x >> y;
			triple texcoord(x, y, 0);
			texcoords.push_back(texcoord);
		}

		if (token == "f")
		{
			while (infile.peek() != '\n')
			{				
				infile >> token;
				
				/*infile.getline(buffer, '/');
				i = atoi(buffer);
				infile.getline(buffer, '/');
				j = atoi(buffer);
				infile.getline(buffer, ' ');
				k = atoi(buffer);*/

				strcpy(buffer, token.c_str());
				stringstream tokenstream(token);
				tokenstream.getline(buffer, 255, '/');
				i = atoi(buffer);
				tokenstream.getline(buffer, 255, '/');
				j = atoi(buffer);
				tokenstream.getline(buffer, 255);
				k = atoi(buffer);
				
				/*int pos = token.find("/");
				if (pos != string::npos)
					token.replace(pos, 1, " ");
				pos = token.find("/");
				if (pos != string::npos)
					token.replace(pos, 1, " ");
				stringstream tokenstream(token);
				tokenstream >> i >> j >> k;
				
				if (k > 0)
				{
					polygons.push_back(k-1); // normal
					polygons.push_back(j-1); // tex
					polygons.push_back(i-1); // vertex
				}
				else if (j > 0)
				{
					polygons.push_back(j-1);
					polygons.push_back(0);
					polygons.push_back(i-1);
				}
				else
				{
					polygons.push_back(i-1);
					polygons.push_back(0);
					polygons.push_back(i-1);
				}*/

				if (k <= 0)
					k = i;
				
				polygons.push_back(k-1);
				polygons.push_back(j-1);
				polygons.push_back(i-1);
				i = j = k = 0;
			}
			//polygons.push_back(-1); // terminator
		}
	}
	if (normals.empty())	
		GenNormals();
	return true;
}

void WavefrontOBJParser::GenNormals()
{
	int t = 0, maxsize = points.size();
	int i1 = 0, i2 = 0, i3 = 0;
	triple p1, p2, p3, normal;
	vector<int> references;
	references.resize(maxsize);
	normals.resize(maxsize);

	for (t = 0; t < maxsize; t++)	
	{
		normals[t] = triple(0, 0, 0);
		references[t] = 0;
	}

	int c = polygons.size();
	c = 0;
	vector<int>::iterator iter;
	iter = polygons.begin();

	for (c = 0; c < polygons.size(); c+=9)
	{
		//iter++; iter++;
		i1 = polygons[c+2];
		p1 = points[i1];

		//iter++; iter++;
		i2 = polygons[c+5];
		p2 = points[i2];

		//iter++; iter++;
		i3 = polygons[c+8];
		p3 = points[i3];
		
		normal = crossProduct((p2 - p1), (p3 - p1));
		normalize(normal);

		normals[i1] = normal;
		normals[i2] = normal;
		normals[i3] = normal;

		references[i1]++; references[i2]++; references[i3]++;
	}
	
	for (t = 0; t < normals.size(); t++)	
	{
		normals[t] = normals[t] / (double)references[t];
		normalize(normals[t]);
	}
}

void WavefrontOBJParser::DrawData() // issue data using OpenGL
{
	int index;
	vector<int>::iterator iter;
	glBegin(GL_TRIANGLES);
	//for (iter = polygons.begin(); iter != polygons.end(); iter++)
	iter = polygons.begin();
	while (iter != polygons.end())
	{
		index = *(iter++);
		if (index >= 0 && index < normals.size())
			glNormal3f(normals[index].x, normals[index].y, normals[index].z);		
		
		index = *(iter++);
		if (index >= 0 && index < texcoords.size())
			glTexCoord2d(texcoords[index].x, texcoords[index].y);

		index = *(iter++);
		if (index >= 0 && index < points.size())
			glVertex3f(points[index].x, points[index].y, points[index].z);
	}
	glEnd();
}
