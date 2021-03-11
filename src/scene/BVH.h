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
    bool intersect(ray& r, isect& i);
    IsectHelperStruct traverse(ray& r, isect& i, BVHNode* node);
    void freeNodes(BVHNode* node);
    BVHNode* getRoot() { return root; }

private:
    BVHNode* root;
    int totalNodes;
};

struct IsectHelperStruct {
    Geometry* object;
    double tMin;
    isect i;
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
    void initializeLeafNode(Geometry* geomObject, BoundingBox bb);

    void initializeInterior(BVHNode *leftNode, BVHNode *rightNode);
    void initializeInterior(BVHNode *leftNode, BVHNode *rightNode, BoundingBox bb);

    BoundingBox getBoundingBox() { return boundingBox; }
    Geometry* getGeom() { return geom; }
    BVHNode* getLeft() { return left; }
    BVHNode* getRight() { return right; }
};