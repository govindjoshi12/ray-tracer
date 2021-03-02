// The main ray tracer.

#pragma warning (disable: 4786)

#include "RayTracer.h"
#include "scene/light.h"
#include "scene/material.h"
#include "scene/ray.h"

#include "parser/Tokenizer.h"
#include "parser/Parser.h"

#include "ui/TraceUI.h"
#include <cmath>
#include <algorithm>
#include <glm/glm.hpp>
#include <glm/gtx/io.hpp>
#include <string.h> // for memset

#include <iostream>
#include <fstream>
// #include <future>

using namespace std;
extern TraceUI* traceUI;

// Use this variable to decide if you want to print out
// debugging messages.  Gets set in the "trace single ray" mode
// in TraceGLWindow, for example.
bool debugMode = false;

// Trace a top-level ray through pixel(i,j), i.e. normalized window coordinates (x,y),
// through the projection plane, and out into the scene.  All we do is
// enter the main ray-tracing method, getting things started by plugging
// in an initial ray weight of (0.0,0.0,0.0) and an initial recursion depth of 0.

bool aa = false; // Flag to check if antialias is needed

glm::dvec3 RayTracer::trace(double x, double y)
{
	// Clear out the ray cache in the scene for debugging purposes,
	if (TraceUI::m_debug)
	{
		scene->clearIntersectCache();		
	}

	ray r(glm::dvec3(0,0,0), glm::dvec3(0,0,0), glm::dvec3(1,1,1), ray::VISIBILITY);
	scene->getCamera().rayThrough(x,y,r);
	double dummy;
	glm::dvec3 ret = traceRay(r, glm::dvec3(1.0,1.0,1.0), traceUI->getDepth(), dummy);
	ret = glm::clamp(ret, 0.0, 1.0);
	return ret;
}

glm::dvec3 RayTracer::tracePixel(int i, int j)
{
	glm::dvec3 col(0, 0, 0);

	if (!sceneLoaded())
		return col;

	double x = double(i) / double(buffer_width);
	double y = double(j) / double(buffer_height);

	unsigned char *pixel = buffer.data() + (i + j * buffer_width) * 3;
	col = trace(x, y);

	pixel[0] = (int)(255.0 * col[0]);
	pixel[1] = (int)(255.0 * col[1]);
	pixel[2] = (int)(255.0 * col[2]);
	return col;
}

glm::dvec3 RayTracer::traceAAPixel(int i, int j) {
	
	glm::dvec3 col(0, 0, 0);

	if (!sceneLoaded())
		return col;

	double x = double(i) / double(buffer_width);
	double y = double(j) / double(buffer_height);

	int tempSamples = samples;
	for (int a = 0; a < tempSamples; a++)
	{
		for (int b = 0; b < tempSamples; b++)
		{
			x = (i + double(a) / double(tempSamples)) / double(buffer_width);
			y = (j + double(b) / double(tempSamples)) / double(buffer_height);
			col = col + trace(x, y);
		}
	}
	col = col / ((double)tempSamples * tempSamples);

	unsigned char *pixel = buffer.data() + (i + j * buffer_width) * 3;

	pixel[0] = (int)(255.0 * col[0]);
	pixel[1] = (int)(255.0 * col[1]);
	pixel[2] = (int)(255.0 * col[2]);
	return col;
}

#define VERBOSE 0

// Do recursive ray tracing!  You'll want to insert a lot of code here
// (or places called from here) to handle reflection, refraction, etc etc.
glm::dvec3 RayTracer::traceRay(ray& r, const glm::dvec3& thresh, int depth, double& t )
{
	isect i;
	glm::dvec3 colorC;

#if VERBOSE
	std::cerr << "== current depth: " << depth << std::endl;
#endif

	colorC = glm::dvec3(0.0, 0.0, 0.0);
	if(depth < 0) {
		return colorC;
	}

	if(scene->intersect(r, i)) {
		// Shade Pixel based on Phong Shading Model
		const Material& m = i.getMaterial();
		colorC = m.shade(scene.get(), r, i);

		t = i.getT();
		glm::dvec3 isectPoint = r.at(t);
		glm::dvec3 isectNormal = i.getN();
		glm::dvec3 rayDir = r.getDirection();

		// Shoot a reflective ray(s) if object is reflective
		/*
			Recursively shoot reflection rays in reflection direction based 
			on formula learned in class. Scale the obtained color by the
			intensity reduction constant (kr(i))
		*/
		if(m.Refl()) {
			const glm::dvec3 reflectionDir = glm::normalize(rayDir - ((2.0 * isectNormal) * (glm::dot(rayDir, isectNormal))));
			ray reflectionRay(isectPoint, reflectionDir, r.getAtten(), ray::REFLECTION);
			colorC += m.kr(i) * traceRay(reflectionRay, thresh, depth - 1.0, t);
		}

		// Shoot refractive ray if object is refractive
		/*
			Use law learned in class. If parameters result in invalid angle,
			shoot reflective ray instead. In this case, multiplying by material 
			constant for that object is "optional."

			Snell's Law
		*/
		if(m.Trans()) {
			double cos1 = glm::dot(rayDir, isectNormal);
			glm::dvec3 incidentNormal = isectNormal;

			double n1 = 1; // Speed in air or vaccuum
			double n2 = m.index(i);

			if(cos1 < 0.0) {
				cos1 = -cos1;
			} else {
				std::swap(n1, n2);
				incidentNormal = -incidentNormal;
			}

			double iof = n1 / n2;
			// double sin1 = iof * std::sqrt(std::max(0.0, 1.0 - std::pow(cos1, 2)));
			double tir = 1 - (std::pow(iof, 2) *  (1 - std::pow(cos1, 2)));
			// Check if there is total internal reflection
			if (tir < 0) {  // sin1 < -1.0 || sin1 > 1.0

				// Total internal reflection
				// Make method for reflection...?????
				const glm::dvec3 reflectionDir = glm::normalize(rayDir - ((2.0 * isectNormal) * (glm::dot(rayDir, isectNormal))));
				ray reflectionRay(isectPoint, reflectionDir, r.getAtten(), ray::REFLECTION);
				colorC += m.kr(i) * traceRay(reflectionRay, thresh, depth - 1.0, t);

			} else {
				// Bottom equation from scratchapixel, uses trig identities
				// How could we use sin1 to rotate incidentNormal to correct dir?
				double c = 1 - std::pow(iof, 2.0) * (1.0 - std::pow(cos1, 2.0));
				glm::dvec3 refrDir = (iof * rayDir) 
					+ (((iof * cos1) - std::sqrt(c)) * incidentNormal);
				ray refrRay(isectPoint, refrDir, r.getAtten(), ray::REFRACTION);
				colorC += traceRay(refrRay, thresh, depth-1.0, t);
			}
		}

	} else {
		// No intersection.  This ray travels to infinity, so we color
		// it according to the background color, which in this (simple) case
		// is just black.
		//
		// FIXME: Add CubeMap support here.
		// TIPS: CubeMap object can be fetched from traceUI->getCubeMap();
		//       Check traceUI->cubeMap() to see if cubeMap is loaded
		//       and enabled.

		colorC = glm::dvec3(0.0, 0.0, 0.0);
	}
#if VERBOSE
	std::cerr << "== depth: " << depth+1 << " done, returning: " << colorC << std::endl;
#endif
	return colorC;
}

RayTracer::RayTracer()
	: scene(nullptr), buffer(0), thresh(0), buffer_width(0), buffer_height(0), m_bBufferReady(false)
{
}

RayTracer::~RayTracer()
{
}

void RayTracer::getBuffer( unsigned char *&buf, int &w, int &h )
{
	buf = buffer.data();
	w = buffer_width;
	h = buffer_height;
}

double RayTracer::aspectRatio()
{
	return sceneLoaded() ? scene->getCamera().getAspectRatio() : 1;
}

bool RayTracer::loadScene(const char* fn)
{
	ifstream ifs(fn);
	if( !ifs ) {
		string msg( "Error: couldn't read scene file " );
		msg.append( fn );
		traceUI->alert( msg );
		return false;
	}

	// Strip off filename, leaving only the path:
	string path( fn );
	if (path.find_last_of( "\\/" ) == string::npos)
		path = ".";
	else
		path = path.substr(0, path.find_last_of( "\\/" ));

	// Call this with 'true' for debug output from the tokenizer
	Tokenizer tokenizer( ifs, false );
	Parser parser( tokenizer, path );
	try {
		scene.reset(parser.parseScene());
	}
	catch( SyntaxErrorException& pe ) {
		traceUI->alert( pe.formattedMessage() );
		return false;
	} catch( ParserException& pe ) {
		string msg( "Parser: fatal exception " );
		msg.append( pe.message() );
		traceUI->alert( msg );
		return false;
	} catch( TextureMapException e ) {
		string msg( "Texture mapping exception: " );
		msg.append( e.message() );
		traceUI->alert( msg );
		return false;
	}

	if (!sceneLoaded())
		return false;

	return true;
}

void RayTracer::traceSetup(int w, int h)
{
	size_t newBufferSize = w * h * 3;
	if (newBufferSize != buffer.size()) {
		bufferSize = newBufferSize;
		buffer.resize(bufferSize);
	}
	buffer_width = w;
	buffer_height = h;
	std::fill(buffer.begin(), buffer.end(), 0);
	m_bBufferReady = true;

	/*
	 * Sync with TraceUI
	 */

	threads = traceUI->getThreads();
	block_size = traceUI->getBlockSize();
	thresh = traceUI->getThreshold();
	samples = traceUI->getSuperSamples();
	aaThresh = traceUI->getAaThreshold();

	// YOUR CODE HERE
	// FIXME: Additional initializations
}

/*
 * RayTracer::traceImage
 *
 *	Trace the image and store the pixel data in RayTracer::buffer.
 *
 *	Arguments:
 *		w:	width of the image buffer
 *		h:	height of the image buffer
 *
 */
void RayTracer::traceImage(int w, int h)
{
	// Always call traceSetup before rendering anything.
	traceSetup(w,h);

	// YOUR CODE HERE
	// FIXME: Start one or more threads for ray tracing
	//
	// TIPS: Ideally, the traceImage should be executed asynchronously,
	//       i.e. returns IMMEDIATELY after working threads are launched.
	//
	//       An asynchronous traceImage lets the GUI update your results
	//       while rendering.
	for(int i = 0; i < w; i++)
	{
		for(int j = 0; j < h; j++)
		{
			tracePixel(i, j);
		}
	}
}

int RayTracer::aaImage()
{
	// YOUR CODE HERE
	// FIXME: Implement Anti-aliasing here
	//
	// TIP: samples and aaThresh have been synchronized with TraceUI by
	//      RayTracer::traceSetup() function
	for(int i = 0; i < buffer_width; i++)
	{
		for(int j = 0; j < buffer_height; j++)
		{
			traceAAPixel(i, j);
		}
	}
	return 0;
}

bool RayTracer::checkRender()
{
	// YOUR CODE HERE
	// FIXME: Return true if tracing is done.
	//        This is a helper routine for GUI.
	//
	// TIPS: Introduce an array to track the status of each worker thread.
	//       This array is maintained by the worker threads.
	return true;
}

void RayTracer::waitRender()
{
	// YOUR CODE HERE
	// FIXME: Wait until the rendering process is done.
	//        This function is essential if you are using an asynchronous
	//        traceImage implementation.
	//
	// TIPS: Join all worker threads here.
}


glm::dvec3 RayTracer::getPixel(int i, int j)
{
	unsigned char *pixel = buffer.data() + ( i + j * buffer_width ) * 3;
	return glm::dvec3((double)pixel[0]/255.0, (double)pixel[1]/255.0, (double)pixel[2]/255.0);
}

void RayTracer::setPixel(int i, int j, glm::dvec3 color)
{
	unsigned char *pixel = buffer.data() + ( i + j * buffer_width ) * 3;

	pixel[0] = (int)( 255.0 * color[0]);
	pixel[1] = (int)( 255.0 * color[1]);
	pixel[2] = (int)( 255.0 * color[2]);
}

