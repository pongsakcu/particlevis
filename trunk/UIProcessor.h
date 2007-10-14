/*
UIProcessor
	
		   The UIProcessor manages various types of user input, such as mouse motion
		and keyboard events.  It also reads and writes configuration information to a
		registry (in this case the windows registry, using Afx).

		Vincent Hoon, 2007
*/
#pragma once
#include "ChildView.h"
#include "ParticleSet.h"
#include <string>
using std::string;

class UIProcessor
{
public:
	UIProcessor(CChildView* mainwindow, ParticleSet* primary);
	~UIProcessor(void);
	bool ProcessArgs(char* argv);
	string OnHelpCmdline();
	bool KeyboardInput(unsigned int nChar); // accept key input.  return true if redraw is necessary.
	bool KeyboardEndInput(unsigned int nChar); // accept key-up input.  return true if redraw is necessary.
	void LoadGradientPreset(int preset);
	void ReadScenePrefs(); // read preferences
	void WriteScenePrefs(); // write preferences
	string ProduceSummary();

	void AutoSceneSettings(); // set camera, scaling, automatically

	static const int SPECTRUM = 0;
	static const int GREYSCALE = 1;
	static const int THERMAL = 2;
	static const int SPRAY1 = 3;
	static const int SPRAY2 = 4;
private:

	bool GetRegistryData(string key, double& value);
	bool GetRegistryData(string key, int& value);
	bool GetRegistryData(string key, bool& value);
	bool GetRegistryData(string key, triple& value);

	bool WriteRegistryData(string key, bool value);
	bool WriteRegistryData(string key, int value);
	bool WriteRegistryData(string key, double value);
	bool WriteRegistryData(string key, triple value);

	CChildView* mainwindow;
	ParticleSet* primary;
	Camera* primary_cam;
};
