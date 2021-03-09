#include "cubeMap.h"
#include "ray.h"
#include "../ui/TraceUI.h"
#include "../scene/material.h"
extern TraceUI* traceUI;

glm::dvec3 CubeMap::getColor(ray r) const
{
	// YOUR CODE HERE
	// FIXME: Implement Cube Map here

	// +-x,y,z
	int squareFace; // x = 0, -x = 1, ...
	glm::dvec3 d = r.getDirection();

	double abs[] = {std::abs(d[0]), std::abs(d[1]), std::abs(d[2])};
	
	// Which square of the cube map will ray hit
	// tMap[n]
	// getColor from Texture map
	// Algo from https://en.wikipedia.org/wiki/Cube_mapping
	double u;
	double v;
	double largest;

	if(abs[0] == d[0] && abs[0] >= abs[1] && abs[0] >= abs[2]) { // +x
		squareFace = 0;
		largest = abs[0];
		u = -d[2];
		v = d[1];
	}
	else if(abs[0] != d[0] && abs[0] >= abs[1] && abs[0] >= abs[2]) { // -x
		squareFace = 1;
		largest = abs[0];
		u = d[2];
		v = d[1];
	}
	else if(abs[1] == d[1] && abs[1] >= abs[0] && abs[1] >= abs[2]) { // +y
		squareFace = 2;
		largest = abs[1];
		u = d[0];
		v = -d[2];
	}
	else if(abs[1] != d[1] && abs[1] >= abs[0] && abs[1] >= abs[2]) { // -y
		squareFace = 3;
		largest = abs[1];
		u = d[0];
		v = d[2];
	}
	else if(abs[2] == d[2] && abs[2] >= abs[0] && abs[2] >= abs[1]) { // +z
		squareFace = 4;
		largest = abs[2];
		u = d[0];
		v = d[1];
	}
	else if(abs[2] != d[2] && abs[2] >= abs[0] && abs[2] >= abs[1]) { // -z
		squareFace = 5;
		largest = abs[2];
		u = -d[0];
		v = d[1];
	}

	u = ((u / largest) + 1.0)/2.0;
	v = ((v / largest) + 1.0)/2.0;
	// u = 0.5 * (u / largest + 1.0);
  	// v = 0.5 * (u / largest + 1.0);
	return tMap[squareFace]->getMappedValue(glm::dvec2(u,v));
}

CubeMap::CubeMap()
{
}

CubeMap::~CubeMap()
{
}

void CubeMap::setNthMap(int n, TextureMap* m)
{
	if (m != tMap[n].get()) {
		tMap[n].reset(m);
	}	
}
