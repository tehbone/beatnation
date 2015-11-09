#ifndef MODEL_HPP_
#define MODEL_HPP_

#include "utils.hpp"
#include "light.hpp"



class Model;
class Mesh;

class Triangle
{
public:
	int verts[3];
};



class Model
{
public:
	Model(char* filename);
	void Cleanup();
	void draw(Light *l);
	void draw(Light *l,Vector3f p,int f1, int f2,float offset);
	void UpdateTags(int f1, int f2, float offset);
	int FindTag(char* name);
//---------------Helper classes--------------------
		typedef struct
	{
		float x,z,y;
	} VECTOR;
	
	typedef struct 
	{
		VECTOR min_bounds;
		VECTOR max_bounds;
		VECTOR local_origin;
		float radius;
		char name[16];
	} MODELFRAME;

	typedef struct
	{
		char name[64];
		VECTOR origin;
		VECTOR axis[3];
	} TAG;
	typedef struct
	{
		char ident[4];
		char name[64];
		long flags;
		int numFrames;
		int numShaders;
		int numVerts;
		int numTriangles;
		int offsettriangles;
		int offsetshaders;
		int offsetst;
		int offsetxyznormal;
		int size;
	} MODELSURFACE;
	
	typedef struct
	{
		char name[64];
		int shader;
	} SHADER;
	typedef struct
	{
		int indeces[3];
	} TRIANGLE;
	typedef struct
	{
		float uv[2];
	} TEXCOORD;
	typedef struct
	{
		short x;
		short z;
		short y;
		short normal;
	} VERTEX;

	class Mesh
	{
	public:
		
		void CreateMesh(const MODELSURFACE &surf, VERTEX *verts, TRIANGLE *tris, TEXCOORD *uv);
		void draw(Light *l,Matrix4f m);
		void draw(Light *l,Matrix4f m, Vector3f p, int f1, int f2,float offset);
		void TransformPermanent(Matrix4f &m);
		void Cleanup();
		
		//Mesh & Mesh::operator=(Mesh m);
		TEXCOORD *uv;
		Vector3f **verts;
		Vector3f **normals;
		int numFrames;
		int numTriangles;
		Triangle *tris;
		int numVerts;
		Vector3f minPoint;
		Vector3f maxPoint;
		int texIndex;
	};


//--------------------------------------------

private:
	int flags;
	int numFrames;
	int numSurfaces;
	int numSkins;
	int numTags;
	
	bool hastexture;
public:
	int texture;
	Mesh *meshes;
	MODELFRAME* frames;
	Matrix4f modelMatrix;
	TAG** tags;
	char name[64];
	Vector3f *tagPos;


};


typedef struct
{
	char name[256];
	int begin;
	int end;
	int beginloop;
	int state;
	bool lh;
	bool rh;
	bool lf;
	bool rf;
} Animation;

Animation* FindAnimation(int state, Animation* a, int numA);
#endif /* MODEL_HPP_ */
	
