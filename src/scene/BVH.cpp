#include "BVH.h"
#include "scene.h"
#include "../SceneObjects/trimesh.h"
#include <float.h>
#include <algorithm>

BVH::BVH(std::vector<Geometry*> &objects)
{   
    if(objects.size() > 0)
        root = buildTree(objects, 0, objects.size());
    else   
        root = new BVHNode();
    
    // Maintain a orderedObjects list which places leaf
    // nodes in contiguous segments of memory
    // objects.swap(orderedObjects);   
}

BVH::BVH()
{
    // No op
}

BVH::~BVH()
{
    // Need to Free all allocated nodes here through DFS
    freeNodes(root);
}

void BVH::freeNodes(BVHNode* node) {
    if(node->getLeft() != nullptr)
        freeNodes(node->getLeft());
    if(node->getRight() != nullptr)
        freeNodes(node->getRight());
    
    delete node;
}

// TODO: BVH::buildTree();
// We need list of all objects in scene
// Pseudocode:
/*      
    If list of objects contains only one object
        Create leaf node which contains this object and return it
    
    Compute Bounding box based on objects in list.
    
    Now find longest axis (how?) then split objects along this line
    => objects on both sides of line go in both subsequent lists of objects
    => leftObjects and rightObjects
    return bounding box with left node: buildTree(leftObjects) and rightNode: buildTree(rightObjects) 
*/
// Notes:
// Should store Trimesh as single object that gets stored in leaf
// nodes just like any other object. However, need to add BVH tree
// component inside each trimesh object. Whenever we are creating 
// a leaf node, if it's a trimesh, we can call the trimesh specific
// build tree. This will be almost identical to general buildTree
// but based on individual trimeshFaces instead. Similarly, when 
// object found by traverse is trimesh, just call the trimesh 
// specific traverse on it and return that value.

BVHNode* BVH::buildTree(std::vector<Geometry*> &objects,
                        int start, int end) {
    BVHNode *node = new BVHNode();

    // Compute Bounding Box based on objects in list, top-down
    BoundingBox bounds = objects[start]->getBoundingBox();
    for(int i = start; i < end; i++)
    {
        bounds.merge(objects[i]->getBoundingBox());
    }

    int numObjects = end - start;
    if(numObjects == 1) {
        // Create Leaf Node
        node->initializeLeafNode(objects[start], bounds);
        return node;
    } else {

        // Splitting against longest axis
        BoundingBox centerBoundingBox = BoundingBox();
        for(int i = start; i < end; i++) {
            glm::dvec3 point = 0.5 * objects[i]->getBoundingBox().getMin()
                            + 0.5 * objects[i]->getBoundingBox().getMax();
            centerBoundingBox.merge(point);
        }
        int longestAxis = centerBoundingBox.MaxExtent();

        int mid = (start + end) / 2;

        // Splitting Heuristic - Split into equally sized subsets
        std::nth_element(&objects[start], &objects[mid], &objects[end - 1] + 1, 
            [longestAxis](const Geometry* a, const Geometry* b) 
            {
                glm::dvec3 centerA = 0.5 * a->getBoundingBox().getMin()
                            + 0.5 * a->getBoundingBox().getMax();

                glm::dvec3 centerB = 0.5 * b->getBoundingBox().getMin()
                            + 0.5 * b->getBoundingBox().getMax();

                return centerA[longestAxis] < centerB[longestAxis];
            });

        BVHNode* left = buildTree(objects, start, mid);
        BVHNode* right = buildTree(objects, mid, end);
        node->initializeInterior(left, right, bounds);
    }
    
    return node;
}

bool BVH::intersect(ray& r, isect& i) {
    IsectHelperStruct result = traverse(r, i, root);
    if(result.object != nullptr) {
        i = result.i;
        return true;
    }
    return false;
}

// TODO: BVH::traverse(ray r);
// Traverses the BVH Tree and returns the object stored
// in the leaf node at which traversal ends. 
// How to check for case where ray intersects with 
// certain boxes but never hits an object?

// pbr recommends compacting tree into 
// array representation but skipping
// that to save time and am unsure how
// generalize to trimesh faces
IsectHelperStruct BVH::traverse(ray& r, isect& i, BVHNode* node) {

    IsectHelperStruct ret = {nullptr, DBL_MAX};
    if(node == nullptr)
        return ret;

    double tMin, tMax;
    bool intersect = node->getBoundingBox().intersect(r, tMin, tMax);
    
    // No intersection
    if(!intersect)
        return ret;

    ret.tMin = tMin;
    // printf("tMin: %f\n", tMin);

    // node is a leaf node
    if(node->getGeom() != nullptr) {
        // Finally found an actual intersection
        if(node->getGeom()->intersect(r, i)) {
            // printf("Intersections galore!\n");
            ret.object = node->getGeom();
            ret.i = i;
        }
        return ret;
        // printf("Found Geom: %p\n", ret.object);
    }

    // if(node->getLeft() == nullptr && node->getRight() == nullptr)
    //     printf("This should never happen.\n");

    // There was an intersection, but not a leaf.
    // Therefore, need to travel deeper down the tree
    IsectHelperStruct leftResult = traverse(r, i, node->getLeft());
    IsectHelperStruct rightResult = traverse(r, i, node->getRight());

    if(leftResult.object == nullptr && rightResult.object == nullptr)
    {
        // Ray didn't hit anything after current intersection
        return ret;
    }
    else if(leftResult.object != nullptr && rightResult.object == nullptr) {
        return leftResult;
    } else if(leftResult.object == nullptr && rightResult.object != nullptr) {
        return rightResult;
    } else {
        // Return the object which was hit first/earlier
        if(rightResult.tMin < leftResult.tMin)
            return rightResult;
        else
            return leftResult;
    }
}

// BVHNode Methods
void BVHNode::initializeLeafNode(Geometry* geomObject) {
    
    if(Trimesh* trimesh = dynamic_cast<Trimesh*>(geomObject)) {
        // geomObject was safely casted to Trimesh
        // printf("Making the trimesh BVH\n");
        trimesh->initBVHTree();
    } 

    geom = geomObject;
    boundingBox = geomObject->getBoundingBox();
}

void BVHNode::initializeLeafNode(Geometry* geomObject, BoundingBox bb) {
    initializeLeafNode(geomObject);
    boundingBox = bb;
}

// Computes bounding box bottom-up 
void BVHNode::initializeInterior(BVHNode *leftNode, BVHNode *rightNode) {
    left = leftNode;
    right = rightNode;

    // New bounding box which compasses objects of both nodes
    boundingBox = leftNode->boundingBox;
    boundingBox.merge(rightNode->boundingBox);
}

// Compute Bounding Box top-down outside this method
// then pass it in  here
void BVHNode::initializeInterior(BVHNode *leftNode, BVHNode *rightNode, BoundingBox bb) {
    left = leftNode;
    right = rightNode;

    boundingBox = bb;
}