#pragma once

#include <glm/vec3.hpp>
#include <vector>
#include <memory>
#include "bbox.h"
#include "ray.h"

// Forward Declaring this class
class Geometry;
class BVHNode;
class BVH;
struct IsectHelperStruct;

class BVH {

public:
	BVH(std::vector<Geometry*> &objects);
	BVH();
    ~BVH();

    BVHNode* buildTree(std::vector<Geometry*> &objects, 
                        int start, int end);
    Geometry* intersect(const ray& r);
    IsectHelperStruct traverse(const ray& r, BVHNode* node);
    void freeNodes(BVHNode* node);
    BVHNode* getRoot() { return root; }

private:
    BVHNode* root;
    int totalNodes;
};

struct IsectHelperStruct {
    Geometry* object;
    double tMin;
};

class BVHNode {

private:
    BoundingBox boundingBox;
    Geometry* geom = nullptr;
    BVHNode *left = nullptr;
    BVHNode *right = nullptr;  

public: 
    BVHNode() { }
    ~BVHNode() { }

    void initializeLeafNode(Geometry* geomObject);
    void initializeInterior(BVHNode *leftNode, BVHNode *rightNode);

    BoundingBox getBoundingBox() { return boundingBox; }
    Geometry* getGeom() { return geom; }
    BVHNode* getLeft() { return left; }
    BVHNode* getRight() { return right; }
};