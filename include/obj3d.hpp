#pragma once

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <malloc.h>
#include <math.h>
#include <string>
#include <vector>
#include <iostream>
#include <fstream>

#include <ogcsys.h>
#include <gccore.h>

#include "grrlib.h"


struct TexCoord {
	float x, y;
};

struct Face {
	guVector vtxa, vtxb, vtxc;
	guVector nrma, nrmb, nrmc;
	TexCoord txa, txb, txc;
};

class Obj3d {
private:


public:

	std::vector<Face> faces;

	//constructor
	Obj3d(const std::string& filePath);

	//destructor
	~Obj3d();
	
	//uses GX to draw the object, need to set GRRLIB_ObjectView() and 3dmode with normals and textures manually;
	void DrawObject();

};