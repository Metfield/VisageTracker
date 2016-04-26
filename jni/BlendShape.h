#ifndef BLENDSHAPE_H_
#define BLENDSHAPE_H_

#include <ActionUnitBinding.h>
#include <Helper.h>
#include <string>

class BlendShape {

public:
	// Setters
	inline void AddVertex(const float3 vertex)
	{
		this->vertices.push_back(vertex);
	}

	inline void AddNormal(const float3 vertex)
	{
		this->normals.push_back(vertex);
	}

	inline void ClearVectors()
	{
		this->vertices.clear();
		this->normals.clear();
	}

	// Getters
	inline const std::vector<float3>* DeltaVertices() {return &vertices;}
	inline const std::vector<float3>* DeltaNormals() {return &normals;}

	ActionUnitBinding *actionUnitBinding;
	int id;

public:
	std::vector<float3> vertices;
	std::vector<float3> normals;
};


#endif /* BLENDSHAPE_H_ */
