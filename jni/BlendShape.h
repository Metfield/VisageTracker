#ifndef BLENDSHAPE_H_
#define BLENDSHAPE_H_

#include <ActionUnitBinding.h>
#include <Helper.h>
#include <string>

class BlendShape {

public:
	// Setters
	inline void SetDeltaVertices(const std::vector<float3> *vertices_) {vertices = vertices_;}
	inline void SetDeltaNormals(const std::vector<float3> *normals_) {normals = normals_;}

	// Getters
	inline const std::vector<float3>* DeltaVertices() {return vertices;}
	inline const std::vector<float3>* DeltaNormals() {return normals;}

	ActionUnitBinding *actionUnitBinding;
	std::string name;

private:
	const std::vector<float3> *vertices;
	const std::vector<float3> *normals;
};


#endif /* BLENDSHAPE_H_ */
