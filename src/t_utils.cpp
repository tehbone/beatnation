#ifndef T_UTILS_CPP
#define T_UTILS_CPP

#include <cstdlib>
#include "t_utils.hpp"

// Linked list methods

//---------------------------------------------------------------------------------
template <class POINTER>
void DLinkedList_DeleteTailObjects(DLinkedList<POINTER>* list)
//---------------------------------------------------------------------------------
{
	DLinkedList<POINTER>* nextList = NULL;
	while(list != NULL){
		nextList = list->nextList;
		if(nextList == NULL)
			delete list->listSize;
		delete list->object;
		delete list;
		list = nextList;		
	}
}


//---------------------------------------------------------------------------------
template <class TYPE>
void DLinkedList_DeleteTail(DLinkedList<TYPE>* list)
//---------------------------------------------------------------------------------
{
	DLinkedList<TYPE>* nextList = NULL;
	while(list != NULL){
		nextList = list->nextList;
		if(nextList == NULL)
			delete list->listSize;
		delete list;
		list = nextList;		
	}
}



//---------------------------------------------------------------------------------
template <class TYPE>
DLinkedList<TYPE>* DLinkedList_DeleteEntry(DLinkedList<TYPE>* list, TYPE object)
//---------------------------------------------------------------------------------
{
	DLinkedList<TYPE>* startList = list;

	for(;list != NULL; list = list->nextList)
	{
		if(list->object == object){
			// Change size
			int listSize = *(list->listSize);
			if(listSize == 1) delete list->listSize;
			else *(list->listSize) = listSize - 1;

			// Reconnect
			if(list->prevList != NULL)
				list->prevList->nextList = list->nextList;
			else
				startList = list->nextList;

			if(list->nextList != NULL)
				list->nextList->prevList = list->prevList;
			
			delete list; list = NULL;
			
			return startList;
		}
	}

	return startList;
}


//---------------------------------------------------------------------------------
template <class TYPE>
DLinkedList<TYPE>* DLinkedList_AddHead(DLinkedList<TYPE>* list,
									   DLinkedList<TYPE>* listEntry)
//---------------------------------------------------------------------------------
{
	listEntry->indicator += 4200;
	return DLinkedList_AddBefore<TYPE>(list, NULL, listEntry);
}

//---------------------------------------------------------------------------------
template <class TYPE>
DLinkedList<TYPE>* DLinkedList_AddAfter(DLinkedList<TYPE>* baseList,
										DLinkedList<TYPE>* list,
										DLinkedList<TYPE>* listEntry)
//---------------------------------------------------------------------------------
{
	if(list == NULL) list = baseList;
	if(listEntry == NULL) return baseList;

	listEntry->prevList = list;
	
	if(list != NULL){
		
		DLinkedList<TYPE>* tail = DLinkedList_GetTail<TYPE>(listEntry);
		
		*(list->listSize) = *(list->listSize) + *(listEntry->listSize);
		delete listEntry->listSize;

		for(DLinkedList<TYPE>* newEntries = listEntry;
		    newEntries != NULL;
			newEntries = newEntries->nextList)
		{
			newEntries->listSize = list->listSize;	
		}

		tail->nextList = list->nextList;
		
		
		if(list->nextList != NULL){
			list->nextList->prevList = listEntry;
		}
		list->nextList = listEntry;

	}
	else{

		baseList = listEntry;
	}

	return baseList;
}


//---------------------------------------------------------------------------------
template <class TYPE>
DLinkedList<TYPE>* DLinkedList_AddBefore(DLinkedList<TYPE>* baseList,
										 DLinkedList<TYPE>* list,
										 DLinkedList<TYPE>* listEntry)
//---------------------------------------------------------------------------------
{
	if(list == NULL) list = baseList;
	
	if(list != NULL){
		
		DLinkedList<TYPE>* tail = DLinkedList_GetTail<TYPE>(listEntry);

		tail->nextList = list;

		*(list->listSize) = *(list->listSize) + *(listEntry->listSize);
		delete listEntry->listSize;

		for(DLinkedList<TYPE>* newEntries = listEntry;
		    newEntries != NULL;
			newEntries = newEntries->nextList)
		{
			newEntries->listSize = list->listSize;	
		}
		
		listEntry->prevList = list->prevList;
		
		if(list->prevList != NULL){
			list->prevList->nextList = listEntry;
		}
		else baseList = listEntry;
		list->prevList = tail;
	}
	else{

		baseList = listEntry;
	}

	return baseList;
}


//---------------------------------------------------------------------------------
template <class TYPE>
DLinkedList<TYPE>* DLinkedList_AddHeadEntry(DLinkedList<TYPE>* list, TYPE object)
//---------------------------------------------------------------------------------
{
	return DLinkedList_AddHeadEntry<TYPE>(list, object, 0);
}

//---------------------------------------------------------------------------------
template <class TYPE>
DLinkedList<TYPE>* DLinkedList_AddHeadEntry(DLinkedList<TYPE>* list, TYPE object, int indicator)
//---------------------------------------------------------------------------------
{
	DLinkedList<TYPE>* newEntry = new DLinkedList<TYPE>;
	newEntry->nextList = NULL;
	newEntry->prevList = NULL;
	newEntry->indicator = indicator;
	newEntry->listSize = new int;
	*(newEntry->listSize) = 1;

	newEntry->object = object;
	return DLinkedList_AddBefore<TYPE>(list, NULL, newEntry);
}


//---------------------------------------------------------------------------------
template <class TYPE>
DLinkedList<TYPE>* DLinkedList_AddBeforeEntry(DLinkedList<TYPE>* list, DLinkedList<TYPE>* listEntry, TYPE object)
//---------------------------------------------------------------------------------
{
	DLinkedList<TYPE>* newEntry = new DLinkedList<TYPE>;
	newEntry->object = object;
	return DLinkedList_AddBefore<TYPE>(list, listEntry, newEntry);
}

//---------------------------------------------------------------------------------
template <class TYPE>
DLinkedList<TYPE>* DLinkedList_AddAfterEntry(DLinkedList<TYPE>* list, DLinkedList<TYPE>* listEntry, TYPE object)
//---------------------------------------------------------------------------------
{
	DLinkedList<TYPE>* newEntry = new DLinkedList<TYPE>;
	newEntry->object = object;
	return DLinkedList_AddAfter<TYPE>(list, listEntry, newEntry);
}
//---------------------------------------------------------------------------------
template <class TYPE>
DLinkedList<TYPE>* DLinkedList_Remove(DLinkedList<TYPE>* list, DLinkedList<TYPE>* listEntry)
//---------------------------------------------------------------------------------
{
	if(listEntry == NULL || list == NULL) return list;
	
	DLinkedList<TYPE>* returnList = list;

	if(listEntry->prevList != NULL)
		listEntry->prevList->nextList = listEntry->nextList;
	else
		returnList = listEntry->nextList;
	if(listEntry->nextList != NULL)
		listEntry->nextList->prevList = listEntry->prevList;

	listEntry->nextList = NULL;
	listEntry->prevList = NULL;

	if(returnList != NULL){
			*(returnList->listSize) = *(returnList->listSize) - 1;
			listEntry->listSize = new int;
			*(listEntry->listSize) = 1;
	}

	return returnList;
}

//---------------------------------------------------------------------------------
template <class TYPE>
DLinkedList<TYPE>* DLinkedList_GetTail(DLinkedList<TYPE>* list)
//---------------------------------------------------------------------------------
{
	if(list == NULL) return NULL;

	for(;list->nextList != NULL; list = list->nextList);
	return list;
}



//---------------------------------------------------------------------------------
template <class KEY, class VALUE>
HashMap<KEY, VALUE>::HashMap()
//---------------------------------------------------------------------------------
{
	init(HASHMAP_DEFAULT_BUCKETS, NULL);	
}

//---------------------------------------------------------------------------------
template <class KEY, class VALUE>
HashMap<KEY, VALUE>::HashMap(VALUE nullValue)
//---------------------------------------------------------------------------------
{
	init(HASHMAP_DEFAULT_BUCKETS, nullValue);
}

//---------------------------------------------------------------------------------
template <class KEY, class VALUE>
HashMap<KEY, VALUE>::HashMap(int numBuckets)
//---------------------------------------------------------------------------------
{
	init(numBuckets, NULL);
}

//---------------------------------------------------------------------------------
template <class KEY, class VALUE>
HashMap<KEY, VALUE>::HashMap(int numBuckets, VALUE nullValue)
//---------------------------------------------------------------------------------
{
	init(numBuckets, nullValue);
}

//---------------------------------------------------------------------------------
template <class KEY, class VALUE>
void HashMap<KEY, VALUE>::init(int numBuckets, VALUE nullValue)
//---------------------------------------------------------------------------------
{
	buckets = (HashBucket<KEY, VALUE>**) std::calloc(numBuckets, sizeof(HashBucket<KEY, VALUE>*));
	this->numBuckets = numBuckets;
	this->numEntries = 0;
	this->nullValue = nullValue;
}

//---------------------------------------------------------------------------------
template <class KEY, class VALUE>
HashMap<KEY,VALUE>::~HashMap()
//---------------------------------------------------------------------------------
{

	int i;
	for(i = 0; i < numBuckets; i++){
		if(buckets[i] != NULL){
			HashBucket<KEY, VALUE>* bucket = buckets[i];
			do{
				HashBucket<KEY, VALUE>* newBucket = bucket->nextBucket;
				delete bucket;
				bucket = newBucket;
			}
			while(bucket != NULL);
		}
	}

	std::free(buckets);
	buckets = NULL;

}

//---------------------------------------------------------------------------------
template <class KEY, class VALUE>
unsigned int HashMap<KEY,VALUE>::hash(KEY key)
//---------------------------------------------------------------------------------
{
	//FIXME: better hashing.
	unsigned int conversion = *reinterpret_cast<unsigned int*>(&key);
	return conversion ^ (conversion >> 1);
}

//---------------------------------------------------------------------------------
template <class KEY, class VALUE>
void HashMap<KEY,VALUE>::Put(KEY key, VALUE value)
//---------------------------------------------------------------------------------
{
	unsigned int hashKey = hash(key);

	int hashBucket = hashKey % numBuckets;
	
	// Check for previous key entries
	HashBucket<KEY,VALUE>* oldBucket = buckets[hashBucket];
	while(oldBucket != NULL){
		if(oldBucket->key == key){
			oldBucket->value = value;
			return;
		}
		oldBucket = oldBucket->nextBucket;
	}

	// Add a new key entry if the others don't exist
	HashBucket<KEY, VALUE>* bucket = new HashBucket<KEY, VALUE>;
	bucket->hashKey = hashKey;
	bucket->key = key;
	bucket->value = value;
	bucket->nextBucket = buckets[hashBucket];
	buckets[hashBucket] = bucket;
	numEntries++;

	if((float) numEntries / (float) numBuckets > HASHMAP_MAX_HASH_RATIO)
		rehash(numBuckets * 2 + 1);
}

//---------------------------------------------------------------------------------
template <class KEY, class VALUE>
VALUE HashMap<KEY,VALUE>::Get(KEY key)
//---------------------------------------------------------------------------------
{
	int hashBucket = hash(key) % numBuckets;
	HashBucket<KEY, VALUE>* bucket = buckets[hashBucket];

	VALUE returnValue = nullValue;

	while(bucket != NULL){
		if(key == bucket->key){
			returnValue = bucket->value;
			break;
		}
		else bucket = bucket->nextBucket;
	}

	return returnValue;
}

//---------------------------------------------------------------------------------
template <class KEY, class VALUE>
VALUE HashMap<KEY,VALUE>::Remove(KEY key)
//---------------------------------------------------------------------------------
{
	int hashBucket = hash(key) % numBuckets;
	HashBucket<KEY, VALUE>* bucket = buckets[hashBucket];
	HashBucket<KEY, VALUE>* lastBucket = NULL;

	VALUE returnValue = nullValue;

	while(bucket != NULL){
		if(key == bucket->key){
			returnValue = bucket->value;
			numEntries--;

			if(lastBucket == NULL){
				buckets[hashBucket] = bucket->nextBucket;
				delete bucket;
			}
			else{
				lastBucket->nextBucket = bucket->nextBucket;
				delete bucket;
			}
			
			break;
		}
		else{
			lastBucket = bucket;
			bucket = bucket->nextBucket;
		}
			
	}

	return returnValue;
}

//---------------------------------------------------------------------------------
template <class KEY, class VALUE>
void HashMap<KEY,VALUE>::rehash(int numNewBuckets)
//---------------------------------------------------------------------------------
{
	int numOldBuckets = this->numBuckets;
	numBuckets = numNewBuckets;
	
	HashBucket<KEY, VALUE>** oldBuckets = new HashBucket<KEY, VALUE>*[numOldBuckets];
	
	int i;
	for(i = 0; i < numOldBuckets; i++)
		oldBuckets[i] = buckets[i];

	buckets = (HashBucket<KEY, VALUE>**) realloc(buckets, numNewBuckets * sizeof(HashBucket<KEY, VALUE>*));

	for(i = 0; i < numNewBuckets; i++) buckets[i] = NULL;
	
	for(i = 0; i < numOldBuckets; i++){
		HashBucket<KEY, VALUE>* bucket = oldBuckets[i];
		while(bucket != NULL){
			int newIndex = bucket->hashKey % numNewBuckets;
			HashBucket<KEY, VALUE>* nextBucket = bucket->nextBucket;
			bucket->nextBucket = buckets[newIndex];
			buckets[newIndex] = bucket;
			bucket = nextBucket;
		}
	}

	delete [] oldBuckets; oldBuckets = NULL;
}

//---------------------------------------------------------------------------------
template <class KEY, class VALUE>
KEY* HashMap<KEY,VALUE>::GetKeys()
//---------------------------------------------------------------------------------
{
	int numKeys = numEntries;

	KEY* keys = new KEY[numKeys];

	int numBuckets = this->numBuckets;

	int i;
	int keyNum = 0;
	for(i = 0; i < numBuckets; i++){
		HashBucket<KEY, VALUE>* bucket = buckets[i];
		while(bucket != NULL){
			keys[keyNum] = bucket->key;
			keyNum++;
			bucket = bucket->nextBucket;
		}
	}

	return keys;
}


//---------------------------------------------------------------------------------
template <class KEY, class VALUE>
int HashMap<KEY,VALUE>::GetNumEntries()
//---------------------------------------------------------------------------------
{
	return numEntries;
}

//---------------------------------------------------------------------------------
template <class KEY, class VALUE>
int HashMap<KEY,VALUE>::GetNumBuckets()
//---------------------------------------------------------------------------------
{
	return numBuckets;
}

//---------------------------------------------------------------------------------
template <class KEY, class VALUE>
void HashMap<KEY,VALUE>::SetNumBuckets(int numBuckets)
//---------------------------------------------------------------------------------
{
	rehash(numBuckets);
}


//---------------------------------------------------------------------------------
template <class KEY, class VALUE>
void HashMap<KEY,VALUE>::printDistribution()
//---------------------------------------------------------------------------------
{

	int i;
	for(i = 0; i < numBuckets; i++){
		int numBuckets = 0;
		if(buckets[i] != NULL){
			HashBucket<KEY, VALUE>* bucket = buckets[i];
			do{
				HashBucket<KEY, VALUE>* newBucket = bucket->nextBucket;
				bucket = newBucket;
				numBuckets++;
			}
			while(bucket != NULL);
		}
		printf("\nBucket %d: %d entries.", i, numBuckets);
	}

}




//---------------------------------------------------------------------------------
template <class KEYA, class KEYB, class VALUE>
CollideMap<KEYA, KEYB, VALUE>::CollideMap()
//---------------------------------------------------------------------------------
{
	init(HASHMAP_DEFAULT_BUCKETS, NULL);	
}

//---------------------------------------------------------------------------------
template <class KEYA, class KEYB, class VALUE>
CollideMap<KEYA, KEYB, VALUE>::CollideMap(VALUE nullValue)
//---------------------------------------------------------------------------------
{
	init(HASHMAP_DEFAULT_BUCKETS, nullValue);
}

//---------------------------------------------------------------------------------
template <class KEYA, class KEYB, class VALUE>
CollideMap<KEYA, KEYB, VALUE>::CollideMap(int numBuckets)
//---------------------------------------------------------------------------------
{
	init(numBuckets, NULL);
}

//---------------------------------------------------------------------------------
template <class KEYA, class KEYB, class VALUE>
CollideMap<KEYA, KEYB, VALUE>::CollideMap(int numBuckets, VALUE nullValue)
//---------------------------------------------------------------------------------
{
	init(numBuckets, nullValue);
}

//---------------------------------------------------------------------------------
template <class KEYA, class KEYB, class VALUE>
void CollideMap<KEYA, KEYB, VALUE>::init(int numBuckets, VALUE nullValue)
//---------------------------------------------------------------------------------
{
	buckets = (HashBucket<MultiKey<KEYA, KEYB>, VALUE>**) std::calloc(numBuckets, sizeof(HashBucket<MultiKey<KEYA, KEYB>, VALUE>*));
	this->numBuckets = numBuckets;
	this->numEntries = 0;
	this->nullValue = nullValue;
}

//---------------------------------------------------------------------------------
template <class KEYA, class KEYB, class VALUE>
CollideMap<KEYA, KEYB, VALUE>::~CollideMap()
//---------------------------------------------------------------------------------
{

	int i;
	for(i = 0; i < numBuckets; i++){
		if(buckets[i] != NULL){
			HashBucket<MultiKey<KEYA,KEYB>, VALUE>* bucket = buckets[i];
			do{
				HashBucket<MultiKey<KEYA,KEYB>, VALUE>* newBucket = bucket->nextBucket;
				delete bucket;
				bucket = newBucket;
			}
			while(bucket != NULL);
		}
	}

	std::free(buckets);
	buckets = NULL;

}

//---------------------------------------------------------------------------------
template <class KEYA, class KEYB, class VALUE>
unsigned int CollideMap<KEYA, KEYB, VALUE>::hash(MultiKey <KEYA, KEYB> key)
//---------------------------------------------------------------------------------
{
	unsigned int conversion = *reinterpret_cast<unsigned int*>(key.keyA) + *reinterpret_cast<unsigned int*>(key.keyB);
	return conversion ^ (conversion >> 1);
}

//---------------------------------------------------------------------------------
template <class KEYA, class KEYB, class VALUE>
void CollideMap<KEYA, KEYB, VALUE>::Put(MultiKey <KEYA, KEYB> key, VALUE value)
//---------------------------------------------------------------------------------
{
	unsigned int hashKey = hash(key);

	int hashBucket = hashKey % numBuckets;
	
	// Check for previous key entries
	HashBucket<MultiKey <KEYA, KEYB>,VALUE>* oldBucket = buckets[hashBucket];
	while(oldBucket != NULL){
		if(oldBucket->key.keyA == key.keyA &&
		   oldBucket->key.keyB == key.keyB)
		{
			oldBucket->value = value;
			return;
		}
		oldBucket = oldBucket->nextBucket;
	}

	// Add a new key entry if the others don't exist
	HashBucket<MultiKey <KEYA, KEYB>, VALUE>* bucket = new HashBucket<MultiKey <KEYA, KEYB>, VALUE>;
	bucket->hashKey = hashKey;
	bucket->key = key;
	bucket->value = value;
	bucket->nextBucket = buckets[hashBucket];
	buckets[hashBucket] = bucket;
	numEntries++;

	if((float) numEntries / (float) numBuckets > HASHMAP_MAX_HASH_RATIO)
		rehash(numBuckets * 2 + 1);
}

//---------------------------------------------------------------------------------
template <class KEYA, class KEYB, class VALUE>
VALUE CollideMap<KEYA, KEYB, VALUE>::Get(MultiKey <KEYA, KEYB> key)
//---------------------------------------------------------------------------------
{
	int hashBucket = hash(key) % numBuckets;
	HashBucket<MultiKey <KEYA, KEYB>, VALUE>* bucket = buckets[hashBucket];

	VALUE returnValue = nullValue;

	while(bucket != NULL){
		if(key.keyA == bucket->key.keyA &&
		   key.keyB == bucket->key.keyB)
		{
			returnValue = bucket->value;
			break;
		}
		else bucket = bucket->nextBucket;
	}

	return returnValue;
}

//---------------------------------------------------------------------------------
template <class KEYA, class KEYB, class VALUE>
VALUE CollideMap<KEYA, KEYB, VALUE>::Remove(MultiKey <KEYA, KEYB> key)
//---------------------------------------------------------------------------------
{
	int hashBucket = hash(key) % numBuckets;
	HashBucket<MultiKey <KEYA, KEYB>, VALUE>* bucket = buckets[hashBucket];
	HashBucket<MultiKey <KEYA, KEYB>, VALUE>* lastBucket = NULL;

	VALUE returnValue = nullValue;

	while(bucket != NULL){
		if(key.keyA == bucket->key.keyA &&
		   key.keyB == bucket->key.keyB)
		{
			returnValue = bucket->value;
			numEntries--;

			if(lastBucket == NULL){
				buckets[hashBucket] = bucket->nextBucket;
				delete bucket;
			}
			else{
				lastBucket->nextBucket = bucket->nextBucket;
				delete bucket;
			}
			
			break;
		}
		else{
			lastBucket = bucket;
			bucket = bucket->nextBucket;
		}
			
	}

	return returnValue;
}

//---------------------------------------------------------------------------------
template <class KEYA, class KEYB, class VALUE>
void CollideMap<KEYA, KEYB, VALUE>::rehash(int numNewBuckets)
//---------------------------------------------------------------------------------
{
	int numOldBuckets = this->numBuckets;
	numBuckets = numNewBuckets;
	
	HashBucket<MultiKey <KEYA, KEYB>, VALUE>** oldBuckets = new HashBucket<MultiKey <KEYA, KEYB>, VALUE>*[numOldBuckets];
	
	int i;
	for(i = 0; i < numOldBuckets; i++)
		oldBuckets[i] = buckets[i];

	buckets =(HashBucket<MultiKey <KEYA, KEYB>, VALUE>**) realloc(buckets, numNewBuckets * sizeof(HashBucket<MultiKey <KEYA, KEYB>, VALUE>*));

	for(i = 0; i < numNewBuckets; i++) buckets[i] = NULL;
	
	for(i = 0; i < numOldBuckets; i++){
		HashBucket<MultiKey <KEYA, KEYB>, VALUE>* bucket = oldBuckets[i];
		while(bucket != NULL){
			int newIndex = bucket->hashKey % numNewBuckets;
			HashBucket<MultiKey <KEYA, KEYB>, VALUE>* nextBucket = bucket->nextBucket;
			bucket->nextBucket = buckets[newIndex];
			buckets[newIndex] = bucket;
			bucket = nextBucket;
		}
	}

	delete [] oldBuckets; oldBuckets = NULL;
}

//---------------------------------------------------------------------------------
template <class KEYA, class KEYB, class VALUE>
MultiKey <KEYA, KEYB>* CollideMap<KEYA, KEYB, VALUE>::GetKeys()
//---------------------------------------------------------------------------------
{
	int numKeys = numEntries;

	MultiKey <KEYA, KEYB>* keys = new MultiKey <KEYA, KEYB>[numKeys];

	int numBuckets = this->numBuckets;

	int i;
	int keyNum = 0;
	for(i = 0; i < numBuckets; i++){
		HashBucket<MultiKey <KEYA, KEYB>, VALUE>* bucket = buckets[i];
		while(bucket != NULL){
			keys[keyNum] = bucket->key;
			keyNum++;
			bucket = bucket->nextBucket;
		}
	}

	return keys;
}

//---------------------------------------------------------------------------------
template <class KEYA, class KEYB, class VALUE>
void CollideMap<KEYA, KEYB, VALUE>::RemoveAll()
//---------------------------------------------------------------------------------
{
	int numBuckets = this->numBuckets;

	int i;
	for(i = 0; i < numBuckets; i++){
		HashBucket<MultiKey <KEYA, KEYB>, VALUE>* bucket = buckets[i];
		HashBucket<MultiKey <KEYA, KEYB>, VALUE>* nextBucket = NULL;
		while(bucket != NULL){
			nextBucket = bucket->nextBucket;
			delete bucket;
			bucket = nextBucket;
		}
		buckets[i] = NULL;
	}

	numEntries = 0;
}


//---------------------------------------------------------------------------------
template <class KEYA, class KEYB, class VALUE>
int CollideMap<KEYA, KEYB, VALUE>::GetNumEntries()
//---------------------------------------------------------------------------------
{
	return numEntries;
}

//---------------------------------------------------------------------------------
template <class KEYA, class KEYB, class VALUE>
int CollideMap<KEYA, KEYB, VALUE>::GetNumBuckets()
//---------------------------------------------------------------------------------
{
	return numBuckets;
}

//---------------------------------------------------------------------------------
template <class KEYA, class KEYB, class VALUE>
void CollideMap<KEYA, KEYB, VALUE>::SetNumBuckets(int numBuckets)
//---------------------------------------------------------------------------------
{
	rehash(numBuckets);
}


//---------------------------------------------------------------------------------
template <class KEYA, class KEYB, class VALUE>
void CollideMap<KEYA, KEYB, VALUE>::printDistribution()
//---------------------------------------------------------------------------------
{

	int i;
	for(i = 0; i < numBuckets; i++){
		int numBuckets = 0;
		if(buckets[i] != NULL){
			HashBucket<MultiKey <KEYA, KEYB>, VALUE>* bucket = buckets[i];
			do{
				HashBucket<MultiKey <KEYA, KEYB>, VALUE>* newBucket = bucket->nextBucket;
				bucket = newBucket;
				numBuckets++;
			}
			while(bucket != NULL);
		}
		printf("\nBucket %d: %d entries.", i, numBuckets);
	}

}

#endif
