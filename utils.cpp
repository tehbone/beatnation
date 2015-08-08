/****************************************
*										*
*				Utils.cpp				*
*    definitions for the utilities		*
*										*
****************************************/

#include "utils.h"


Vector3f Vector3f::cross(const Vector3f v)
{
	Vector3f temp;
	temp.x = y*v.z - z*v.y;
	temp.y = z*v.x - x*v.z;
	temp.z = x*v.y - y*v.x;
	return temp;
}

float Vector3f::dot(const Vector3f v)
{
	return x*v.x + y*v.y + z*v.z;
}

float Vector3f::magnitude()
{
	return sqrtf(dot(*this));
}

void Vector3f::normalize()
{
	*this = *this / magnitude();
}

void Vector3f::set(float x, float y, float z)
{
	this->x = x;
	this->y = y;
	this->z = z;
}

Vector3f Vector3f::operator*(float f)
{
	Vector3f temp(x*f,y*f,z*f);
	return temp;
}

Vector3f Vector3f::operator/(float f)
{
	Vector3f temp(x/f,y/f,z/f);
	return temp;
}

Vector3f Vector3f::operator+(Vector3f v)
{
	Vector3f temp(x+v.x,y+v.y,z+v.z);
	return temp;
}

Vector3f & Vector3f::operator=(Vector3f v)
{
	x = v.x;
	y = v.y;
	z = v.z;
	return *this;
}

Vector3f Vector3f::operator-(Vector3f v)
{
	Vector3f temp(x-v.x,y-v.y,z-v.z);
	return temp;
}

float Vector4f::dot(const Vector3f v)
{
	return getVector().dot(v);
}

float Vector4f::dot(const Vector4f v)
{
	return x*v.x + y*v.y + z * v.z + w*v.w;
}

Vector3f Vector4f::getVector()
{
	Vector3f temp(x,y,z);
	return temp;
}

float Vector4f::magnitude()
{
	return sqrtf(dot(*this));
}

void Vector4f::normalize()
{
	*this = *this/magnitude();
}

Vector4f Vector4f::operator*(float f)
{
	Vector4f temp(x*f,y*f,z*f,w*f);
	return temp;
}

Vector4f Vector4f::operator/(float f)
{
	Vector4f temp(x/f,y/f,z/f,w/f);
	return temp;
}

Vector4f Vector4f::operator+(Vector4f v)
{
	Vector4f temp(x+v.x,y+v.y,z+v.z,w+v.w);
	return temp;
}

Vector4f Vector4f::operator-(Vector4f v)
{
	Vector4f temp(x-v.x,y-v.y,z-v.z,w-v.w);
	return temp;
}

Vector4f & Vector4f::operator=(Vector4f v)
{
	x=v.x;
	y=v.y;
	z=v.z;
	w=v.w;
	return *this;
}

void Vector4f::setVector(Vector3f v,float f)
{
	x = v.x;
	y = v.y;
	z = v.z;
	w = f;
}

void Vector4f::set(float x, float y, float z, float w)
{
	this->x = x;
	this->y = y;
	this->z = z;
	this->w = w;
}

void Matrix4f::SetData(int r, int c, float d)
{
	data[r][c] = d;
}

void Matrix4f::SetColumn(int c, Vector4f v)
{
	data[0][c] = v.x;
	data[1][c] = v.y;
	data[2][c] = v.z;
	data[3][c] = v.w;
}

void Matrix4f::SetColumn(int c, Vector3f v)
{
	data[0][c] = v.x;
	data[1][c] = v.y;
	data[2][c] = v.z;
	data[3][c] = c ==3 ? 1.0f : 0.0f;
}

void Matrix4f::SetRow(int r, Vector4f v)
{
	data[r][0] = v.x;
	data[r][1] = v.y;
	data[r][2] = v.z;
	data[r][3] = v.w;
}

Matrix4f Matrix4f::scale(float x,float y, float z)
{
	Matrix4f temp;
	temp.SetData(0,0,x);
	temp.SetData(1,1,y);
	temp.SetData(2,2,z);
	return *this * temp;
}



float Matrix4f::GetData(int r, int c)
{
	return data[r][c];
}

Vector4f Matrix4f::GetRow(int r)
{
	Vector4f temp(data[r][0],data[r][1],data[r][2],data[r][3]);
	return temp;
}

Vector4f Matrix4f::GetColumn(int c)
{
	Vector4f temp(data[0][c],data[1][c],data[2][c],data[3][c]);
	return temp;
}

//***********************************************
// Name:  Matrix4f::invert()
// Desc:  Inverts the matrix, assuming the matrix
//    is orthogonal
//***********************************************
Matrix4f Matrix4f::invert()
{
	Matrix4f temp;
	for(int i = 0; i < 4; i++)
	{
		for(int j = 0; j < 4; j++)
		{
			temp.SetData(j,i,cofactor(i,j));
			
		}
	}
	temp = temp / det();
	return temp;
}

Matrix4f Matrix4f::transpose()
{
	Matrix4f temp;
	for(int i = 0; i < 4; i++)
	{
		for(int j = 0; j < 4; j++)
		{
			temp.SetData(j,i,data[i][j]);
		}
	}
	return temp;
}

float Matrix4f::det()
{
	float sum = 0;
	sum+= data[0][0]*(data[1][1]*data[2][2]*data[3][3]-data[1][1]*data[2][3]*data[3][2] +
		              data[1][2]*data[2][3]*data[3][1]-data[1][2]*data[2][1]*data[3][3] +
					  data[1][3]*data[2][1]*data[3][2]-data[1][3]*data[2][2]*data[3][1]);

	sum-= data[0][1]*(data[1][0]*data[2][2]*data[3][3]-data[1][0]*data[2][3]*data[3][2] +
		              data[1][2]*data[2][3]*data[3][0]-data[1][2]*data[2][0]*data[3][3] +
					  data[1][3]*data[2][0]*data[3][2]-data[1][3]*data[2][2]*data[3][0]);

	sum+= data[0][2]*(data[1][0]*data[2][1]*data[3][3]-data[1][0]*data[2][3]*data[3][1] +
		              data[1][1]*data[2][3]*data[3][0]-data[1][1]*data[2][0]*data[3][3] +
					  data[1][3]*data[2][0]*data[3][1]-data[1][3]*data[2][1]*data[3][0]);

	sum-= data[0][3]*(data[1][0]*data[2][1]*data[3][2]-data[1][0]*data[2][2]*data[3][1] +
		              data[1][1]*data[2][2]*data[3][0]-data[1][1]*data[2][0]*data[3][2] +
					  data[1][2]*data[2][0]*data[3][1]-data[1][2]*data[2][1]*data[3][0]);
	return sum;
}

float Matrix4f::cofactor(int i, int j)
{
	float factor = 1.0f;
	if((i+j) %2==1)
	{
		factor = -1.0f;
	}
	int rows[3];
	int cols[3];
	for(int k = 0; k < 3; k++)
	{
		rows[k] = k >= i ? k+1 : k;
		cols[k] = k >= j ? k+1 : k;
	}
	return factor*(data[rows[0]][cols[0]]*data[rows[1]][cols[1]]*data[rows[2]][cols[2]]-data[rows[0]][cols[0]]*data[rows[1]][cols[2]]*data[rows[2]][cols[1]] +
				   data[rows[0]][cols[1]]*data[rows[1]][cols[2]]*data[rows[2]][cols[0]]-data[rows[0]][cols[1]]*data[rows[1]][cols[0]]*data[rows[2]][cols[2]] +
				   data[rows[0]][cols[2]]*data[rows[1]][cols[0]]*data[rows[2]][cols[1]]-data[rows[0]][cols[2]]*data[rows[1]][cols[1]]*data[rows[2]][cols[0]]);
}

Vector3f Matrix4f::operator*(Vector3f v)
{
	Vector3f temp;
	temp.x = v.x * data[0][0] + v.y * data[0][1] + v.z * data[0][2];
	temp.y = v.x * data[1][0] + v.y * data[1][1] + v.z * data[1][2];
	temp.z = v.x * data[2][0] + v.y * data[2][1] + v.z * data[2][2];
	return temp;
}

Matrix4f Matrix4f::operator*(float f)
{
	Matrix4f m;
	for(int i = 0; i < 4; i++)
	{
		for(int j = 0; j < 4; j++)
		{
			m.SetData(i,j,data[i][j] * f);
		}
	}
	return m;
}

Matrix4f Matrix4f::operator/(float f)
{
	Matrix4f m;
	for(int i = 0; i < 4; i++)
	{
		for(int j = 0; j < 4; j++)
		{
			m.SetData(i,j,data[i][j] / f);
		}
	}
	return m;
}
	
void Matrix4f::SetData(float* f)
{
	for(int j = 0; j < 4; j++)
	{
		for(int i = 0; i<4; i++)
		{
 			data[i][j] = f[i+j*4];
		}
	}
}

Matrix4f Matrix4f::operator*(Matrix4f m)
{
	Vector4f temp;
	Matrix4f tempmatrix;
	for(int i = 0; i < 4; i++)
	{
		temp=m.GetColumn(i);
		for(int j = 0; j < 4; j++)
		{
			tempmatrix.SetData(j,i,GetRow(j).dot(temp));
		}
	}
	return tempmatrix;
}

Vector4f Matrix4f::operator*(Vector4f v)
{
	Vector4f temp;
	temp.x = v.dot(GetRow(0));
	temp.y = v.dot(GetRow(1));
	temp.z = v.dot(GetRow(2));
	temp.w = v.dot(GetRow(3));
	return temp;
}

Matrix4f Matrix4f::translate(Vector3f v)
{
	Matrix4f temp;
	temp.SetColumn(3,v);
	return *this * temp;
}


Quaternion Quaternion::conj()
{
	Quaternion temp(s,v*-1.0f);
	return temp;
}

float Quaternion::magnitude()
{
	return sqrtf(s*s + v.dot(v));
}

Quaternion Quaternion::operator*(Quaternion q)
{
	Quaternion temp(s*q.s-v.dot(q.v),v * q.s + q.v*s + v.cross(q.v));
	return temp;
}

Quaternion Quaternion::operator*(float f)
{
	Quaternion temp(s*f,v*f);
	return temp;
}

Quaternion Quaternion::operator+(Quaternion q)
{
	Quaternion temp(s+q.s,v+q.v);
	return temp;
}

Quaternion Quaternion::operator-(Quaternion q)
{
	Quaternion temp(s-q.s,v-q.v);
	return temp;
}

Quaternion Quaternion::operator/(float f)
{
	Quaternion temp(s/f, v/f);
	return temp;
}

Quaternion Quaternion::rotate(float angle)
{
	Vector3f norm = v;
	norm.normalize();
	Quaternion temp(cosf(0.5f*angle),norm*sinf(0.5f*angle));
	return temp;
}

Quaternion::Quaternion()
{
	s = 0;
}

Quaternion::Quaternion(float f,Vector3f vector) : s(f),v(vector)
{
}

unsigned int LoadTGATexture(char *filename)
{
	unsigned int texID;
	targaData *data = LoadTarga(filename);
	glGenTextures(1,&texID);
	glBindTexture(GL_TEXTURE_2D,texID);
	glTexImage2D(GL_TEXTURE_2D,0,GL_RGBA,data->width,data->height,0,GL_RGBA,GL_UNSIGNED_BYTE,data->data);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MIN_FILTER,GL_LINEAR);
	glTexParameteri(GL_TEXTURE_2D,GL_TEXTURE_MAG_FILTER,GL_LINEAR);
	if(data->data)
	{
		delete [] data->data;
		data->data = NULL;
	}
	delete data;
	data = NULL;
	return texID;
}

StringMap* AddEntry(StringMap *s, char* key, void* o)
{
	StringMap *it;
	if(s == NULL)
	{
		it = new StringMap(key,o);
		return it;
	}
	for(it = s; it->nextMap != NULL; it = it->nextMap);
	it->nextMap = new StringMap(key,o);
	return s;
}

void DeleteStringMap(StringMap *s)
{
	StringMap* it = s;
	StringMap* temp;
	while(it != NULL)
	{
		delete it->object;
		temp = it;
		it = it->nextMap;
		delete temp;
	}
}

void* FindEntry(StringMap *s, char* key)
{
	for(StringMap *it=s; it !=NULL; it = it->nextMap)
	{
		if(!strcmp(key,it->key))
			return it->object;
	}
	return NULL;
}
