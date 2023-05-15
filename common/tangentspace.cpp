#include <vector>
#include "vmath.h"

#include "tangentspace.h"

void computeTangentBasis(
	// inputs
	std::vector<vmath::vec4> & vertices,
	std::vector<vmath::vec2> & uvs,
	std::vector<vmath::vec3> & normals,
	// outputs
	std::vector<vmath::vec3> & tangents,
	std::vector<vmath::vec3> & binormals
){

	for (unsigned int i=0; i<vertices.size(); i+=3 ){

		// Shortcuts for vertices
		vmath::vec4 & v0 = vertices[i+0];
		vmath::vec4 & v1 = vertices[i+1];
		vmath::vec4 & v2 = vertices[i+2];

		// Shortcuts for UVs
		vmath::vec2 & uv0 = uvs[i+0];
		vmath::vec2 & uv1 = uvs[i+1];
		vmath::vec2 & uv2 = uvs[i+2];

		// Edges of the triangle : postion delta
		vmath::vec3 deltaPos1;
		deltaPos1[0] = v1[0]-v0[0];
        deltaPos1[1] = v1[1]-v0[1];
        deltaPos1[2] = v1[2]-v0[2];
		vmath::vec3 deltaPos2;
		deltaPos2[0] = v2[0]-v0[0];
        deltaPos2[1] = v2[1]-v0[1];
        deltaPos2[2] = v2[2]-v0[2];

		// UV delta
		vmath::vec2 deltaUV1 = uv1-uv0;
		vmath::vec2 deltaUV2 = uv2-uv0;

		float r = 1.0f / (deltaUV1[0] * deltaUV2[1] - deltaUV1[1] * deltaUV2[0]);
		vmath::vec3 tangent = (deltaPos1 * deltaUV2[1]   - deltaPos2 * deltaUV1[1])*r;
		vmath::vec3 binormal = (deltaPos2 * deltaUV1[0]   - deltaPos1 * deltaUV2[0])*r;

		// Set the same tangent for all three vertices of the triangle.
		// They will be merged later, in vboindexer.cpp
		tangents.push_back(tangent);
		tangents.push_back(tangent);
		tangents.push_back(tangent);

		// Same thing for binormals
		binormals.push_back(binormal);
		binormals.push_back(binormal);
		binormals.push_back(binormal);

	}

	// See "Going Further"
	for (unsigned int i=0; i<vertices.size(); i+=1 )
	{
		vmath::vec3 & n = normals[i];
		vmath::vec3 & t = tangents[i];
		vmath::vec3 & b = binormals[i];
		
		// Gram-Schmidt orthogonalize
		t = vmath::normalize(t - n * vmath::dot(n, t));
		
		// Calculate handedness
		if (vmath::dot(vmath::cross(n, t), b) < 0.0f){
			t = t * -1.0f;
		}

	}


}


