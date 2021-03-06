#ifndef COLLISION_HPP_
#define COLLISION_HPP_

#include "gfx.hpp"
#include "utils.hpp"
#include "t_utils.hpp"

// This constant determines how many objects should be added to a 
// OctTree space before the objects in that space are partitioned.
#define OCTTREE_MAX_NODE_BOX_COLLISIONS 3
// This constant determines the point at which OctTree boxes stop being subdivided
#define OCTTREE_BOX_COLLISION_GRANULARITY 5.0
// This constant determines the default maximum number of OctTree spaces
// a CollideObject can be partitioned into
#define OCTTREE_BOX_DIVISION_LIMIT 8

#define ENTITY_COLLISION_TYPE_BOUNDING_BOX 0x01
#define ENTITY_COLLISION_TYPE_MESH 0x02

class OctTree;
class CollideSpace;
class Model;
class PhysicsEntity;

class BoundBox {
public:
	Vector3f pos;
	Vector3f extents;

	void draw();
};

class CollideObject {
public:
	// The bounding box for the entity
	// Other shapes may follow
	BoundBox oldBoundBox;
	BoundBox boundBox;

	Model* model;

	// Holds the space that completely encloses this object
	OctTree* container;
	// Holds the physicsEntity that is related to this object
	PhysicsEntity* physicsEntity;

	// Holds a list of spaces the entity currently resides in
	DLinkedList<OctTree*>* spaceList;
	// Holds a list of collision spaces which were found for the entity
	DLinkedList<CollideSpace*>* collideSpaceList;

	// Holds the number of spaces remaining for the entity to be partitioned into
	int remainingSpaces;
	// Holds the types of collisions that will be calculated for the entity
	int collisionTypes;

public:
	CollideObject() { init(); }
	virtual void init(); /* FIXME: virtual functions should not be called from constructor */

	CollideObject(BoundBox& box, int collisionTypes){ init(box, collisionTypes); }
	virtual void init(BoundBox& box, int collisionTypes); /* FIXME: virtual functions should not be called from constructor */

	virtual ~CollideObject(){ cleanUp(); }
	virtual void cleanUp();

	CollideObject & operator=(CollideObject c);
};

struct CollideInfo {
	BoundBox box;
	Vector3f normal;
};


/**
 *  The OctTree class is a recursive structure, it represents a space with a given 
 * extent and position.  A user should only create OctTrees with NULL parents,
 * because the OctTree will partition itself as it needs to.  Any attempt to mess 
 * with the tree otherwise will probably result in serious, serious pain in the form
 * of untraceable pointer errors..
 */
class OctTree {
public:
	//FIXME: Should all be private with appropriate accessors.
	/**
	 * Center of space.
	 */
	Vector3f actualPos;
	// X, Y, and Z extent of space radiating from the origin
	float extent;
	// Subspaces
	OctTree** subTrees;
	// Parent space
	OctTree* parentTree;
	// List of objects to collision detect in this space
	DLinkedList<CollideObject*>* objectList;

protected:
	OctTree(OctTree *parent, Vector3f &actualPos, float extent);

	// Initializes the tree
	void init(OctTree* parent, Vector3f& actualPos, float extent);
	
	// Determines how to handle the addition of a new object into this
	// occupied space.  (Lots o' logic)
	bool resolveCollideList(CollideObject* object, bool doCollide);
	
	// Adds an object to certain subspaces
	DLinkedList<OctTree*>* partitionIntoSubspaces(CollideObject* object, unsigned int subspaces);

	// Triangle partitioning functions
	DLinkedList<int>* borderTrianglesIntersecting(OctTree* space, CollideObject* object, DLinkedList<int>** triangleIndices);
	DLinkedList<int>* trianglesInside(OctTree* space, CollideObject* object, DLinkedList<int>** triangleIndices);

	// Returns the subspaces a box occupies in this space
	unsigned int partitionBoundingBox(CollideObject* object);
	
	// Performs local collision detection
	bool collideObjectLocal(CollideObject* object);

	// Check to see whether an object is completely enclosed by this space
	bool encloses(BoundBox& object);
	// Check to see whether a point is contained in this space
	bool contains(Vector3f* point);
	// Check to see whether an object completely encloses this space
	bool isEnclosedBy(BoundBox& object);

	// To be used later?
	bool approximateShapeCollision();
	bool actualShapeCollision();

public:
	/**
	 * Creates an OctTree with NULL parent, which represents a cube of 
	 * space centered at actualPos, with radius of extent in all three
	 * directions.
	 */
	OctTree(Vector3f &actualPos, float extent);
	virtual ~OctTree();

	
	/**
	 * NOT similar to UpdateObject, checks an object for collision before
	 * it is added to any collision space.  This method can be used in the
	 * same way as the CheckObject(object) function from the current object
	 * space root or object container, but will probably be slower because
	 * container information is NOT used.  Recommend using the static 
	 * function for objects already in collision space.
	 *
	 * Returns true if a collision occurred, and the
	 * CollideObject's collideList is updated.
	 */
	virtual bool CheckObjectHere(CollideObject* object);

	/**
	 * OpenGL drawing function
	 */
	void draw(CGparameter color);

	/* STATIC UTILITY FUNCTIONS 
	 *
	 * These functions need to be static, because they can be used to
	 * modify and update any OctTree.  Multiple collision detection
	 * "worlds" are possible in this way.
	 * FIXME: Do these *really* need to be static?
	 */

	/**
	 * Adds an object to this particular space.
	 * Note:  This function should only be called on the 
	 * root space, i.e. parent = NULL.  Attempts to call this
	 * function for a subspace can result in missed collisions.
	 *
	 * Returns true if a collision occurred, and the
	 * CollideObject's collideList is updated.
	 */
	static bool AddObject(OctTree* space, CollideObject* object);
	static bool AddObject(OctTree* space, CollideObject* object, bool doCollide); 

	/**
	 * Checks a modified CollideObject for new collisions in the 
	 * space it is contained in.  Also reorganizes the OctTree
	 * space to better fit the new position.
	 *
	 * Returns true if a collision occurred, and the
	 * CollideObject's collideList is updated.
	 */
	static bool UpdateObject(CollideObject* object);
	static bool UpdateObject(CollideObject* object, bool doCollide);
	
	/**
	 * Removes an object completely from the space it is contained
	 * in.  Returns false if the object was not contained in any space.
	 */
	static bool RemoveObject(CollideObject* object);

	// Returns the number of spaces an object currently occupies in the OctTree
	static int GetNumSpaces(CollideObject* object);

	// Returns the number of objects an OctTree currently holds
	static int GetNumObjects(OctTree* space);
	
	// Quick absf function
	static float absf(float x);

	// Quick sign function
	static int sign(float x);

protected:
	// Adds an object to this particular space
	static bool addObject(OctTree* space, CollideObject* object, bool doCollide);

	static CollideMap<CollideObject*, CollideObject*, bool> collideMap;

public: //FIXME: WHY
	static CollideMap<CollideObject*, CollideObject*, bool> excludeMap;

protected:
	// Used to store mesh triangle locations in OctTrees
	static CollideMap<OctTree*, CollideObject*, DLinkedList<int>*> meshMap;

	/*
	 * Disables a space from detecting collisions.
	 * This function does not REMOVE the space from the 
	 * tree, It simply removes a space from detecting collisions with any
	 * objects currently in the tree.  Updating objects will allow them to
	 * collide in this space again.  UNTESTED, used internally..
	 */
	static void RemoveSpace(OctTree* space);

	// Decodes and returns the number of subspaces something occupies
	static int countSubspaces(unsigned int subspaces);
	
	// Collides an object recursively downward through a given space's 
	// subspaces
	static bool collideObjectRecursiveDown(CollideObject* object, OctTree* space);
	// Collides an object recursively upward through a given space's 
	// parents
	static bool collideObjectRecursiveUp(CollideObject* object, OctTree* space);
	
	// Adds an object to the local object database
	static void addToSpace(CollideObject* object, OctTree* space);
	// Removes an object from the local object database
	static void removeFromSpace(CollideObject* object, OctTree* space, bool removeIfEmpty);
	// Remove a space from the tree, if safe to do so
	static void safelyRemoveSubspace(OctTree* subspace, const OctTree* container);

	// Determines whether an object is in a particular space, and
	// the entry in which it is stored.
	static DLinkedList<CollideObject*>* 
		findObjectEntry(OctTree* space, CollideObject* object);
	// Determines whether an object is contained in a particular space,
	// and the entry in which it is stored.
	static DLinkedList<OctTree*>* 
		findSpaceEntry(CollideObject* object, OctTree* space);

	// Check to see whether two bounding boxes collide, and if so, give the
	// CSG intersection
	static CollideInfo* boundingBoxCollision(CollideObject* objA, CollideObject* objB);
	//
	static DLinkedList<CollideInfo*>* boundingBoxMeshCollision(CollideObject *objA, CollideObject *objB, OctTree* space);

	static bool addInitialTriangles(OctTree* space, CollideObject* object);
};

class CollideSpace {
public:
	//FIXME: should be private
	CollideObject* objectA;
	CollideObject* objectB;
	//BoundBox boundBox;
	CollideInfo info;

private:
	void init(OctTree* parent, CollideObject* objectA, CollideObject* objectB, CollideInfo& c);

public:

	CollideSpace(OctTree* parent, CollideObject* objectA, CollideObject* objectB, CollideInfo& c);
	virtual ~CollideSpace();
};
#endif /* COLLISION_HPP_ */

