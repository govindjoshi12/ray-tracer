#include "BVH.h"

BVH::BVH(std::vector<std::unique_ptr<Geometry>> objects)
{
    root = buildTree(objects);
}

BVH::~BVH()
{
}

// TODO: BVH::traverse(ray r);
// Traverses the BVH Tree and returns the object stored
// in the leaf node at which traversal ends. 
// How to check for case where ray intersects with 
// certain boxes but never hits an object?


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

BVHNode BVH::buildTree(std::vector<std::unique_ptr<Geometry>> objects) {
    if(objects.size() <= 1) {
        // Create Leaf Node
        // If trimesh, build BVH for trimeshFaces
    }
    
    /* Calculate bounding box of list of objects */
    // Safer than using arbitrary vectors to initialize
    glm::dvec3 bMin = objects[0]->getBoundingBox().getMin();
    glm::dvec3 bMax = objects[0]->getBoundingBox().getMax();

    // This is inefficient, we should be sorting
    // But how? Based on what heuristic? Do we need 
    // two sorts, one based on min, and other on max?
    for(const auto& obj : objects) { 
        bMin = glm::min(obj->getBoundingBox().getMin(), bMin);
        bMax = glm::max(obj->getBoundingBox().getMax(), bMax);
    }

    // Create top-level bounding box
    BoundingBox bb = BoundingBox(bMin, bMax);
    BVHNode parent = BVHNode();
    parent.setBoundingBox(bb);
    
    // Split object list based on heuristic

    return parent;
}
