// GLShader class
//
// Encapsulate a GLSL fragment + vertex program
//
// Vincent Hoon, 2007
// See copyright notice in copyright.txt for conditions of distribution and use.

#pragma once
#define SHADERLINES 2048

#include <string>

class GLShader
{
public:
	GLShader(void);
	~GLShader(void);	

	bool LoadVertFile(const char * vert_file);
	bool LoadFragFile(const char * vert_file);
	bool LoadVertSource(const char * vert_source);
	bool LoadFragSource(const char * frag_source);
	bool CheckLog(char * buffer, int len);

	bool Bind();
	bool UnBind();	

	unsigned int GetProgramID() { return program; }

	static void SetPath(std::string path);

private:
	void LoadSourceFile(const char * filename);
	unsigned int frag_shader, vert_shader, program;
	bool valid;
	char code_buffer[SHADERLINES*128];
	int lines;
	static std::string base_path;
};
