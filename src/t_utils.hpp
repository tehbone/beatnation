
/************************************
*	Versioning Information			*
************************************/
// 4/12/2004
// Started versioning - GS

#ifndef T_UTILS_H
#define T_UTILS_H

/*
 * DATA STRUCTURE TEMPLATES
 */ 


// Doubly linked list
template <class TYPE>
struct DLinkedList
{
	TYPE object;
	int* listSize;

	int indicator;
	
	DLinkedList<TYPE>* nextList;
	DLinkedList<TYPE>* prevList;
};


template <class POINTER>
void DLinkedList_DeleteTailObjects(DLinkedList<POINTER>* list);

template <class TYPE>
void DLinkedList_DeleteTail(DLinkedList<TYPE>* list);

template <class TYPE>
DLinkedList<TYPE>* DLinkedList_DeleteEntry(DLinkedList<TYPE>* list, TYPE object);

template <class TYPE>
DLinkedList<TYPE>* DLinkedList_AddHead(DLinkedList<TYPE>* list,
									   DLinkedList<TYPE>* listEntry);

template <class TYPE>
DLinkedList<TYPE>* DLinkedList_AddAfter(DLinkedList<TYPE>* baseList,
										DLinkedList<TYPE>* list,
										DLinkedList<TYPE>* listEntry);

template <class TYPE>
DLinkedList<TYPE>* DLinkedList_AddBefore(DLinkedList<TYPE>* baseList,
										 DLinkedList<TYPE>* list,
										 DLinkedList<TYPE>* listEntry);

template <class TYPE>
DLinkedList<TYPE>* DLinkedList_AddHeadEntry(DLinkedList<TYPE>* list, TYPE object);

template <class TYPE>
DLinkedList<TYPE>* DLinkedList_AddAfterEntry(DLinkedList<TYPE>* list, DLinkedList<TYPE>* listEntry, TYPE object);

template <class TYPE>
DLinkedList<TYPE>* DLinkedList_AddBeforeEntry(DLinkedList<TYPE>* list, DLinkedList<TYPE>* listEntry, TYPE object);

template <class TYPE>
DLinkedList<TYPE>* DLinkedList_Remove(DLinkedList<TYPE>* list, DLinkedList<TYPE>* listEntry);

template <class TYPE>
DLinkedList<TYPE>* DLinkedList_GetTail(DLinkedList<TYPE>* list);

template <class KEY, class VALUE>
struct HashBucket
{
	unsigned int hashKey;
	KEY key;
	VALUE value;

	HashBucket<KEY, VALUE>* nextBucket;	
};



// HashMap data structure

#define HASHMAP_DEFAULT_BUCKETS 5
#define HASHMAP_MAX_HASH_RATIO 0.75 

template <class KEY, class VALUE>
class HashMap
{
public:	
	HashMap();
	HashMap(int initialSize);
	HashMap(VALUE nullValue);
	HashMap(int initialSize, VALUE nullValue);
	~HashMap();

	KEY* GetKeys();

	int GetNumEntries();
	int GetNumBuckets();
	void SetNumBuckets(int numBuckets);

	void Put(KEY key, VALUE value);
	VALUE Get(KEY key);
	VALUE Remove(KEY key);

	VALUE nullValue;

	void printDistribution();

protected:

	void init(int initialSize, VALUE nullValue);

	unsigned int hash(KEY key);
	void rehash(int newBuckets);

	int numEntries;
	int numBuckets;
	
	HashBucket<KEY, VALUE>** buckets;
};


// CollideMap Data Structures

template <class KEYA, class KEYB>
struct MultiKey
{
	KEYA keyA;
	KEYB keyB;
};

template <class KEYA, class KEYB, class VALUE>
class CollideMap
{
public:	
	CollideMap();
	CollideMap(int initialSize);
	CollideMap(VALUE nullValue);
	CollideMap(int initialSize, VALUE nullValue);
	~CollideMap();

	MultiKey<KEYA, KEYB>* GetKeys();
	void RemoveAll();

	int GetNumEntries();
	int GetNumBuckets();
	void SetNumBuckets(int numBuckets);

	void Put(MultiKey<KEYA, KEYB> key, VALUE value);
	VALUE Get(MultiKey<KEYA, KEYB> key);
	VALUE Remove(MultiKey<KEYA, KEYB> key);

	VALUE nullValue;

	void printDistribution();

protected:

	void init(int initialSize, VALUE nullValue);

	unsigned int hash(MultiKey<KEYA, KEYB> key);
	void rehash(int newBuckets);

	int numEntries;
	int numBuckets;
	
	HashBucket<MultiKey<KEYA, KEYB>, VALUE>** buckets;

};

#endif