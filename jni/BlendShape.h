#ifndef BLENDSHAPE_H_
#define BLENDSHAPE_H_

#include <ActionUnitBinding.h>
#include <Helper.h>

class BlendShape {

public:
	// Setters
	inline void SetDeltaVertices(const std::vector<float3> *vertices_) {vertices = vertices_;}
	inline void SetDeltaNormals(const std::vector<float3> *normals_) {normals = normals_;}
	inline void SetDeltaTangents(const std::vector<float3> *tangents_) {tangents = tangents_;}

	// Getters
	inline const std::vector<float3>* DeltaVertices() {return vertices;}
	inline const std::vector<float3>* DeltaNormals() {return normals;}
	inline const std::vector<float3>* DeltaTangents() {return tangents;}

	ActionUnitBinding *actionUnitBinding;

private:
	const std::vector<float3> *vertices;
	const std::vector<float3> *normals;
	const std::vector<float3> *tangents;
};


#endif /* BLENDSHAPE_H_ */
