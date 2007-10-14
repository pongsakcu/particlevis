#include "GLShader.h"
#include <stdlib.h>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>
#include <vector>
#include <GL/glew.h>

using std::ifstream;
using std::string;
using std::vector;
using std::stringstream;

string GLShader::base_path = "";

GLShader::GLShader(void)
{	
	valid = false;
	frag_shader = vert_shader = program = -1;
}

GLShader::~GLShader(void)
{
}

void GLShader::LoadSourceFile(const char * filename)
{
	string fullname = base_path;
	fullname.append(filename);
	lines = 0;
	ifstream infile;
	infile.open(fullname.c_str(), std::ios::in);
	code_buffer[0] = '\0';
	while (!infile.eof() && !infile.fail())
	{
		char buffer[512];
		infile.getline(buffer, 512);
		strcat(buffer, "\n");
		strcat(code_buffer, buffer);
		lines++;
	}
}

bool GLShader::LoadVertFile(const char * vert_file)
{
	LoadSourceFile(vert_file);
	return LoadVertSource((const char*)code_buffer);
}

bool GLShader::LoadFragFile(const char * frag_file)
{
	LoadSourceFile(frag_file);
	return LoadFragSource((const char*)code_buffer);	
}

bool GLShader::LoadVertSource(const char * vert_source)
{
	if (GLEW_VERSION_2_0 && GLEW_ARB_vertex_shader)
	{
		int success = GL_FALSE;
		if (vert_shader == -1)	
			vert_shader = glCreateShaderObjectARB(GL_VERTEX_SHADER_ARB);
		if (program == -1)
			program = glCreateProgramObjectARB();			
				
		glShaderSourceARB(vert_shader, 1, &vert_source, NULL);
		glCompileShaderARB(vert_shader);
		glAttachObjectARB(program, vert_shader);
		glLinkProgramARB(program);		

		glGetProgramiv(program, GL_LINK_STATUS, &success);
		if (success == GL_TRUE)
		{
			valid = true;	
			return true;
		}
		else
		{
			valid = false;
			return false;
		}
	}
	else
	{
		valid = false;
		return false;
	}	
}

bool GLShader::LoadFragSource(const char * frag_source)
{
	if (GLEW_VERSION_2_0 && GLEW_ARB_fragment_shader)
	{
		int success = GL_FALSE;
		if (frag_shader == -1)	
			frag_shader = glCreateShaderObjectARB(GL_FRAGMENT_SHADER_ARB);
		if (program == -1)
			program = glCreateProgramObjectARB();			
		
		glShaderSourceARB(frag_shader, 1, &frag_source, NULL);
		glCompileShaderARB(frag_shader);
		glAttachObjectARB(program, frag_shader);
		glLinkProgramARB(program);

		glGetProgramiv(program, GL_LINK_STATUS, &success);
		if (success == GL_TRUE)
		{
			valid = true;	
			return true;
		}
		else
		{
			valid = false;
			return false;
		}
	}
	else
	{
		valid = false;
		return false;
	}	
}

bool GLShader::CheckLog(char * buffer, int len)
{
	if (GLEW_ARB_shader_objects)
		glGetInfoLogARB(frag_shader, len, NULL, buffer);
	if (strlen(buffer) > 0)
		return true;
	return false;
}

bool GLShader::Bind() {
	if (valid && GLEW_ARB_shader_objects)
	{
		glUseProgramObjectARB(program);
		return true;
	}
	else
		return false;
}

bool GLShader::UnBind() {
	if (GLEW_ARB_shader_objects)
		glUseProgramObjectARB(GL_NONE);
	return true;
}

void GLShader::SetPath(std::string path)
{
	GLShader::base_path = path;
}