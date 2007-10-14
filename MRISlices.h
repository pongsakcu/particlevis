/*
 *
 *	MRISLICES
 *	
 *	This class extends the PlanarSlices class to allow the loading
 *	of 3DHEAD.VOL.  It sets appropriate parameters for rendering
 *	the head optimally.
 *
 */

#pragma once
#include "stdafx.h"
#include "GLPlanarSlices.h"

using namespace std;

class MRISlices :
	public GLPlanarSlices
{
public:
	MRISlices(void);
	~MRISlices(void);
	bool RenderTextures();
protected:
	virtual void SetTextureMatrix();
};
