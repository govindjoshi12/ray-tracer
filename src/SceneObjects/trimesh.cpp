#include "trimesh.h"
#include <assert.h>
#include <float.h>
#include <string.h>
#include <algorithm>
#include <cmath>
#include "../ui/TraceUI.h"
#include <glm/gtx/io.hpp>

extern TraceUI* traceUI;

using namespace std;

Trimesh::~Trimesh()
{
	for (auto m : materials)
		delete m;
	for (auto f : faces)
		delete f;
	delete BVHTree;
}

// must add vertices, normals, and materials IN ORDER
void Trimesh::addVertex(const glm::dvec3& v)
{
	vertices.emplace_back(v);
}

void Trimesh::addMaterial(Material* m)
{
	materials.emplace_back(m);
}

void Trimesh::addNormal(const glm::dvec3& n)
{
	normals.emplace_back(n);
}

// Returns false if the vertices a,b,c don't all exist
bool Trimesh::addFace(int a, int b, int c)
{
	int vcnt = vertices.size();

	if (a >= vcnt || b >= vcnt || c >= vcnt)
		return false;

	TrimeshFace* newFace = new TrimeshFace(
	        scene, new Material(*this->material), this, a, b, c);
	newFace->setTransform(this->transform);
	if (!newFace->degen)
		faces.push_back(newFace);
	else
		delete newFace;

	// Don't add faces to the scene's object list so we can cull by bounding
	// box
	return true;
}

// Check to make sure that if we have per-vertex materials or normals
// they are the right number.
const char* Trimesh::doubleCheck()
{
	if (!materials.empty() && materials.size() != vertices.size())
		return "Bad Trimesh: Wrong number of materials.";
	if (!normals.empty() && normals.size() != vertices.size())
		return "Bad Trimesh: Wrong number of normals.";

	return 0;
}

bool Trimesh::intersectLocal(ray& r, isect& i) const
{
	bool have_one = false;
	// for (auto face : faces) {
	// 	isect cur;
	// 	if (face->intersectLocal(r, cur)) {
	// 		if (!have_one || (cur.getT() < i.getT())) {
	// 			i = cur;
	// 			have_one = true;
	// 		}
	// 	}
	// }

	ray* temp = new ray(r);
	Geometry* geom = BVHTree->intersect(*temp);
	// Guarantee that this cast will work because
	// BVHTree in trimesh is populated only with
	// trimeshFaces
	if(geom != nullptr) {
		TrimeshFace* trimeshFace = dynamic_cast<TrimeshFace*>(geom);
		if(trimeshFace != nullptr && trimeshFace->intersectLocal(r, i)) {
			have_one = true;
		}
	}

	if (!have_one)
		i.setT(1000.0);
	return have_one;
}

void Trimesh::initBVHTree() 
{
	std::vector<TrimeshFace *> castedFaces = (std::vector<TrimeshFace *>)faces;
	std:vector<Geometry*> geomFaces;

	// These new lists will go out of scope, but
	// the underlying pointer to the faces list
	// will be ultimately saved in the node (Hopefully)
	for(auto face: castedFaces)
	{
		// face *is a* geometry
		geomFaces.push_back(face);
	}
	BVHTree = new BVH(geomFaces);
}

bool TrimeshFace::intersect(ray& r, isect& i) const
{
	return intersectLocal(r, i);
}

// Intersect ray r with the triangle abc.  If it hits returns true,
// and put the parameter in t and the barycentric coordinates of the
// intersection in u (alpha) and v (beta).
bool TrimeshFace::intersectLocal(ray& r, isect& i) const
{
	// YOUR CODE HERE
	//
	// FIXME: Add ray-trimesh intersection

	glm::dvec3 p = r.getPosition();
	glm::dvec3 d = r.getDirection();
	glm::dvec3 n = this->normal;

	glm::dvec3 a = parent->vertices[ids[0]];
	glm::dvec3 b = parent->vertices[ids[1]];
	glm::dvec3 c = parent->vertices[ids[2]];

	/* Code for finding correct value of t
		Modeled after Badouel's Algorithm 
	   https://citeseerx.ist.psu.edu/viewdoc/download?doi=10.1.1.127.8440&rep=rep1&type=pdf
	*/
	double distance = (glm::dot(-a, n));		
	double nDotPosition = (glm::dot(p, n));
	double nDotDirection = (glm::dot(d, n));
	double t = -1 * ((distance + nDotPosition) / nDotDirection);

	/* Also works, and I still have no idea why, but it's cleaner */
	// double t = glm::dot(( a - p ), n) / (glm::dot(d, n));

	if( t <= RAY_EPSILON ) {
		return false;
	}

	// o + vt = P
	glm::dvec3 pos = r.at( t );

	// Assumption that "-" operator is overloaded for dvec3
	bool leftOfBA = glm::dot(glm::cross(b-a, pos-a), n) >= -RAY_EPSILON;
	bool leftOfCB = glm::dot(glm::cross(c-b, pos-b), n) >= -RAY_EPSILON;
	bool leftOfAC = glm::dot(glm::cross(a-c, pos-c), n) >= -RAY_EPSILON;

	if(leftOfBA && leftOfCB && leftOfAC) {
		// printf("Intersection");

		// Need to do barycentric interpolation if no per-vertex normals
		glm::dvec3 newNorm = this->normal;
		Material newMaterial = this->getMaterial();

		// Barycentric Interpolation
		double topLeft = glm::dot(b - a, b - a);
		double topRight = glm::dot(c - a, b - a);
		double bottomLeft = glm::dot(b - a, c - a);
		double bottomRight = glm::dot(c - a, c - a);

		// How does constructor assign vals in matrix?
		glm::mat2x2 matrixAInverse = glm::inverse(glm::mat2x2(topLeft, bottomLeft, topRight, bottomRight));

		// center of mass => intersection point => pos
		double top = glm::dot(b - a, pos - a);
		double bottom = glm::dot(c - a, pos - a);
		// vec2 (instead of dvec2) is a column vector
		glm::vec2 matrixB = glm::dvec2(top, bottom);
		// TODO: Validate Coordinates
		glm::vec2 coords = matrixAInverse * matrixB;

		glm::dvec3 weights = glm::dvec3(1 - coords[0] - coords[1], coords[0], coords[1]);
		i.setBary(weights);

		if(parent->vertNorms) {
			// New "Smooth" normal with barycentric coords as weights
			glm::dvec3 aNorm = (double)weights[0] * parent->normals[ids[0]];
			glm::dvec3 bNorm = (double)weights[1] * parent->normals[ids[1]];
			glm::dvec3 cNorm = (double)weights[2] * parent->normals[ids[2]];
			newNorm = glm::normalize(aNorm + bNorm + cNorm);
		} 

		if(parent->materials.size() > 0) 
		{
			Material material1 = *(parent->materials)[ids[0]];
			Material material2 = *(parent->materials)[ids[1]];
			Material material3 = *(parent->materials)[ids[2]];
			
			newMaterial = (double)weights[0] * material1;
			newMaterial += (double)weights[1] * material2;
			newMaterial += (double)weights[2] * material3;
		}

		i.setObject(this);
		i.setT(t);
		i.setMaterial(newMaterial);
		i.setN(newNorm);
		return true;
	}

	return false;
}

// Once all the verts and faces are loaded, per vertex normals can be
// generated by averaging the normals of the neighboring faces.
void Trimesh::generateNormals()
{
	int cnt = vertices.size();
	normals.resize(cnt);
	std::vector<int> numFaces(cnt, 0);

	for (auto face : faces) {
		glm::dvec3 faceNormal = face->getNormal();

		for (int i = 0; i < 3; ++i) {
			normals[(*face)[i]] += faceNormal;
			++numFaces[(*face)[i]];
		}
	}

	for (int i = 0; i < cnt; ++i) {
		if (numFaces[i])
			normals[i] /= numFaces[i];
	}

	vertNorms = true;
}

