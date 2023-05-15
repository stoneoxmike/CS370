#ifndef TANGENTSPACE_HPP
#define TANGENTSPACE_HPP

void computeTangentBasis(
	// inputs
	std::vector<vmath::vec4> & vertices,
	std::vector<vmath::vec2> & uvs,
	std::vector<vmath::vec3> & normals,
	// outputs
	std::vector<vmath::vec3> & tangents,
	std::vector<vmath::vec3> & bitangents
);


#endif