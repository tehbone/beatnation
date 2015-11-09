#include <algorithm>
#include "light.hpp"

void
Light::computeColor(Vector3f norm, Vector3f c, Vector3f ambient, Vector3f p, float* out)
{
	static float shade[36] = {0,0,0,0,0.25f,0.25f,0.25f,0.25f,0.25f,0.25f,0.50f,0.50f,0.50f,0.50f,0.50f,0.50f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.00f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f,1.0f};
	Vector3f temp = pos-p;
	temp.normalize();
	switch(type) {
	case LT_POINT:
		out[0] = ambient.x/* c.x*/ + std::max(temp.dot(norm),0.f)/*c.x*/;
		out[0] = shade[(int)std::min((out[0] * 35.0f),35.f)]*c.x;
		out[1] = ambient.y /* c.y*/ + std::max(temp.dot(norm),0.f)/*c.y*/;
		out[1] = shade[(int)std::min((out[1] * 35.0f),35.f)]*c.y;
		out[2] = ambient.z /* c.z*/ + std::max(temp.dot(norm),0.f)/*c.z*/;
		out[2] = shade[(int)std::min((out[2] * 35.0f),35.f)]*c.z;
		break;
	}
}

