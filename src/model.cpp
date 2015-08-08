/****************************************
*										*
*				Model.cpp				*
*		class defines for model loading	*
*										*
****************************************/

/************************************
*	Versioning Information			*
************************************/
// 4/12/2004
// Started versioning - GS

#include "model.hpp"

void Model::UpdateTags(int f1, int f2, float offset)
{
	Vector3f temp;
	for(int i = 0; i < numTags; i++)
	{
		tagPos[i].x = tags[f1][i].origin.x*(1.0f-offset) + tags[f2][i].origin.x*(offset);
		tagPos[i].y = tags[f1][i].origin.y*(1.0f-offset) + tags[f2][i].origin.y*(offset);
		tagPos[i].z = tags[f1][i].origin.z*(1.0f-offset) + tags[f2][i].origin.z*(offset);
		temp = tagPos[i];
		tagPos[i] = modelMatrix*tagPos[i];
		temp = tagPos[i];
		temp = temp;
	}
}

int Model::FindTag(char *name)
{
	for(int i = 0; i < numTags; i++)
	{
		if(!strcmp(name,tags[0][i].name))
			return i;
	}
	return -1;
}

Model::Model(char* filename)
{
	tagPos = NULL;
	FILE* file;
	file = fopen(filename,"rb");
	texture = 0;
	if(file)
	{
		
		int tempint;
		char tempchar[4];
		int i;
		int frameoffset,surfaceoffset,tagoffset,eofoffset;
		MODELSURFACE surface;
		TRIANGLE* tris;
		VERTEX* verts;
		
		fread(tempchar,sizeof(char),4,file);
		fread(&tempint,sizeof(int),1,file);
		fread(name,sizeof(char),64,file);
		fread(&flags,sizeof(int),1,file);
		fread(&numFrames,sizeof(int),1,file);
		fread(&numTags,sizeof(int),1,file);
		fread(&numSurfaces,sizeof(int),1,file);
		fread(&tempint,sizeof(int),1,file);
		fread(&frameoffset,sizeof(int),1,file);
		fread(&tagoffset,sizeof(int),1,file);
		fread(&surfaceoffset,sizeof(int),1,file);
		fread(&eofoffset,sizeof(int),1,file);
		
		meshes = new Mesh[numSurfaces];
		fseek(file,frameoffset,SEEK_SET);
        frames = new MODELFRAME[numFrames];
		fread(frames,sizeof(MODELFRAME),numFrames,file);
        
		fseek(file,tagoffset,SEEK_SET);
		tags = new TAG*[numFrames];
		for(i = 0; i < numFrames; i++)
		{
			tags[i] = new TAG[numTags];
			fread(tags[i],sizeof(TAG),numTags,file);
		}
		tagPos = new Vector3f[numTags];
		long offset = surfaceoffset;
		for(i = 0; i < numSurfaces; i++)
		{
			int numread;
			fseek(file,offset,SEEK_SET);
			fread(&surface,sizeof(MODELSURFACE),1,file);
			//surface.numTriangles=1;
			
			tris = new TRIANGLE[surface.numTriangles];
			fseek(file,offset+surface.offsettriangles,SEEK_SET);
			fread(tris,sizeof(TRIANGLE),surface.numTriangles,file);
			fseek(file,offset+surface.offsetst,SEEK_SET);
			TEXCOORD *uv = new TEXCOORD[surface.numVerts];
			fread(uv,sizeof(TEXCOORD),surface.numVerts,file);
			
			fseek(file,offset+surface.offsetxyznormal,SEEK_SET);
			verts = new VERTEX[numFrames*surface.numVerts];
			fread(verts,sizeof(VERTEX),numFrames*surface.numVerts,file);
			offset+=surface.size;
			meshes[i].CreateMesh(surface,verts,tris,uv);
			
			delete [] tris;
			delete [] verts;
			
			tris = NULL;
			verts = NULL;
		}
		fclose(file);
	}
	else
	{
		file = file;
	}

}

void Model::Cleanup()
{
	int i;
	if(meshes)
	{
		for(i = 0; i < numSurfaces; i++)
		{
			meshes[i].Cleanup();
		}
		delete [] meshes;
		meshes = NULL;
	}
	if(tags)
	{
		for(i =0; i < numTags; i++)
		{
			delete [] tags[i];
		}
		delete [] tags;
		tags = NULL;
	}
	if(tagPos)
	{
		delete [] tagPos;
	}
	
}
	 

void Model::Mesh::CreateMesh(const Model::MODELSURFACE &surf, Model::VERTEX *verts, Model::TRIANGLE *tris, Model::TEXCOORD *uv)
{
	texIndex = 0;
	this->numFrames =surf.numFrames;
	this->numTriangles = surf.numTriangles;
	this->verts = new Vector3f*[numFrames];
	this->normals = new Vector3f*[numFrames];
	this->numVerts = surf.numVerts;
	int i,j;
	float lat, longitude;
    this->tris = new Triangle[surf.numTriangles];
	for(i = 0; i < surf.numTriangles; i++)
	{
		this->tris[i].verts[0] = tris[i].indeces[0];
		this->tris[i].verts[1] = tris[i].indeces[1];
		this->tris[i].verts[2] = tris[i].indeces[2];
	}

	

	for(i = 0; i < numFrames; i++)
	{
		this->verts[i] = new Vector3f[surf.numVerts];
		this->normals[i] = new Vector3f[surf.numVerts];

		for(j = 0; j < surf.numVerts; j++)
		{
			this->verts[i][j].set((float)verts[i*surf.numVerts + j].x/64.0f,(float)verts[i*surf.numVerts + j].y/64.0f,(float)verts[i*surf.numVerts + j].z/64.0f);
			if(i == 0 && j == 0){
				minPoint = this->verts[i][j];
				maxPoint = this->verts[i][j];
			}
			else{
				if(this->verts[i][j].x < minPoint.x) minPoint.x = this->verts[i][j].x;
				if(this->verts[i][j].y < minPoint.y) minPoint.y = this->verts[i][j].y;
				if(this->verts[i][j].z < minPoint.z) minPoint.z = this->verts[i][j].z;
				if(this->verts[i][j].x > maxPoint.x) maxPoint.x = this->verts[i][j].x;
				if(this->verts[i][j].y > maxPoint.y) maxPoint.y = this->verts[i][j].y;
				if(this->verts[i][j].z > maxPoint.z) maxPoint.z = this->verts[i][j].z;
			}

			lat = (float)(verts[i*surf.numVerts+j].normal & 0xFF);
			lat *= 3.1485f*2.0f/255.0f;
			longitude = (float)((verts[i*surf.numVerts+j].normal >> 8) & 0xFF);
			longitude *= 3.1485f*2.0f/255.0f;
			this->normals[i][j].y = cosf(longitude);
			this->normals[i][j].x = sinf(longitude);
			this->normals[i][j].z = this->normals[i][j].x*sinf(lat);
			this->normals[i][j].x *= cosf(lat);
			this->normals[i][j].normalize();
		}
	}
	this->uv = uv;
}

void Model::Mesh::TransformPermanent(Matrix4f &m)
{
	for(int i = 0; i < numFrames; i++)
	{
		for(int j = 0; j < numVerts; j++)
		{
			verts[i][j] = m*verts[i][j];
		}
	}
	maxPoint = m*maxPoint;
	minPoint = m*minPoint;
}

void Model::Mesh::Cleanup()
{
	int i;
	if(tris)
	{
		delete [] tris;
		tris = NULL;
	}
	if(verts)
	{
		for(i = 0; i < numFrames; i++)
		{
			if(verts[i])
			{
				delete [] verts[i];
				verts[i] = NULL;
			}
		}
		delete [] verts;
	}
	if(normals)
	{
		for(i = 0; i < numFrames; i++)
		{
			if(normals[i])
			{
				delete [] normals[i];
				normals[i] = NULL;
			}
		}
		delete [] normals;
	}
	if(uv)
	{
		delete [] uv;
	}

	
}

void Model::draw(Light *l)
{
	glBindTexture(GL_TEXTURE_2D,texture);
	for(int i = 0; i < numSurfaces; i++)
	{
		meshes[i].draw(l,modelMatrix);
	}
}

void Model::draw(Light *l, Vector3f p, int f1, int f2,float offset)
{
	for(int i = 0; i < numSurfaces; i++)
	{
		meshes[i].draw(l,modelMatrix,p,f1,f2,offset);
	}
	
}

void Model::Mesh::draw(Light *l,Matrix4f m)
{
	Vector3f color(1.0f,1.0f,1.0f);
	Vector3f ambient(0.2f,0.2f,0.2f);
	Vector3f v;
	Vector3f n;
	
	Matrix4f inverse = m.transpose();
	inverse = inverse.invert();
	glEnable(GL_LIGHTING);
	glEnable(GL_LIGHT0);
	glEnable(GL_TEXTURE_2D);

	float outcolor[3];
	if(l)
	{
		
		for(int i = 0; i < numTriangles; i++)
		{
			glBegin(GL_TRIANGLES);
			v = m*verts[0][tris[i].verts[0]];
			n = inverse * normals[0][tris[i].verts[0]];
			n.normalize();
			//l->computeColor(n,color,ambient,v,outcolor);
			//glColor3fv(outcolor);
				glTexCoord2f(uv[tris[i].verts[0]].uv[0],uv[tris[i].verts[0]].uv[1]);
				glNormal3f(n.x,n.y,n.z);
				glVertex3f(v.x,v.y,v.z);
			v = m*verts[0][tris[i].verts[1]];
			n = inverse * normals[0][tris[i].verts[1]];
			n.normalize();
			//l->computeColor(n,color,ambient,v,outcolor);
			//glColor3fv(outcolor);
				glTexCoord2f(uv[tris[i].verts[1]].uv[0],uv[tris[i].verts[1]].uv[1]);
			glNormal3f(n.x,n.y,n.z);
				glVertex3f(v.x,v.y,v.z);
			v = m*verts[0][tris[i].verts[2]];
			n = inverse * normals[0][tris[i].verts[2]];
			n.normalize();
			//l->computeColor(n,color,ambient,v,outcolor);
			//glColor3fv(outcolor);
				glTexCoord2f(uv[tris[i].verts[2]].uv[0],uv[tris[i].verts[2]].uv[1]);
				glNormal3f(n.x,n.y,n.z);
				glVertex3f(v.x,v.y,v.z);
			glEnd();
			
				
		}
	
	}
	else
	{
		for(int i = 0; i < numTriangles; i++)
		{
			
			glBegin(GL_TRIANGLES);
				v = m*verts[0][tris[i].verts[0]];
				glVertex3f(v.x,v.y,v.z);
				v = m*verts[0][tris[i].verts[1]];
				glVertex3f(v.x,v.y,v.z);
				v = m*verts[0][tris[i].verts[2]];
				glVertex3f(v.x,v.y,v.z);
			glEnd();
		}
	}
	glDisable(GL_TEXTURE_2D);
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
}

void Model::Mesh::draw(Light *l,Matrix4f m,Vector3f p,int f1, int f2,float offset)
{
	Vector3f color(1.0f,1.0f,1.0f);
	Vector3f ambient(0.1f,0.1f,0.1f);
	Vector3f v,v1,v2,v3;
	Vector3f n,n1,n2,n3;
	
	Matrix4f inverse = m.transpose();
	inverse = inverse.invert();
	Matrix4f lala = inverse * m.transpose();
	lala = lala;
	//glColor4f(1.0f,1.0f,1.0f,1.0f);
	//glEnable(GL_LIGHTING);
	//glEnable(GL_LIGHT0);
	glColor3f(1.0f,1.0f,1.0f);
	float outcolor[3];
	if(l)
	{
		for(int i = 0; i < numTriangles; i++)
		{
			glBegin(GL_TRIANGLES);
			v = m*(verts[f1][tris[i].verts[0]]*(1.0f-offset)+verts[f2][tris[i].verts[0]]*offset);
			n = inverse * (normals[f1][tris[i].verts[0]]*(1.0f-offset)+normals[f2][tris[i].verts[0]]*offset);
			n.normalize();
			n1 = n;
			v1 = v;
			l->computeColor(n,color,ambient,p+v,outcolor);
			glColor3fv(outcolor);
				//glNormal3f(n.x,n.y,n.z);
				glVertex3f(v.x,v.y,v.z);
			v = m*(verts[f1][tris[i].verts[1]]*(1.0f-offset)+verts[f2][tris[i].verts[1]]*offset);
			n = inverse * (normals[f1][tris[i].verts[1]]*(1.0f-offset)+normals[f2][tris[i].verts[1]]*offset);
			n.normalize();
			n2 = n;
			v2 = v;
			l->computeColor(n,color,ambient,p+v,outcolor);
			glColor3fv(outcolor);
				//glNormal3f(n.x,n.y,n.z);
				glVertex3f(v.x,v.y,v.z);
			v = m*(verts[f1][tris[i].verts[2]]*(1.0f-offset)+verts[f2][tris[i].verts[2]]*offset);
			n = inverse * (normals[f1][tris[i].verts[2]]*(1.0f-offset)+normals[f2][tris[i].verts[2]]*offset);
			n.normalize();
			n3 = n;
			v3 = v;
			l->computeColor(n,color,ambient,p+v,outcolor);
			glColor3fv(outcolor);
				//glNormal3f(n.x,n.y,n.z);
				glVertex3f(v.x,v.y,v.z);
			
			glEnd();
			
				
		}
		
	
	}
	glDisable(GL_LIGHTING);
	glDisable(GL_LIGHT0);
}

Animation* FindAnimation(int state, Animation* a, int numA)
{
	for(int i = 0; i < numA; i++)
	{  
		if(a[i].state == state)
			return a + i;
	}
	return NULL;
}


