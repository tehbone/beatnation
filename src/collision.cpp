/************************************
*	Versioning Information			*
************************************/



// Collision Detection functions and methods
#include <malloc.h>
#include <stdio.h>
#include <math.h>
#include <cstdlib>
#include <windows.h>
#include <gl/gl.h>
#include <gl/glaux.h>
//#include "memtrack.cpp"
#include "gfx.hpp"
#include "collision.hpp"
#include "model.hpp"
#include "t_utils.cpp"


CollideMap<CollideObject*, CollideObject*, bool> OctTree::collideMap(false);
CollideMap<CollideObject*, CollideObject*, bool> OctTree::excludeMap(false);
CollideMap<OctTree*, CollideObject*, DLinkedList<int>*> OctTree::meshMap(HASHMAP_DEFAULT_BUCKETS, NULL);

void hashmapTest();
void octtreeTest();
void pause();

//*********************************************************************************
void pause()
//*********************************************************************************
{
	long int sum = 30000;
	for(int i = 0; i < 30000; i++) printf("Pausing...\n");
}

//*********************************************************************************
void hashmapTest()
//*********************************************************************************
{

	printf("Testing Hashmap...");

	long int q;
	long int numObjs = 700000;

	for(q = 0; q < numObjs; q++){

		long int i;
		long int numReps = 20000;
	
		HashMap<float, float>* map = new HashMap<float, float>((float) 0.0);
		for(i = 0; i < numReps; i++)
			//map->Put((float) i, (float) i + 1);

		//map->printDistribution();

		printf("all put...");
		for(i = numReps / 2; i < numReps; i++)
			map->Get((float) i);

		delete map;
	}
	

}




//---------------------------------------------------------------------------------
OctTree::OctTree(Vector3f& actualPos, float extent)
//---------------------------------------------------------------------------------
{
	init(NULL, actualPos, extent);
}

//---------------------------------------------------------------------------------
OctTree::OctTree(OctTree* parent, Vector3f& actualPos, float extent)
//---------------------------------------------------------------------------------
{
	init(parent, actualPos, extent);
}

//---------------------------------------------------------------------------------
OctTree::~OctTree()
//---------------------------------------------------------------------------------
{
	// Delete tree (if it exists)
	//printf("Deleting...");
	if(subTrees != NULL){
		int i;
		for(i = 0; i < 8; i++){
			if(subTrees[i] != NULL){
				delete subTrees[i];
				subTrees[i] = NULL;
			}
		}

		delete [] subTrees;
		subTrees = NULL;
	}
	
	// Remove from position maps
	RemoveSpace(this);

	// Remove object list
	DLinkedList_DeleteTail<CollideObject*>(objectList);
	objectList = NULL;

	//printf("Deleting tree: (%f, %f, %f)\n", actualPos.x, actualPos.y, actualPos.z);

	//printf("Deleting 2...");
}

//---------------------------------------------------------------------------------
bool OctTree::AddObject(OctTree* space, CollideObject* object)
//---------------------------------------------------------------------------------
{
	return AddObject(space, object, true);
}


//---------------------------------------------------------------------------------
bool OctTree::AddObject(OctTree* space, CollideObject* object, bool doCollide)
//---------------------------------------------------------------------------------
{
	// Assumption:  The object is always inserted into a box that is big enough for
	// that object.  Since we should only insert in the root space, this should hold.
//	MEMORY_TRACKING = true;
	object->container = space;
	
	bool meshObject = ((object->collisionTypes & ENTITY_COLLISION_TYPE_MESH) != 0);
	bool trianglesFound = addInitialTriangles(space, object);

	if(!meshObject){
		trianglesFound = trianglesFound;
	}

	bool returnValue = (trianglesFound || !meshObject) &&
		               addObject(space, object, doCollide);

	// Empty the collision DB for the next items
	collideMap.RemoveAll();

	return returnValue;
}

//---------------------------------------------------------------------------------
bool OctTree::addObject(OctTree* space, CollideObject* object, bool doCollide)
//---------------------------------------------------------------------------------
{

	bool returnValue = false;

	DLinkedList<OctTree*>* fringe = NULL;
    fringe = DLinkedList_AddHeadEntry<OctTree*>(fringe, space, 10);
	DLinkedList<OctTree*>* currFringeEntry = fringe;
	DLinkedList<OctTree*>* tail = fringe;

	object->remainingSpaces--;	// Subtract 1 for the first space this object is put into.
	object->container = space;

	while(currFringeEntry != NULL){
		
		OctTree* currSpace = currFringeEntry->object;

		// Determine if this object can be the object's container
		if(currSpace->encloses(object->boundBox)) object->container = currSpace;
		
		// Figure out how many subspaces we can partition the object into
		unsigned int objectSpaces = currSpace->partitionBoundingBox(object);
		int numSpaces = countSubspaces(objectSpaces);

		// Determine whether we can partition the object, and if not, move on to the
		// next object.
		bool canPartitionObject = 
			(object->remainingSpaces - numSpaces + 1 >= 0);
		if(!canPartitionObject){
			currFringeEntry = currFringeEntry->nextList;
			continue;
		}
		
		// Properties of the object and space that we should figure out.
		bool hasSubspaces = (currSpace->subTrees != NULL);
		bool largeBoxes = (currSpace->extent > OCTTREE_BOX_COLLISION_GRANULARITY);
		bool tooManyLocalCollisions =
			(GetNumObjects(currSpace) + 1 > OCTTREE_MAX_NODE_BOX_COLLISIONS);

		// Heavy objects drop to the bottom of the tree right away
		bool heavyObject = 
			((object->collisionTypes & ENTITY_COLLISION_TYPE_MESH) != 0);

		// Determine whether the object should be partitioned.
		bool partitionObject = 
			(heavyObject && 
			 // always a reason to partition heavy objects
			 //...and if the boxes aren't getting too small.
			 largeBoxes
			)
		  || 
			(
			  !heavyObject &&
			  // ,,,if there is a reason to partition
			  (tooManyLocalCollisions || hasSubspaces) &&
			  // ...and if partitioning will help
			  !currSpace->isEnclosedBy(object->boundBox) && 
			  //...and if the boxes aren't getting too small.
			  largeBoxes
			);

		//if(tooManyLocalCollisions) printf("Too many local collisions.\n");

		//if(partitionObject) printf("Partitioning added object into %d subspaces...\n", numSpaces);
		//else printf("Not partitioning object into %d subspaces...\n", numSpaces);

		bool partitionList = 
			// ...if there is a reason to partition the list
			(tooManyLocalCollisions || hasSubspaces) &&
			//...and if the boxes aren't getting too small.
			largeBoxes;


		if(partitionObject){
			
			if(doCollide){
				returnValue = currSpace->collideObjectLocal(object) | returnValue;
			}

			DLinkedList<OctTree*>* subspaces =
				currSpace->partitionIntoSubspaces(object, objectSpaces);

			if(subspaces != NULL){

				object->remainingSpaces = 
					object->remainingSpaces - *(subspaces->listSize) + 1;

				if(object->remainingSpaces < 0){
					returnValue = returnValue;
				}

				// Add subspaces to the END of the fringe
				DLinkedList<OctTree*>* newTail = DLinkedList_GetTail<OctTree*>(subspaces);
				fringe = DLinkedList_AddAfter<OctTree*>(fringe, tail, subspaces);
				tail = newTail;

				// Remove the currFringeEntry from the fringe
				DLinkedList<OctTree*>* currEntry = currFringeEntry;
				currFringeEntry = currFringeEntry->nextList;
				fringe = DLinkedList_Remove<OctTree*>(fringe, currEntry);
				DLinkedList_DeleteTail<OctTree*>(currEntry); currEntry = NULL;

			}


		}
		else{
			currFringeEntry = currFringeEntry->nextList;
			continue;
		}


	}

	// Finish up downward collisions, add the object to the fringe spaces
	for(currFringeEntry = fringe;
		currFringeEntry != NULL;
		currFringeEntry = currFringeEntry->nextList)
	{
		OctTree* currSpace = currFringeEntry->object;

		if(doCollide){
			returnValue = currSpace->collideObjectLocal(object) | returnValue;
			returnValue = collideObjectRecursiveDown(object, currSpace) | returnValue;
		}

		//*************************************
		// New collision object added here
		//*************************************
		addToSpace(object, currSpace);
	}

	DLinkedList_DeleteTail<OctTree*>(fringe); fringe = NULL; tail = NULL; currFringeEntry = NULL;

	return returnValue;
}

//---------------------------------------------------------------------------------
inline bool OctTree::addInitialTriangles(OctTree* space, CollideObject* object)
//---------------------------------------------------------------------------------
{
	if(object->model != NULL){
		Model::Mesh mesh = *(object->model->meshes);
		int numTriangles = mesh.numTriangles;
		DLinkedList<int>* triangles = NULL;
		
		for(int i = 0; i < numTriangles; i++)
			triangles = DLinkedList_AddHeadEntry(triangles, i, 11);

		MultiKey<OctTree*, CollideObject*> key;
		key.keyA = space;
		key.keyB = object;
		meshMap.Put(key, triangles);
		
		return true;
	}
	return false;
}

//---------------------------------------------------------------------------------
inline void OctTree::addToSpace(CollideObject* object, OctTree* space)
//---------------------------------------------------------------------------------
{

	// Add to space/object list
	space->objectList = DLinkedList_AddHeadEntry<CollideObject*>(space->objectList, object, 12);
	// Add to object/space list
	object->spaceList = DLinkedList_AddHeadEntry<OctTree*>(object->spaceList, space, 13);
	
}


//---------------------------------------------------------------------------------
DLinkedList<CollideObject*>* OctTree::findObjectEntry(OctTree* space, CollideObject* object)
//---------------------------------------------------------------------------------
{
	for(DLinkedList<CollideObject*>* currEntry = space->objectList;
		currEntry != NULL; currEntry = currEntry->nextList)
	{
		if(currEntry->object == object) return currEntry;
		
	}
	return NULL;
}

//---------------------------------------------------------------------------------
DLinkedList<OctTree*>* OctTree::findSpaceEntry(CollideObject* object, OctTree* space)
//---------------------------------------------------------------------------------
{
	for(DLinkedList<OctTree*>* currEntry = object->spaceList;
		currEntry != NULL; currEntry = currEntry->nextList)
	{
		if(currEntry->object == space) return currEntry;
		
	}
	return NULL;
}

//---------------------------------------------------------------------------------
inline int OctTree::GetNumSpaces(CollideObject* object)
//---------------------------------------------------------------------------------
{
	return object->spaceList != NULL ? *(object->spaceList->listSize) : 0;
}

//---------------------------------------------------------------------------------
inline int OctTree::GetNumObjects(OctTree* space)
//---------------------------------------------------------------------------------
{
	return space->objectList != NULL ? *(space->objectList->listSize) : 0;
}

//---------------------------------------------------------------------------------
bool OctTree::RemoveObject(CollideObject* object)
//---------------------------------------------------------------------------------
{
	DLinkedList<OctTree*>* currEntry = object->spaceList;

	if(object->spaceList == NULL) return false;

	meshMap = meshMap;

	collideMap = collideMap;

	while(currEntry != NULL)
	{
		OctTree* currSpace = (OctTree*) currEntry->object;
		currEntry = currEntry->nextList;
		removeFromSpace(object, currSpace, true);

		object->remainingSpaces++;
	}

//	MEMORY_TRACKING = false;
//	DumpUnfreed();

	meshMap = meshMap;

	return true;
}	

//---------------------------------------------------------------------------------
void OctTree::safelyRemoveSubspace(OctTree* subspace, const OctTree* container)
//---------------------------------------------------------------------------------
{
	/*
	 * This function will check to make sure that there is nothing in this subspace, 
	 * that it is not a container, that it is not the root space, and that it has
	 * no subtrees before deleting anything.
	 */

	if(subspace == NULL) return;
	OctTree* space = subspace->parentTree;

	if(subspace->objectList != NULL ||
	   subspace->subTrees != NULL ||
	   space == NULL ||
	   subspace == container ||
	   space->subTrees == NULL) return;
	
	int i;
	bool spaceFound = false;
	for(i = 0; i < 8; i++){
		OctTree* subTree = space->subTrees[i];
		if(subTree == subspace){
			delete subTree;
			space->subTrees[i] = NULL;
		}
		else spaceFound = spaceFound | (subTree != NULL);
	}

	// Delete the current space too if nothing's in it...
	if(!spaceFound){
		delete [] space->subTrees;
		space->subTrees = NULL;

		safelyRemoveSubspace(space, container);
	}
}

//---------------------------------------------------------------------------------
void OctTree::RemoveSpace(OctTree* space)
//---------------------------------------------------------------------------------
{
	DLinkedList<CollideObject*>* currEntry = space->objectList;

	while(currEntry != NULL){
		//printf("CurrEntry: %d", currEntry);
		CollideObject* currObject = currEntry->object;

		currObject->remainingSpaces++;

		currEntry = currEntry->nextList;

		// Remove from space/object list
		currObject->spaceList = DLinkedList_DeleteEntry<OctTree*>(currObject->spaceList, space);
		// Remove from object/space list
		space->objectList = DLinkedList_DeleteEntry<CollideObject*>(space->objectList, currObject);
	}
}


//---------------------------------------------------------------------------------
inline void OctTree::removeFromSpace(CollideObject* object, OctTree* space, bool removeIfEmpty)
//---------------------------------------------------------------------------------
{		
	// Remove from space/object list
	object->spaceList = DLinkedList_DeleteEntry<OctTree*>(object->spaceList, space);
		
	// Remove from object/space list
	space->objectList = DLinkedList_DeleteEntry<CollideObject*>(space->objectList, object);

	// Remove triangles from meshMap
	MultiKey<OctTree*, CollideObject*> key;
	key.keyA = space;
	key.keyB = object;
	DLinkedList<int>* triangles = meshMap.Remove(key);
	DLinkedList_DeleteTail<int>(triangles); triangles = NULL;
	
	// Need this check here, b/c sometimes this function is used to quickly remove an item
	// from a space b/f adding to that space's subspaces
	if(removeIfEmpty){
		safelyRemoveSubspace(space, object->container);
		space = NULL;
	}

}


//---------------------------------------------------------------------------------
void OctTree::init(OctTree* parent, Vector3f& actualPos, float extent)
//---------------------------------------------------------------------------------
{
	this->subTrees = NULL;
	this->parentTree = parent;

	this->actualPos = actualPos;
	this->extent = extent;

	this->objectList = NULL;
}

//---------------------------------------------------------------------------------
DLinkedList<OctTree*>* OctTree::partitionIntoSubspaces(CollideObject* object, unsigned int subspaces)
//---------------------------------------------------------------------------------
{
	DLinkedList<OctTree*>* returnSpaces = NULL;

	float boxIncrement = extent / (float) 2.0;

	DLinkedList<int>* triangles = NULL;
	DLinkedList<int>** subTriangles = NULL;
	
	bool partitionTriangles = 
		((object->collisionTypes & ENTITY_COLLISION_TYPE_MESH) != 0);

	if(partitionTriangles){
		
		MultiKey<OctTree*, CollideObject*> key;
		key.keyA = this;
		key.keyB = object;

		triangles = meshMap.Remove(key);
		subTriangles = new DLinkedList<int>*[8];
	}

	unsigned int i;

	// Loop to create all the subspaces and original subtriangle lists

	int n = 0; // Holds # of subtrees found/created in this call
	int j = 8; // Holds current # of subtree in loop, (see subtree diagram below? above?)
	for(i = 0x01; i < 0x90; i = i << 1){ 
		if((subspaces & i) != 0){
			
			// Create subTree array if not already created
			if(subTrees == NULL){
				subTrees = new OctTree*[8];
				for(int q = 0; q < 8; q++) subTrees[q] = NULL;
			}

			//printf("Repartitioning object into subspace #%d...\n", j);

			OctTree* subTree = subTrees[j - 1];
			
			// Create subTree if not already created
			if(subTree == NULL){
				Vector3f newPos;

				if((i & 0xF0) != 0) newPos.z = actualPos.z + boxIncrement; 
				else newPos.z = actualPos.z - boxIncrement;
				if((i & 0xCC) != 0) newPos.x = actualPos.x + boxIncrement;
				else newPos.x = actualPos.x - boxIncrement;
				if((i & 0x99) != 0) newPos.y = actualPos.y + boxIncrement;
				else newPos.y = actualPos.y - boxIncrement;


				subTree = new OctTree(this, newPos, boxIncrement);
				subTrees[j - 1] = subTree;
			}
			
			// Partition mesh into subspace, if necessary
			if(partitionTriangles){
				if(triangles != NULL)
					subTriangles[j - 1] = trianglesInside(subTree, object, &triangles);
				else subTriangles[j - 1] = NULL;
			}
			
			n++;
		}
		j--;
	}


	// Loop to partition all the border triangles
	if(partitionTriangles){
		
		n = 0; // Holds # of subtrees found/created in this call
		j = 8; // Holds current # of subtree in loop, (see subtree diagram below? above?)
		for(i = 0x01; i < 0x90; i = i << 1){ 
			if((subspaces & i) != 0){
				OctTree* subTree = subTrees[j - 1];
					
				DLinkedList<int>* borderTriangles = NULL;
				
				if(triangles){
					borderTriangles = borderTrianglesIntersecting(subTree, object, &triangles);
				}

				if(borderTriangles){

					DLinkedList<int>* subTriangle = subTriangles[j - 1];

					DLinkedList<int>* tail = 
						DLinkedList_GetTail<int>(borderTriangles);	
				
					subTriangle =
						DLinkedList_AddAfter<int>(borderTriangles, tail, subTriangles[j - 1]);

					subTriangles[j - 1] = subTriangle;

					if(*(subTriangle->listSize) == 1422){
						subTriangle = subTriangle;
					}

				}

				if(subTriangles[j -1]){

					int len = *(subTriangles[j - 1]->listSize);
					
					// Put the new triangles into the subspace/object/triangle map
					MultiKey<OctTree*, CollideObject*> key;
					key.keyA = subTree;
					key.keyB = object;
					
					meshMap.Put(key, subTriangles[j - 1]);
					//DLinkedList_DeleteTail<int>(subTriangles[j - 1]);
				}
				else{
					// TODO:  Make deletion of subtrees go, but have to do it so that the numbers
					// work out.
					
					// May as well delete the subtree, no triangles in it.
					subspaces = (subspaces & (~i));
					delete subTree; subTree = NULL;
					subTrees[j - 1] = NULL;
					n--;
				}
				
				n++;
			}
			j--;
		}

/*
								int q = 0;
					for(DLinkedList<int>* r = triangles;
						r != NULL;
						r = r->nextList){
						q++;
						if(r != triangles && r->listSize != r->prevList->listSize){
							r = r;
						}
					}
					q = q;
*/

		// Clean up the border triangles
		DLinkedList_DeleteTail<int>(triangles); triangles = NULL;

		// Clean up the subTriangles array
		delete [] subTriangles; subTriangles = NULL;

		// Clean up if all subspaces were deleted b/c triangles aren't in them
		if(n <= 0){
			delete [] subTrees;
			subTrees = NULL;
			return returnSpaces;
		}
	}

	// Loop to actually update the subspaces
	n = 0; // Holds # of subtrees found/created in this call
	j = 8; // Holds current # of subtree in loop, (see subtree diagram below? above?)
	for(i = 0x01; i < 0x90; i = i << 1){ 
		if((subspaces & i) != 0){
			OctTree* subTree = subTrees[j - 1];
					
			returnSpaces = DLinkedList_AddHeadEntry<OctTree*>(returnSpaces, subTree, 1);

			n++;
		}
		j--;
	}

	return returnSpaces;
}


//---------------------------------------------------------------------------------
DLinkedList<int>* OctTree::borderTrianglesIntersecting(OctTree* space, CollideObject* object, DLinkedList<int>** triangles)
//---------------------------------------------------------------------------------
{
	DLinkedList<int>* returnList = NULL;

	if(triangles == NULL || *(triangles) == NULL || object->model == NULL) return false;

	DLinkedList<int>* triangleIndices = *(triangles);

	Model::Mesh mesh = *(object->model->meshes);

	float extent = space->extent;
	Vector3f pos = space->actualPos;

	Vector3f extents(extent, extent, extent);
	Vector3f spaceMax = extents + pos;
	Vector3f spaceMin = pos - extents;

	for(DLinkedList<int>* currTriangleIndexList = triangleIndices;
		currTriangleIndexList != NULL;
		currTriangleIndexList = currTriangleIndexList->nextList)
	{

		Triangle t = mesh.tris[currTriangleIndexList->object];
		Vector3f a = (mesh.verts[0][t.verts[0]]);
		Vector3f b = (mesh.verts[0][t.verts[1]]);
		Vector3f c = (mesh.verts[0][t.verts[2]]);
	
		// See if the triangles points lie outside the box in any dimension

		if(
			(!(a.x > spaceMax.x && b.x > spaceMax.x && c.x > spaceMax.x)) &&
		    (!(a.x < spaceMin.x && b.x < spaceMin.x && c.x < spaceMin.x)) &&
			(!(a.y > spaceMax.y && b.y > spaceMax.y && c.y > spaceMax.y)) &&
		    (!(a.y < spaceMin.y && b.y < spaceMin.y && c.y < spaceMin.y)) &&
			(!(a.z > spaceMax.z && b.z > spaceMax.z && c.z > spaceMax.z)) &&
		    (!(a.z < spaceMin.z && b.z < spaceMin.z && c.z < spaceMin.z)) )
		{
			returnList = DLinkedList_AddHeadEntry<int>(returnList, currTriangleIndexList->object, 2);

			if(returnList->object == 0){
				space = space;
			}
		}
	}

	return returnList;

}


//---------------------------------------------------------------------------------
DLinkedList<int>* OctTree::trianglesInside(OctTree* space, CollideObject* object, DLinkedList<int>** triangles)
//---------------------------------------------------------------------------------
{
	DLinkedList<int>* returnList = NULL;
	
	if(triangles == NULL || *(triangles) == NULL || object->model == NULL) return false;

	DLinkedList<int>* triangleIndices = *(triangles);

	
	Model::Mesh mesh = *(object->model->meshes);

	int n = 0;
	for(DLinkedList<int>* currTriangleIndexList = triangleIndices;
		currTriangleIndexList != NULL;)
	{

		Triangle t = mesh.tris[currTriangleIndexList->object];
		Vector3f a = (mesh.verts[0][t.verts[0]]);
		Vector3f b = (mesh.verts[0][t.verts[1]]);
		Vector3f c = (mesh.verts[0][t.verts[2]]);

		if(space->contains(&a) && space->contains(&b) && space->contains(&c)){

			DLinkedList<int>* currEntry = currTriangleIndexList;
			currTriangleIndexList = currTriangleIndexList->nextList;
			triangleIndices = DLinkedList_Remove<int>(triangleIndices, currEntry);
			returnList = DLinkedList_AddHead<int>(returnList, currEntry);

			n++;
			continue;
			
		}
		
		currTriangleIndexList = currTriangleIndexList->nextList;
		n++;
	}

	*(triangles) = triangleIndices;

	return returnList;

}


//---------------------------------------------------------------------------------
unsigned int OctTree::partitionBoundingBox(CollideObject* object)
//---------------------------------------------------------------------------------
{
	BoundBox boundBox = object->boundBox;
	// Determine which of the eight sub-zones the bounding box fits into
	unsigned int zones = 0;

	//              Z
	//              /\ 
	//				||
	//
	//         /----- /-----\
	//       /  3   /  2  / |s
	//     /----- /-----\   |
	//   /      /     / |   /   
	// /-----\/-----\   | / |   => X
	// |  4  ||  1  |  /    | 
	// |     ||     | / | 6 |
	// \-----/\-----/   |   /
	// /-----\/-----\   | /
	// |  8  ||  5  |   /
	// |     ||     | /
	// \-----/\-----/
	//
	// unsigned int bit order:  0000000012345678

	if(boundBox.pos.z + boundBox.extents.z > actualPos.z)
		zones = zones | 0xF0;
	if(boundBox.pos.z - boundBox.extents.z < actualPos.z)
		zones = zones | 0x0F;
	if(boundBox.pos.x + boundBox.extents.x < actualPos.x)
		zones = zones & 0x33;
	if(boundBox.pos.x - boundBox.extents.x > actualPos.x)
		zones = zones & 0xCC;
	if(boundBox.pos.y + boundBox.extents.y < actualPos.y)
		zones = zones & 0x66;
	if(boundBox.pos.y - boundBox.extents.y > actualPos.y)
		zones = zones & 0x99;

	return zones;
	
}

//---------------------------------------------------------------------------------
inline int OctTree::countSubspaces(unsigned int subspaces)
//---------------------------------------------------------------------------------
{
	int n = 0;
	int i;
	for(i = 0x01; i < 0x90; i = i << 1){ 
		if((subspaces & i) != 0) n++;
	}

	return n;
}


//---------------------------------------------------------------------------------
bool OctTree::actualShapeCollision()
//---------------------------------------------------------------------------------
{
	return true;
}

//---------------------------------------------------------------------------------
bool OctTree::approximateShapeCollision()
//---------------------------------------------------------------------------------
{
	return true;
}

//---------------------------------------------------------------------------------
inline float OctTree::absf(float x)
//---------------------------------------------------------------------------------
{
	if(x < 0) return -x;
	else return x;
}


//---------------------------------------------------------------------------------
inline bool OctTree::contains(Vector3f* point)
//---------------------------------------------------------------------------------
{	return (point->x < actualPos.x + extent) &&
		   (point->x > actualPos.x - extent) &&
		   (point->y < actualPos.y + extent) &&
		   (point->y > actualPos.y - extent) &&
		   (point->z < actualPos.z + extent) &&
		   (point->z > actualPos.z - extent);
}

//---------------------------------------------------------------------------------
inline int OctTree::sign(float x)
//---------------------------------------------------------------------------------
{	
	if(x > 0) return 1;
	else if(x < 0) return -1;
	else return 0;
}


//---------------------------------------------------------------------------------
inline DLinkedList<CollideInfo*>* OctTree::boundingBoxMeshCollision(CollideObject *objA, CollideObject *objB, OctTree* space)
//---------------------------------------------------------------------------------
{
	if(objB->model == NULL) return NULL;

	DLinkedList<CollideInfo*>* allReturnInfo = NULL;
		
	BoundBox box = objA->boundBox;
	Matrix4f m = (objB->model->modelMatrix);
	Model::Mesh mesh = (objB->model->meshes[0]);
	
	Vector3f moveVector = box.pos - objA->oldBoundBox.pos;
	Vector3f unitMV = moveVector;  unitMV.normalize();

	int numTriangles = mesh.numTriangles;
	OctTree* container = objA->container;

	Vector3f boxVectors[12];
	Vector3f boxPoints[8];
	Vector3f endVectors[12];
	

	int i;
	int currBoxVector = 0;
	int currBoxPoint = 0;
	int coeffA = -1;
	int coeffB = -1;
	int coeffC = -1;
	Vector3f pointB;

	for(i = 0; i < 8; i++){

		coeffA = -coeffA;
		if(i % 2 == 0) coeffB = -coeffB;
		if(i % 4 == 0) coeffC = -coeffC;

		boxPoints[i].set(box.pos.x + coeffA * box.extents.x,
						box.pos.y + coeffB * box.extents.y,
						box.pos.z + coeffC * box.extents.z);
		
		
	}


	// Arranged so that the endPoints of the vectors go in the order (0-7, 0-3)
	// (lets us use mod arithmetic to calc the endpoints, plus groups in order of
	// directions, so we can eliminate four if close to parallel)
	
	// Top of box
	boxVectors[0] = boxPoints[1] - boxPoints[0];
	boxVectors[1] = boxPoints[3] - boxPoints[1];
	boxVectors[2] = boxPoints[0] - boxPoints[2];
	boxVectors[3] = boxPoints[2] - boxPoints[3];
	
	// Bottom of box
	boxVectors[4] = boxPoints[5] - boxPoints[4];
	boxVectors[5] = boxPoints[7] - boxPoints[5];
	boxVectors[6] = boxPoints[4] - boxPoints[6];
	boxVectors[7] = boxPoints[6] - boxPoints[7];

	// Middle of box
	boxVectors[8] = boxPoints[4] - boxPoints[0];
	boxVectors[9] = boxPoints[5] - boxPoints[1];
	boxVectors[10] = boxPoints[6] - boxPoints[2];
	boxVectors[11] = boxPoints[7] - boxPoints[3];


	mesh = (objB->model->meshes[0]);

	Vector3f normalSum;
	
	MultiKey<OctTree*, CollideObject*> key;
	key.keyA = space;
	key.keyB = objB;
	DLinkedList<int>* triangles = meshMap.Get(key);

	if(triangles != NULL){
		triangles = triangles;
	}

	for(DLinkedList<int>* currTriangleIndexList = triangles;
		currTriangleIndexList != NULL;
		currTriangleIndexList = currTriangleIndexList->nextList)
	{
		
		Triangle t = mesh.tris[currTriangleIndexList->object];
		Vector3f a = (mesh.verts[0][t.verts[0]]);
		Vector3f b = (mesh.verts[0][t.verts[1]]);
		Vector3f c = (mesh.verts[0][t.verts[2]]);

		Vector3f triNormal = (b - a).cross(c - a); 
		if(triNormal.magnitude() != 0) triNormal.normalize();

		if(triNormal.dot(moveVector) >= 0) continue;

		float tol = 0.005f;

		int j;
		bool parallelGroupA = false;
		bool parallelGroupB = false;
		for(j = 0; j < 12; j++){

			//if(j < 8 && parallelGroupA && (j % 4 == 3 || j % 4 == 0))
			//	continue;

			//if(j < 8 && parallelGroupB && (j % 4 == 1 || j % 4 == 2))
			//	continue;

			Vector3f boxVector = boxVectors[j];
			
			// Now can use mod to figure out where the endVector is
			Vector3f endPoint = boxPoints[j % 8];
			Vector3f pointVector =  a - boxPoints[j % 8];
			
			float bVecProjected = triNormal.dot(boxVector);
			if(bVecProjected < tol && bVecProjected > -tol){
				//if(j < 3){
				//	parallelGroupA = true;
				//}
				//else if(j < 7){
				//	parallelGroupB = true;
				//}
				/*else*/ if(j > 7 && j < 11) j = 11;
				continue;
			}

			float ratio = triNormal.dot(pointVector) / bVecProjected;
			float oldRatio = triNormal.dot(a - (boxPoints[j % 8] - moveVector)) / bVecProjected;

			if(ratio <= 0 || ratio > 1){
				continue;				
			}

			// Collision with plane
			Vector3f collisionPoint = boxVector * ratio + boxPoints[j % 8];
			int crossA = sign((collisionPoint-b).cross(a - b).dot(triNormal));
			int crossB = sign((collisionPoint-c).cross(b - c).dot(triNormal));
			int crossC = sign((collisionPoint-a).cross(c - a).dot(triNormal));
			
			bool inTriangle = (crossA == crossB && crossB == crossC);
			
			if(inTriangle){

				
				// Correction for side collisions, probably not needed
				
				float penetration = 0.0f;
				if(triNormal.dot(boxVector) > 0)
					penetration = (boxVector * ratio).dot(triNormal);
				else
					penetration = -((boxVector * (1.0f - ratio)).dot(triNormal));
				if(penetration < 0){
					penetration = penetration;
				}
				
				bool triedSide = false;
				Vector3f unitBoxVector = boxVector; unitBoxVector.normalize();
				if(penetration > absf(unitMV.dot(boxVector))){
				
					// SWITCHES DIRECTION
					Vector3f oldMoveVector = moveVector;
					moveVector = triNormal * moveVector.dot(triNormal) - moveVector;

					Vector3f fromPoint = collisionPoint + moveVector;			

					Vector3f perpDist;

					int s1 = sign((fromPoint - a).cross(collisionPoint - a).dot(triNormal));
					int s2 = sign((moveVector).cross(c - collisionPoint).dot(triNormal));

					Vector3f v = (fromPoint - a).cross(collisionPoint - a);

					bool reverseCross = true;
					if(sign((fromPoint - b).cross(collisionPoint - b).dot(triNormal)) == -1 && 
					   sign((moveVector).cross(a - collisionPoint).dot(triNormal)) == -1 )
					{
						Vector3f unitSide = (a - b); unitSide.normalize();
						Vector3f distToCorner = (a - collisionPoint);
						perpDist = distToCorner - unitSide * distToCorner.dot(unitSide);
					}
					else if(sign((fromPoint - c).cross(collisionPoint - c).dot(triNormal)) == -1 && 
					   sign((moveVector).cross(b - collisionPoint).dot(triNormal)) == -1 )
					{
						Vector3f unitSide = (b - c); unitSide.normalize();
						Vector3f distToCorner = (b - collisionPoint);
						perpDist = distToCorner - unitSide * distToCorner.dot(unitSide);
					}
					else if(sign((fromPoint - a).cross(collisionPoint - a).dot(triNormal)) == -1 && 
						sign((moveVector).cross(c - collisionPoint).dot(triNormal)) == -1 )
					{
						Vector3f unitSide = (c - a); unitSide.normalize();
						Vector3f distToCorner = (c - collisionPoint);
						perpDist = distToCorner - unitSide * distToCorner.dot(unitSide);
					}
					else{
						perpDist = perpDist;
					}

					CollideInfo* returnInfo = new CollideInfo;
					
					
					Vector3f unitPerpDist = perpDist;  unitPerpDist.normalize();
					perpDist = unitPerpDist * unitPerpDist.dot(oldMoveVector * -1);

					if(unitPerpDist.x != 0)
						returnInfo->box.extents.x = absf((perpDist.x / 1.95f) / unitPerpDist.x);
					else
						returnInfo->box.extents.x = 0;
					if(unitPerpDist.y != 0)
						returnInfo->box.extents.y = absf((perpDist.y / 1.95f) / unitPerpDist.y);
					else
						returnInfo->box.extents.y = 0;
					if(unitPerpDist.z != 0)
						returnInfo->box.extents.z = absf((perpDist.z / 1.95f) / unitPerpDist.z);
					else
						returnInfo->box.extents.z = 0;

					if(returnInfo->box.extents.magnitude() > 3.0f){
						returnInfo = returnInfo;
					}

					if(unitPerpDist.y > 0.1){
						unitPerpDist = unitPerpDist;
					}
						
					   

					returnInfo->box.pos = collisionPoint;
					returnInfo->normal = unitPerpDist;
					
     				allReturnInfo = DLinkedList_AddHeadEntry<CollideInfo*>(allReturnInfo, returnInfo, 3);
					
					moveVector = oldMoveVector;
					break;
				}
				
				CollideInfo* returnInfo = new CollideInfo;
				Vector3f projBVector = triNormal * bVecProjected;
				if(bVecProjected < 0) ratio = ratio - 1.0f;

				if(ratio > 0.3f && ratio < 0.7f){
					ratio = ratio;
				}

				if(triNormal.x != 0)
					returnInfo->box.extents.x = ((projBVector.x * ratio) / 1.95f) / triNormal.x;
				else
					returnInfo->box.extents.x = 0;
				if(triNormal.y != 0)
					returnInfo->box.extents.y = ((projBVector.y * ratio) / 1.95f) / triNormal.y;
				else
					returnInfo->box.extents.y = 0;
				if(triNormal.z != 0)
					returnInfo->box.extents.z = ((projBVector.z * ratio) / 1.95f) / triNormal.z;
				else
					returnInfo->box.extents.z = 0;
				
				if(returnInfo->box.extents.x > 2.0){
					returnInfo = returnInfo;
				}

				returnInfo->box.pos = collisionPoint;
				returnInfo->normal = triNormal;
				
				if(returnInfo->box.extents.y > 9.0){
					returnInfo = returnInfo;
				}

				if(returnInfo->normal.x < 0){
					projBVector = projBVector;
				}	
				
				allReturnInfo = DLinkedList_AddHeadEntry<CollideInfo*>(allReturnInfo, returnInfo, 3);
				break;
			}

			// Collision with triangle
			
			



		}


	}

	return allReturnInfo;

}

//---------------------------------------------------------------------------------
inline CollideInfo* OctTree::boundingBoxCollision(CollideObject *objA, CollideObject *objB)
//---------------------------------------------------------------------------------
{
	BoundBox &boxA = objA->boundBox;
	BoundBox &boxB = objB->boundBox;

	if((boxA.extents.x + boxB.extents.x) < absf(boxA.pos.x - boxB.pos.x) ||
	   (boxA.extents.y + boxB.extents.y) < absf(boxA.pos.y - boxB.pos.y) ||
	   (boxA.extents.z + boxB.extents.z) < absf(boxA.pos.z - boxB.pos.z))
	{
		return NULL;
	}

	//extents.x = temp.x > boxA.extents.x ? -boxA.extents.x : boxA.extents.x;
	//extents.y = temp.y > boxA.extents.y ? -boxA.extents.y : boxA.extents.y;
	//extents.z = temp.z > boxA.extents.z ? -boxA.extents.z : boxA.extents.z;
	
	// TODO:  Add extent change handler

	Vector3f moveVector = boxA.pos - objA->oldBoundBox.pos;
	Vector3f finalNormal;

	float tol = 0.05f;

	if(moveVector.magnitude() > 0){
		
		Vector3f normalVector(1.0f, 0.0f, 0.0f);
		float xComp = normalVector.dot(moveVector);
		bool backX = absf(-1 * xComp + boxA.pos.x -  boxB.pos.x) + tol>
				boxA.extents.x + boxB.extents.x;

		normalVector.set(0.0f, 1.0f, 0.0f);
		float yComp = normalVector.dot(moveVector);
		bool backY = absf(-1 * yComp + boxA.pos.y -  boxB.pos.y) + tol>
				boxA.extents.y + boxB.extents.y;

		normalVector.set(0.0f, 0.0f, 1.0f);
		float zComp = normalVector.dot(moveVector);
		bool backZ = absf(-1 * zComp + boxA.pos.z -  boxB.pos.z) + tol>
			boxA.extents.z + boxB.extents.z;

		if(!backX && !backY && !backZ){
 			finalNormal = (objA->boundBox.pos - objB->boundBox.pos);		
		}
		else{
			if(backX){
				if(xComp < 0) finalNormal.set(1.0f, 0.0f, 0.0f);
				else finalNormal.set(-1.0f, 0.0f, 0.0f);
			}
			if(backY){
				if(yComp < 0) finalNormal.set(finalNormal.x, 1.0f, 0.0f);
				else finalNormal.set(finalNormal.x, -1.0f, 0.0f);
			}
			if(backZ){
				if(zComp < 0) finalNormal.set(finalNormal.x, finalNormal.y, 1.0f);
				else finalNormal.set(finalNormal.x, finalNormal.y, -1.0f);
			}
			int numDirs = 0;
			if(backX) numDirs++;
			if(backY) numDirs++;
			if(backZ) numDirs++;
			if(numDirs > 1){
				return NULL;
			}
		}
	}
	else{
		finalNormal = objA->boundBox.pos - objB->boundBox.pos;
	}

	finalNormal.normalize();

	/*

	Vector3f definingVectorsA[3];
	Vector3f definingVectorsB[3];
	int vectorsFoundA = 0;
	int vectorsFoundB = 0;

	Vector3f offsetVector = objA->boundBox.pos - objB->boundBox.pos;
	
	Vector3f offsetVectorA(-offsetVector.x / boxA.extents.x, 
		                   -offsetVector.y / boxA.extents.y,
					       -offsetVector.z / boxA.extents.z);
	Vector3f offsetVectorB(offsetVector.x / boxB.extents.x, 
		                   offsetVector.y / boxB.extents.y,
					       offsetVector.z / boxB.extents.z);

	float negFactorX = -1;
	float negFactorY = -1;
	float negFactorZ = -1;
	Vector3f cornerVector;
	for(int i = 0; i < 8; i++){
		negFactorX = negFactorX - negFactorX * 2;
		if(i % 2 == 0) negFactorY = negFactorY - negFactorY * 2;
		if(i % 4 == 0) negFactorZ = negFactorZ - negFactorZ * 2;

		cornerVector.set(negFactorX, negFactorY, negFactorZ);
		
		if(vectorsFoundA < 3 && offsetVectorA.dot(cornerVector) >= 0){
			definingVectorsA[vectorsFoundA] = cornerVector;
			vectorsFoundA++;
			if(vectorsFoundB == 3 && vectorsFoundA == 3) break;
		}

		if(vectorsFoundB < 3 && offsetVectorB.dot(cornerVector) >= 0){
			definingVectorsB[vectorsFoundB] = cornerVector;
			vectorsFoundB++;
			if(vectorsFoundB == 3 && vectorsFoundA == 3) break;
		}
	}

	// Find perpendicular vector
	Vector3f parallelA = definingVectorsA[0] - definingVectorsA[1];
	Vector3f parallelB = definingVectorsA[0] - definingVectorsA[2];
	
	Vector3f normalA = parallelA.cross(parallelB);normalA.normalize();
	if(normalA.dot(offsetVectorA) < 0) normalA = normalA * -1;

	parallelA = definingVectorsB[0] - definingVectorsB[1];
	parallelB = definingVectorsB[0] - definingVectorsB[2];

	Vector3f normalB = parallelA.cross(parallelB);normalB.normalize();
	if(normalB.dot(offsetVectorB) < 0) normalB = normalB * -1;
	
	float tol = 0.5f;
	bool normalOnBoxSize = normalB.dot(normalA) > tol;
*/
/*

	Vector3f angle;
	Vector3f normal;
	Vector3f tempn;
	Vector3f temp = objA->boundBox.pos - objB->boundBox.pos;
	float t1=1.0f,t2=0;
	
	tempn.set(1,0,0);
	angle.set(0.0f,0.0f,0.0f);
	angle.x += boxB.extents.x;
	if(tempn.dot(temp) != 0)
	{
		t2 = tempn.dot(angle)/tempn.dot(temp + extents - tempn * extents.dot(tempn));
		t2 = t2;
		if(t2 < 1.0f && t2 > 0.0f && t2 < t1)
		{
			t1 = t2;
			normal = tempn;
		}
	}
	tempn.set(-1,0,0);
	angle.set(0.0f,0.0f,0.0f);
	angle.x -= boxB.extents.x;
	if(tempn.dot(temp) != 0)
	{
		t2 = tempn.dot(angle)/tempn.dot(temp + extents - tempn * extents.dot(tempn));
		if(t2 > 0.0f && t2 < t1)
		{
			t1 = t2;
			normal = tempn;
		}
	}
	tempn.set(0,1,0);
	angle.set(0.0f,0.0f,0.0f);
	angle.y += boxB.extents.y;
	if(tempn.dot(temp) != 0)
	{
		t2 = tempn.dot(angle)/tempn.dot(temp + extents - tempn * extents.dot(tempn));
		if(t2 > 0.0f && t2 < t1)
		{
			t1 = t2;
			normal = tempn;
		}
	}
	tempn.set(0,-1,0);
	angle.set(0.0f,0.0f,0.0f);
	angle.y -= boxB.extents.y;
	if(tempn.dot(temp) != 0)
	{
		t2 = tempn.dot(angle)/tempn.dot(temp + extents - tempn * extents.dot(tempn));
		if(t2 > 0.0f && t2 < t1)
		{
			t1 = t2;
			normal = tempn;
		}
	}
	tempn.set(0,0,1);
	angle.set(0.0f,0.0f,0.0f);
	angle.z += boxB.extents.z;
	if(tempn.dot(temp) != 0)
	{
		t2 = tempn.dot(angle)/tempn.dot(temp + extents - tempn * extents.dot(tempn));
		if(t2 > 0.0f && t2 < t1)
		{
			t1 = t2;
			normal = tempn;
		}
	}
	tempn.set(0,0,-1);
	angle.set(0.0f,0.0f,0.0f);
	angle.z -= boxB.extents.z;
	if(tempn.dot(temp) != 0)
	{
		t2 = tempn.dot(angle)/tempn.dot(temp + extents - tempn * extents.dot(tempn));
		if(t2 > 0.0f && t2 < t1)
		{
			t1 = t2;
			normal = tempn;
		}
	}
		
	
*/	


	CollideInfo *c = new CollideInfo;
	
	
	float aMin = boxA.pos.x - boxA.extents.x;
	float bMin = boxB.pos.x - boxB.extents.x;
	float aMax = boxA.pos.x + boxA.extents.x;
	float bMax = boxB.pos.x + boxB.extents.x;

	float min = (aMin > bMin) ? aMin : bMin;
	float max = (aMax < bMax) ? aMax : bMax;

	c->box.extents.x = (max - min) / (float) 2.0;
	c->box.pos.x = min + c->box.extents.x;
	
	aMin = boxA.pos.y - boxA.extents.y;
	bMin = boxB.pos.y - boxB.extents.y;
	aMax = boxA.pos.y + boxA.extents.y;
	bMax = boxB.pos.y + boxB.extents.y;

	min = (aMin > bMin) ? aMin : bMin;
	max = (aMax < bMax) ? aMax : bMax;

	c->box.extents.y = (max - min) / (float) 2.0;
	c->box.pos.y = min + c->box.extents.y;

	aMin = boxA.pos.z - boxA.extents.z;
	bMin = boxB.pos.z - boxB.extents.z;
	aMax = boxA.pos.z + boxA.extents.z;
	bMax = boxB.pos.z + boxB.extents.z;

	min = (aMin > bMin) ? aMin : bMin;
	max = (aMax < bMax) ? aMax : bMax;

	c->box.extents.z = (max - min) / (float) 2.0;
	c->box.pos.z = min + c->box.extents.z;
/*
	if(!normalOnBoxSize)
		c->normal = normalB;
	else{
		Vector3f extents(c->box.extents.x, c->box.extents.y, c->box.extents.z);
		
		if(absf(extents.dot(normalB)) < absf(extents.dot(normalA)))
			c->normal = normalB;
		else 
			c->normal = normalA * -1;
	}
	*/

	if(finalNormal.magnitude() != 1.0f){
		printf("YEAH");
	}
	c->normal = finalNormal;
/*
	Vector3f up(3.0f, 0.0f, 3.0f);
	if(absf(c->normal.dot(up)) > tol){
		up = up;
	}
	*/

	return c;
}

//---------------------------------------------------------------------------------
bool OctTree::collideObjectRecursiveUp(CollideObject* object, OctTree* space)
//---------------------------------------------------------------------------------
{
	/*
	 * Given a space and an object, this function will climb to the root node, 
	 * and collide with all objects along the way.
	 */
	bool returnValue = false;

	while(space->parentTree != NULL){
		space = space->parentTree;
		returnValue = space->collideObjectLocal(object) | returnValue;
	}

	return returnValue;
	
}

//---------------------------------------------------------------------------------
bool OctTree::collideObjectRecursiveDown(CollideObject* object, OctTree* space)
//---------------------------------------------------------------------------------
{
	/*
	 * Given a space and an object, this function will climb down through all
	 * APPLICABLE subtrees to collide the current object with.
	 *
	 * i.e. NOT ALL SUBTREES ARE CHECKED, JUST THOSE THAT CONTAIN OBJECT
	 */
	bool returnValue = false;

	if(space->subTrees == NULL) return returnValue;

	unsigned int subspaces = space->partitionBoundingBox(object);

	unsigned int i;
	int j = 8; // Holds current # of subtree in loop, (see subtree diagram below? above?)
	for(i = 0x01; i < 0x90; i = i << 1){ 
		if((subspaces & i) != 0){
			OctTree* subTree = (OctTree*) space->subTrees[j - 1];
			if(subTree != NULL){
				returnValue = subTree->collideObjectLocal(object) | returnValue;
				returnValue = collideObjectRecursiveDown(object, subTree) | returnValue;
			}
		}
		j--;
	}

	return returnValue;
}

//---------------------------------------------------------------------------------
bool OctTree::UpdateObject(CollideObject* object)
//---------------------------------------------------------------------------------
{
	return UpdateObject(object, true);
}

//---------------------------------------------------------------------------------
bool OctTree::UpdateObject(CollideObject* object, bool doCollide)
//---------------------------------------------------------------------------------
{
	/*
	 * Method of updating:
	 *
	 * 0. Delete the collideList of the object
	 * 1. Remove an object from all spaces it currently occupies.
	 * 2. Figure out the object's (new) container
	 * 3. Collide with all objects upward in the tree from the (new) container
	 * 4. Add object to (new) container.
	 */

	OctTree* container = (OctTree*) object->container;
	OctTree* eraser = NULL;
	DLinkedList_DeleteTailObjects<CollideSpace*>(object->collideSpaceList);
	object->collideSpaceList = NULL;

	if(!RemoveObject(object)) return false;

	// Find the largest container that still encloses the object
	while(!container->encloses(object->boundBox)){
		if(container->parentTree != NULL)
		{
			eraser = container->parentTree; 
			// Since the container has changed, perhaps we can delete
			// the space?
			safelyRemoveSubspace(container, eraser);
			container = eraser;

		}
		else 
			break;
	}

	bool returnValue = doCollide && collideObjectRecursiveUp(object, container);
	returnValue = AddObject(container, object, doCollide) | returnValue;
	
	return returnValue;
}

//---------------------------------------------------------------------------------
bool OctTree::CheckObjectHere(CollideObject* object)
//---------------------------------------------------------------------------------
{
	/*
	 * Method of checking is NOT similar to method of updating, object is simply
	 * collided down and up the space, no container checks.
	 */

	collideMap.RemoveAll();
	DLinkedList_DeleteTailObjects<CollideSpace*>(object->collideSpaceList);
	object->collideSpaceList = NULL;

	// Collision test the object in this space, up, and down the tree.
	bool returnValue = collideObjectRecursiveUp(object, this);
	returnValue = collideObjectLocal(object) | returnValue;
	returnValue = collideObjectRecursiveDown(object, this) | returnValue;

	return returnValue;
}

//---------------------------------------------------------------------------------
bool OctTree::collideObjectLocal(CollideObject* object)
//---------------------------------------------------------------------------------
{
	bool returnValue = false;

	for(DLinkedList<CollideObject*>* collideList = objectList;
		collideList != NULL;
		collideList = collideList->nextList)
	{
		CollideObject* listObject = collideList->object;

		// Object shouldn't collide with itself
		// NOTE:  This allows quick checks without removing an 
		// object.  
		if(listObject == object) continue;

		bool hasCollided = false;
		MultiKey<CollideObject*, CollideObject*> key;
		key.keyA = object;
		key.keyB = listObject;
		
		if(excludeMap.Get(key))
			continue;

		if(listObject->collisionTypes & ENTITY_COLLISION_TYPE_BOUNDING_BOX){
			bool hasCollided = collideMap.Get(key);
		}

		//if(!hasCollided) printf("Colliding %d and %d\n", object, listObject);

		if(!hasCollided){

			bool realCollision = false;
			if(listObject->collisionTypes & ENTITY_COLLISION_TYPE_BOUNDING_BOX)
			{
				collideMap.Put(key, true);

				CollideInfo* collision = boundingBoxCollision(object, listObject);
				
				if(collision != NULL){

					//*********************************
					// Code to handle collisions of bounding boxes
					//*********************************

					// Add collision entries to the CollideObject
					CollideSpace* collideTree = new CollideSpace(this, object, listObject, *collision);
					delete collision; collision = NULL;
				
					object->collideSpaceList =	DLinkedList_AddHeadEntry<CollideSpace*>(object->collideSpaceList, collideTree, 4);
				}

				realCollision = (collision != NULL);
				
			}
			else if(listObject->collisionTypes & ENTITY_COLLISION_TYPE_MESH)
			{
				DLinkedList<CollideInfo*>* collision =
					boundingBoxMeshCollision(object,listObject,this);

				
				for(DLinkedList<CollideInfo*>* currCollision = collision;
				    currCollision != NULL;
					currCollision = currCollision->nextList)
				{
					//*********************************
					// Code to handle collisions of bounding boxes & meshes
					//*********************************
					
					CollideInfo* collideInfo = currCollision->object;
					CollideSpace* collideTree = new CollideSpace(this, object, listObject, *collideInfo);

					object->collideSpaceList = DLinkedList_AddHeadEntry<CollideSpace*>(object->collideSpaceList, collideTree, 5);

				}
				realCollision = (collision != NULL);
				DLinkedList_DeleteTailObjects<CollideInfo*>(collision); collision = NULL;
			}

			returnValue = returnValue | realCollision;
		}
	}
					
	return returnValue;
}

//---------------------------------------------------------------------------------
inline bool OctTree::isEnclosedBy(BoundBox& boundBox)
//---------------------------------------------------------------------------------
{
	return (boundBox.pos.x + boundBox.extents.x >= actualPos.x + extent) &&
		   (boundBox.pos.x - boundBox.extents.x <= actualPos.x - extent) &&
		   (boundBox.pos.y + boundBox.extents.y >= actualPos.y + extent) &&
	       (boundBox.pos.y - boundBox.extents.y <= actualPos.y - extent) &&
	       (boundBox.pos.z + boundBox.extents.z >= actualPos.z + extent) &&
	       (boundBox.pos.z - boundBox.extents.z <= actualPos.z - extent);	
}

//---------------------------------------------------------------------------------
inline bool OctTree::encloses(BoundBox& boundBox)
//---------------------------------------------------------------------------------
{
	return (boundBox.pos.x + boundBox.extents.x < actualPos.x + extent) &&
		   (boundBox.pos.x - boundBox.extents.x > actualPos.x - extent) &&
		   (boundBox.pos.y + boundBox.extents.y < actualPos.y + extent) &&
		   (boundBox.pos.y - boundBox.extents.y > actualPos.y - extent) &&
		   (boundBox.pos.z + boundBox.extents.z < actualPos.z + extent) &&
		   (boundBox.pos.z - boundBox.extents.z > actualPos.z - extent);
}

//---------------------------------------------------------------------------------
bool OctTree::resolveCollideList(CollideObject* object, bool doCollide)
//---------------------------------------------------------------------------------
{
	/*
	 * This function is the "guts" of the partitioning scheme, all the logic is here.
	 *
	 */

/*

	bool returnValue = false;

	bool hasSubspaces = (subTrees != NULL);
	bool largeBoxes = (extent > OCTTREE_BOX_COLLISION_GRANULARITY);
	bool tooManyLocalCollisions = (GetNumObjects(this) + 1 > OCTTREE_MAX_NODE_BOX_COLLISIONS);

	// Heavy objects drop to the bottom of the tree right away
	bool heavyObject = 
		((object->collisionTypes & ENTITY_COLLISION_TYPE_MESH) != 0);

	if(heavyObject){

		heavyObject=heavyObject;
	}
	
	if(encloses(object->boundBox)) object->container = this;

	unsigned int objectSpaces = partitionBoundingBox(object);
	int numSpaces = countSubspaces(objectSpaces);
	bool canPartitionObject = 
		(object->remainingSpaces - numSpaces + 1 >= 0);

	bool partitionObject = 
		(heavyObject && 
		 // always a reason to partition heavy objects
		 // ...and if object is allowed to be partitioned
		 canPartitionObject &&
		 //...and if the boxes aren't getting too small.
		 largeBoxes
		)
	  || 
		(
		  !heavyObject &&
		  // ,,,if there is a reason to partition
		  (tooManyLocalCollisions || hasSubspaces) &&
		  // ...and if object is allowed to be partitioned
		  canPartitionObject &&
		  // ...and if partitioning will help
		  !isEnclosedBy(object->boundBox) && 
		  //...and if the boxes aren't getting too small.
		  largeBoxes
	    );

	//if(tooManyLocalCollisions) printf("Too many local collisions.\n");

	//if(partitionObject) printf("Partitioning added object into %d subspaces...\n", numSpaces);
	//else printf("Not partitioning object into %d subspaces...\n", numSpaces);

	bool partitionList = 
		// ...if there is a reason to partition the list
		(tooManyLocalCollisions || hasSubspaces) &&
		//...and if the boxes aren't getting too small.
		largeBoxes;

	// Partition the list
	if(partitionList){
		
		DLinkedList<CollideObject*>* currObjectEntry = objectList;

		while(currObjectEntry != NULL)
		{
			//printf("Curr Object: %d\n", currObjectEntry->object);
			CollideObject* currObject = currObjectEntry->object;

			// Advance before deleting the list entry
			currObjectEntry = currObjectEntry->nextList;
			
			// Don't bother even trying to partition if the object
			// encloses this space
			if(isEnclosedBy(object->boundBox)) continue;
			
			unsigned int objectSpaces = partitionBoundingBox(currObject);
			int numSpaces = countSubspaces(objectSpaces);
			
			// Can't partition if the object can't be subdivided any further.
			// This is useful for moving objects, b/c subdividing them is rather
			// pointless and time consuming, as they are removed from the tree each
			// frame.
			bool canPartitionObject = 
				(currObject->remainingSpaces - numSpaces + 1 >= 0);

			// Partition the object, if possible
			if(canPartitionObject){
				// THIS WILL HURT IF YOU CHANGE IT
				currObject->remainingSpaces = 
					currObject->remainingSpaces - numSpaces + 1;
				removeFromSpace(currObject, this, false);
				addToSubspace(currObject, false, objectSpaces);
			}
		}
	}
	// If the object is not partitioned, then recursively collide it with all the local
	// and lower objects in tree
	if(!partitionObject){

		if(doCollide){
			returnValue = collideObjectLocal(object) | returnValue;
			returnValue = collideObjectRecursiveDown(object, this) | returnValue;
		}

		//*************************************
		// New collision object added here
		//*************************************
		addToSpace(object, this);

	}
	// Otherwise collide locally (unless specified otherwise) and partition the object
	else{

		// THIS WILL HURT IF YOU CHANGE IT
		object->remainingSpaces = 
			object->remainingSpaces - numSpaces + 1;
		if(doCollide) returnValue = collideObjectLocal(object) | returnValue;
		returnValue = addToSubspace(object, true, objectSpaces) | returnValue;
	}
	
	return returnValue;
*/
	return false;


}

//---------------------------------------------------------------------------------

//---------------------------------------------------------------------------------


//---------------------------------------------------------------------------------
CollideSpace::CollideSpace(OctTree* parent,
								   CollideObject* objectA,
								   CollideObject* objectB,
								   CollideInfo& c)
//---------------------------------------------------------------------------------
{
	init(parent, objectA, objectB, c);
}

//---------------------------------------------------------------------------------
CollideSpace::~CollideSpace()
//---------------------------------------------------------------------------------
{
}


//---------------------------------------------------------------------------------
void CollideSpace::init(OctTree* parent, 
						CollideObject* objectA, 
						CollideObject* objectB, 
						CollideInfo& c)
//---------------------------------------------------------------------------------
{
	this->objectA = objectA;
	this->objectB = objectB;
	this->info = c;
}

//---------------------------------------------------------------------------------
void CollideObject::init()
//---------------------------------------------------------------------------------
{
	BoundBox point;
	init(point, ENTITY_COLLISION_TYPE_BOUNDING_BOX);
}

//---------------------------------------------------------------------------------
void CollideObject::init(BoundBox& boundBox, int collisionTypes)
//---------------------------------------------------------------------------------
{
	this->boundBox = boundBox;
	this->collisionTypes = collisionTypes;
	this->spaceList = NULL;
	this->collideSpaceList = NULL;
	this->remainingSpaces = OCTTREE_BOX_DIVISION_LIMIT;
	this->container = NULL;
	this->physicsEntity = NULL;
	this->model = NULL;
}

//---------------------------------------------------------------------------------
CollideObject & CollideObject::operator=(CollideObject c)
//---------------------------------------------------------------------------------
{
	

	this->boundBox = c.boundBox;
	this->model = c.model;
	this->collisionTypes = c.collisionTypes;
	this->spaceList = c.spaceList;
	this->collideSpaceList = c.collideSpaceList;
	
	// These fields are specific to the CollideObject instance, so duplicating
	// them will cause OctTree issues.
	this->remainingSpaces = OCTTREE_BOX_DIVISION_LIMIT;
	this->container = NULL;
	
	return *this;
}



//---------------------------------------------------------------------------------
void CollideObject::cleanUp()
//---------------------------------------------------------------------------------
{
	DLinkedList_DeleteTailObjects<CollideSpace*>(collideSpaceList);
	collideSpaceList = NULL;
	DLinkedList_DeleteTail<OctTree*>(spaceList);
	spaceList = NULL;
}


void OctTree::draw(CGparameter color)
{

/*
	int numKeys = meshMap.GetNumEntries();
	MultiKey<OctTree*, CollideObject*>* keys = meshMap.GetKeys();

	int r = (int) (rand() * 127 + 128);
	int g = (int) (rand() * 127 + 128);
	int bl = (int) (rand() * 127 + 128);

	int i = 0;
	for( i = 0; i < numKeys; i++){
		if(keys[i].keyA == this){
			DLinkedList<int>* triangles = meshMap.Get(keys[i]);
			Model::Mesh mesh = *(keys[i].keyB->mesh);

			int n = 0;
			for(DLinkedList<int>* currTriangleIndexList = triangles;
				currTriangleIndexList != NULL;
				currTriangleIndexList = currTriangleIndexList->nextList)
			{
				
				Triangle t = mesh.tris[currTriangleIndexList->object];
				Vector3f a = (mesh.verts[0][t.verts[0]]);
				Vector3f b = (mesh.verts[0][t.verts[1]]);
				Vector3f c = (mesh.verts[0][t.verts[2]]);

				glBegin(GL_LINES);
					glColor3ub(r,g,bl);
					glVertex3f(a.x, a.y, a.z);
					glVertex3f(b.x, b.y, b.z);
					glVertex3f(c.x, c.y, c.z);
					glVertex3f(a.x, a.y, a.z);
					glVertex3f(c.x, c.y, c.z);
					glVertex3f(b.x, b.y, b.z);
				glEnd();
				

				if(a.y < this->actualPos.y - this->extent &&
				   b.y < this->actualPos.y - this->extent && 
				   c.y < this->actualPos.y - this->extent){
					a = a;
				}


				glBegin(GL_TRIANGLES);
					glColor3ub(r,g,bl);
					glVertex3f(a.x, a.y, a.z);
					glVertex3f(b.x, b.y, b.z);
					glVertex3f(c.x, c.y, c.z);
				glEnd();


				n++;
			}
			n = n;
		}
	}
*/
	if(this->objectList){
	glColor3f(0.0f,1.0f,0.0f);
	//cgGLSetParameter4f(color,0.0f,1.0f,0.0f,1.0f);
			glBegin(GL_LINES);
				//glColor3ub(r, g, bl);
				glVertex3f(actualPos.x - extent, actualPos.y - extent, actualPos.z - extent);
				glVertex3f(actualPos.x - extent, actualPos.y - extent, actualPos.z + extent);
				glVertex3f(actualPos.x - extent, actualPos.y - extent, actualPos.z - extent);
				glVertex3f(actualPos.x - extent, actualPos.y + extent, actualPos.z - extent);
				glVertex3f(actualPos.x - extent, actualPos.y - extent, actualPos.z - extent);
				glVertex3f(actualPos.x + extent, actualPos.y - extent, actualPos.z - extent);

				glVertex3f(actualPos.x - extent, actualPos.y + extent, actualPos.z + extent);
				glVertex3f(actualPos.x - extent, actualPos.y + extent, actualPos.z - extent);
				glVertex3f(actualPos.x - extent, actualPos.y + extent, actualPos.z + extent);
				glVertex3f(actualPos.x - extent, actualPos.y - extent, actualPos.z + extent);
				glVertex3f(actualPos.x - extent, actualPos.y + extent, actualPos.z + extent);
				glVertex3f(actualPos.x + extent, actualPos.y + extent, actualPos.z + extent);

				glVertex3f(actualPos.x + extent, actualPos.y - extent, actualPos.z + extent);
				glVertex3f(actualPos.x + extent, actualPos.y - extent, actualPos.z - extent);
				glVertex3f(actualPos.x + extent, actualPos.y - extent, actualPos.z + extent);
				glVertex3f(actualPos.x + extent, actualPos.y + extent, actualPos.z + extent);
				glVertex3f(actualPos.x + extent, actualPos.y - extent, actualPos.z + extent);
				glVertex3f(actualPos.x - extent, actualPos.y - extent, actualPos.z + extent);

				glVertex3f(actualPos.x + extent, actualPos.y + extent, actualPos.z - extent);
				glVertex3f(actualPos.x + extent, actualPos.y - extent, actualPos.z - extent);
				glVertex3f(actualPos.x + extent, actualPos.y + extent, actualPos.z - extent);
				glVertex3f(actualPos.x + extent, actualPos.y + extent, actualPos.z + extent);
				glVertex3f(actualPos.x + extent, actualPos.y + extent, actualPos.z - extent);
				glVertex3f(actualPos.x - extent, actualPos.y + extent, actualPos.z - extent);
			glEnd();
	}

	if(this->subTrees){

		for(int i = 0; i < 8; i++){
			if(subTrees[i]) subTrees[i]->draw(color);
		}
	}
}




