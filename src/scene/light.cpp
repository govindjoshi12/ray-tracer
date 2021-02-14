#include <cmath>
#include <iostream>

#include "light.h"
#include <glm/glm.hpp>
#include <glm/gtx/io.hpp>


using namespace std;

double DirectionalLight::distanceAttenuation(const glm::dvec3& P) const
{
	// distance to light is infinite, so f(di) goes to 0.  Return 1.
	return 1.0;
}


glm::dvec3 DirectionalLight::shadowAttenuation(const ray& r, const glm::dvec3& p) const
{
	// YOUR CODE HERE:
	// You should implement shadow-handling code here.

	// Shadow Rays
	/*
		Check if ray is a shadow ray. If it is, return black 
		if the object intersected with is opaque. Else, shoot
		another shadow ray in the same direction and apply
		shadow attenuation based on kt^d formula. Return color. 
		Shadow Rays will only be shot from shade method.
	*/
	// YOUR CODE HERE:
	// You should implement shadow-handling code here.
	glm::dvec3 dir = r.getDirection();
	ray shadow(p, dir, r.getAtten(), ray::SHADOW);
	
	isect i;

	/* Check that the distance from the base of the shadowray to 
	the intersection is less than the distance from the point light 
	to the base of the shadowray */
	if(scene->intersect(shadow, i)) {
		// Find the point of intersection
		glm::dvec3 isectPoint = shadow.at(i.getT());
		
		// // Get property of material
		const Material &m = i.getMaterial();

		if(m.Trans()) {
			glm::dvec3 normal = i.getN();
			glm::dvec3 attenFactor = glm::dvec3(1.0, 1.0, 1.0);
			double cos1 = glm::dot(normal, shadow.getDirection());
			
			// If ray entering object, atten factor doesn't change
			// else, compute kt^d
			if(cos1 >= 0) {
				double distanceInObj = glm::distance(p, isectPoint);
				glm::dvec3 kt = m.kt(i);
				glm::dvec3 ktToD = glm::dvec3(pow(kt[0], distanceInObj), 
											pow(kt[1], distanceInObj), 
											pow(kt[2], distanceInObj));
				attenFactor *= ktToD;
				
			}

			ray newShadowRay(isectPoint, shadow.getDirection(), shadow.getAtten(), ray::SHADOW);
			return attenFactor * shadowAttenuation(newShadowRay, isectPoint);
		} else {
			// If even one object in shadow ray path is opaque, pixel is black.
			return glm::dvec3(0.0, 0.0, 0.0);
		}
	} else {
		// Base case: Does not intersect
		// No shadow, no attenuation
		return glm::dvec3(1.0, 1.0, 1.0);
	}

}

glm::dvec3 DirectionalLight::getColor() const
{
	return color;
}

glm::dvec3 DirectionalLight::getDirection(const glm::dvec3& P) const
{
	return -orientation;
}

double PointLight::distanceAttenuation(const glm::dvec3& P) const
{

	// YOUR CODE HERE

	// You'll need to modify this method to attenuate the intensity 
	// of the light based on the distance between the source and the 
	// point P.  For now, we assume no attenuation and just return 1.0
	double distance = glm::distance(P, position);
	double linear = linearTerm * distance;
	double quadratic = quadraticTerm * std::pow(distance, 2);
	return std::min((1 / (constantTerm + linear + quadratic)), 1.0);
}

glm::dvec3 PointLight::getColor() const
{
	return color;
}

glm::dvec3 PointLight::getDirection(const glm::dvec3& P) const
{
	return glm::normalize(position - P);
}


glm::dvec3 PointLight::shadowAttenuation(const ray& r, const glm::dvec3& p) const
{
	// YOUR CODE HERE:
	// You should implement shadow-handling code here.
	glm::dvec3 dir = r.getDirection();
	ray shadow(p, dir, r.getAtten(), ray::SHADOW);
	
	isect i;
	bool inter = scene->intersect(shadow, i);

	/* Check that the distance from the base of the shadowray to 
	the intersection is less than the distance from the point light 
	to the base of the shadowray */
	double baseToIntersection = glm::distance(p, shadow.at(i.getT()));
	double baseToPL = glm::distance(p, position);
	inter = inter && (baseToIntersection < baseToPL);

	if(inter) {
		// Find the point of intersection
		glm::dvec3 isectPoint = shadow.at(i.getT());
		
		// // Get property of material
		const Material &m = i.getMaterial();

		if(m.Trans()) {
			glm::dvec3 normal = i.getN();
			glm::dvec3 attenFactor = glm::dvec3(1.0, 1.0, 1.0);
			double cos1 = glm::dot(normal, shadow.getDirection());
			
			// If ray entering object, atten factor doesn't change
			// else, compute kt^d
			if(cos1 >= 0) {
				double distanceInObj = glm::distance(p, isectPoint);
				glm::dvec3 kt = m.kt(i);
				glm::dvec3 ktToD = glm::dvec3(pow(kt[0], distanceInObj), 
											pow(kt[1], distanceInObj), 
											pow(kt[2], distanceInObj));
				attenFactor *= ktToD;
				
			}

			ray newShadowRay(isectPoint, shadow.getDirection(), shadow.getAtten(), ray::SHADOW);
			return attenFactor * shadowAttenuation(newShadowRay, isectPoint);
		} else {
			// If even one object in shadow ray path is opaque, pixel is black.
			return glm::dvec3(0.0, 0.0, 0.0);
		}
	} else {
		// Base case: Does not intersect
		// No shadow, no attenuation
		return glm::dvec3(1.0, 1.0, 1.0);
	}
}

#define VERBOSE 0

