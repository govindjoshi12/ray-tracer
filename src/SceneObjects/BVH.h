#pragma once

#include <glm/vec3.hpp>
#include <vector>
#include <memory>
#include "scene.h"

class BVH {

public:
	BVH(std::vector<std::unique_ptr<Geometry>> objects);
	~BVH();

    BVHNode buildTree(std::vector<std::unique_ptr<Geometry>> objects);
    Geometry* traverse(ray& r);

private:
    BVHNode root;
};

class BVHNode {

// BVHNode assumes BVH Tree is a binary tree
public: 
    BVHNode();
    ~BVHNode();

    void setBoundingBox(BoundingBox bb) { boundingBox = bb; }
    BoundingBox getBoundingBox() { return boundingBox; }

    // If null, then not a leaf node. Else,
    // It's a leaf node.
    void setGeometry(Geometry *geom) { geometry = geom; }
    Geometry* getGeometry() { return geometry; } 

    void setLeft(BVHNode *leftNode) { left = leftNode; }
    BVHNode* getLeft() { return left; }

    void setRight(BVHNode *rightNode) { right = rightNode; }
    BVHNode* getRight() { return right; }

private:
    BoundingBox boundingBox;
    Geometry *geometry = nullptr;
    BVHNode *left = nullptr;
    BVHNode *right = nullptr;  
}