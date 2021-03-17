#include "material.h"
#include "../ui/TraceUI.h"
#include "light.h"
#include "ray.h"
extern TraceUI* traceUI;

#include <glm/gtx/io.hpp>
#include <iostream>
#include "../fileio/images.h"

using namespace std;
extern bool debugMode;

Material::~Material()
{
}

// Apply the phong model to this point on the surface of the object, returning
// the color of that point.
glm::dvec3 Material::shade(Scene* scene, const ray& r, const isect& i) const
{
	// YOUR CODE HERE

	// For now, this method just returns the diffuse color of the object.
	// This gives a single matte color for every distinct surface in the
	// scene, and that's it.  Simple, but enough to get you started.
	// (It's also inconsistent with the phong model...)

	// Your mission is to fill in this method with the rest of the phong
	// shading model, including the contributions of all the light sources.
	// You will need to call both distanceAttenuation() and
	// shadowAttenuation()
	// somewhere in your code in order to compute shadows and light falloff.
	//	if( debugMode )
	//		std::cout << "Debugging Phong code..." << std::endl;

	// When you're iterating through the lights,
	// you'll want to use code that looks something
	// like this:
	//
	// for ( const auto& pLight : scene->getAllLights() )
	// {
	//              // pLight has type unique_ptr<Light>
	// 		.
	// 		.
	// 		.
	// }

	// Get value for color

	// SP=diffuse()*Kd+specular()*Ks.
	glm::dvec3 ambient = ka(i) * scene->ambient();
	
	// I = (ka * I_scene) + I_in (kd*max(l dot n) + ks * max(eye dot reflection, 0)^spec_coeff) 
	glm::dvec3 P = r.at(i.getT());
	glm::dvec3 diffuse = glm::dvec3(0.0, 0.0, 0.0);
	glm::dvec3 specular = glm::dvec3(0.0, 0.0, 0.0);
	glm::dvec3 normal = i.getN();
	glm::dvec3 direction = r.getDirection();
	//printf("normal: (%f, %f, %f)\n", normal[0], normal[1], normal[2]);

	double cos1 = glm::dot(normal, direction);
	// If exiting trans object

	if(cos1 >= 0) {
		normal = -normal;
	}	

	for ( const auto& pLight : scene->getAllLights() ) {
		glm::dvec3 lightDir = pLight->getDirection(P);
		glm::dvec3 lightColor = pLight->getColor() * pLight->distanceAttenuation(P);

		// Need to factor in shadowAttenuation
		ray shadowRay(P, glm::normalize(lightDir), r.getAtten(), ray::SHADOW);
		lightColor *= pLight->shadowAttenuation(shadowRay, P);

		diffuse += lightColor * std::max(glm::dot(lightDir, normal), 0.0);
		if(cos1 >= 0) {
			diffuse = glm::dvec3(std::abs(diffuse[0]), std::abs(diffuse[1]), std::abs(diffuse[2]));
		}		
		// w_in = lightDir "Incident light at direction P"
		// n = normal
		// Reflection Vector: w_out = w_in - (2n(w_in dot n))
		glm::dvec3 reflection = (lightDir - ((2.0 * normal) * (glm::dot(lightDir, normal))));
		specular += lightColor * std::pow(std::max(glm::dot(reflection, direction), 0.0), shininess(i));
	}
	glm::dvec3 shadingColor = ke(i) + (ka(i) * ambient) + (kd(i) * diffuse) + (ks(i) * specular);
	return shadingColor;
}

TextureMap::TextureMap(string filename)
{
	data = readImage(filename.c_str(), width, height);
	if (data.empty()) {
		width = 0;
		height = 0;
		string error("Unable to load texture map '");
		error.append(filename);
		error.append("'.");
		throw TextureMapException(error);
	}
}

glm::dvec3 TextureMap::getMappedValue(const glm::dvec2& coord) const
{
	// YOUR CODE HERE
	//
	// In order to add texture mapping support to the
	// raytracer, you need to implement this function.
	// What this function should do is convert from
	// parametric space which is the unit square
	// [0, 1] x [0, 1] in 2-space to bitmap coordinates,
	// and use these to perform bilinear interpolation
	// of the values.

	double u = coord[0] * width;
	double v = coord[1] * height;
	
	double u1 = std::floor(u);
	double u2 = u1 + 1;
	double v1 = std::floor(v);
	double v2 = v1 + 1;

	double alphaU = (u2 - u) / (u2 - u1);
	double betaU = (u - u1) / (u2 - u1);
	
	double alphaV = (v2 - v) / (v2 - v1);
	double betaV = (v - v1) / (v2 - v1);

	glm::dvec3 colorA = getPixelAt(u1, v1);
	glm::dvec3 colorB = getPixelAt(u2, v1);
	glm::dvec3 colorC = getPixelAt(u2, v2);
	glm::dvec3 colorD = getPixelAt(u1, v2);

	glm::dvec3 finalColor = alphaV * (alphaU * colorA + betaU * colorB)
						+ betaV * (alphaU * colorD + betaU * colorC);

	return finalColor;

	// return getPixelAt(std::floor(u), std::floor(v));
}

glm::dvec3 TextureMap::getPixelAt(int x, int y) const
{
	// YOUR CODE HERE
	//
	// In order to add texture mapping support to the
	// raytracer, you need to implement this function.
	// if(data.empty()) {
	// 	return glm::dvec3(1.0, 1.0, 1.0); 
	// }

	x = glm::clamp(x, 0, width);
	y = glm::clamp(y, 0, height);

	glm::dvec3 colorC;
	// Compute index of 1D array which 
	// represents the image pixels.
	// Multiply by the three because 
	// values for each 3 color channels
	// stored in data.
	int padding = (x + (y * width)) * 3;
	auto pixel = data.data() + padding;

	colorC[0] = (double)pixel[0]/255.0;
	colorC[1] = (double)pixel[1]/255.0;
	colorC[2] = (double)pixel[2]/255.0;
	return colorC;
}

glm::dvec3 MaterialParameter::value(const isect& is) const
{
	if (0 != _textureMap)
		return _textureMap->getMappedValue(is.getUVCoordinates());
	else
		return _value;
}

double MaterialParameter::intensityValue(const isect& is) const
{
	if (0 != _textureMap) {
		glm::dvec3 value(
		        _textureMap->getMappedValue(is.getUVCoordinates()));
		return (0.299 * value[0]) + (0.587 * value[1]) +
		       (0.114 * value[2]);
	} else
		return (0.299 * _value[0]) + (0.587 * _value[1]) +
		       (0.114 * _value[2]);
}
