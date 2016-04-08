#include <BlendShapeTriMesh.h>
#include <GLES2/gl2.h>

BlendShapeTriMesh::BlendShapeTriMesh(triMeshStruct* meshData_) {
	meshData = meshData_;

	// Copy all original data so we can freely interpolate later in the struct.
	vertices = meshData->vertices;
	normals = meshData->normals;
	tangents = meshData->tangents;
	texCoords = meshData->texCoords;

	// TODO: Fix OpenGL stuff
	/*
	glGenVertexArrays(1, &meshData->vaob);
	glBindVertexArray(meshData->vaob);

	glGenBuffers(1, &meshData->vertex_bo);
	glBindBuffer(GL_ARRAY_BUFFER, meshData->vertex_bo);
	glBufferData(GL_ARRAY_BUFFER, meshData->vertices.size() * sizeof(float3), &meshData->vertices[0].x, GL_STATIC_DRAW);
	glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(0);

	glGenBuffers(1, &meshData->normals_bo);
	glBindBuffer(GL_ARRAY_BUFFER, meshData->normals_bo);
	glBufferData(GL_ARRAY_BUFFER, meshData->normals.size() * sizeof(float3), &meshData->normals[0].x, GL_STATIC_DRAW);
	glVertexAttribPointer(1, 3, GL_FLOAT, false, 0, 0);
	glEnableVertexAttribArray(1);

	if(meshData->texCoords.size() > 0){
		glGenBuffers(1, &meshData->texCoords_bo);
		glBindBuffer(GL_ARRAY_BUFFER, meshData->texCoords_bo);
		glBufferData(GL_ARRAY_BUFFER, meshData->texCoords.size() * sizeof(texCoord), &meshData->texCoords[0].u, GL_STATIC_DRAW);
		glVertexAttribPointer(2, 2, GL_FLOAT, false, 0, 0);
	}
	glEnableVertexAttribArray(2);
	*/
}


triMeshStruct* BlendShapeTriMesh::GetInterpolatedMesh() {

	// Reset all mesh data then start interpolating
	meshData->vertices = vertices;
	meshData->normals = normals;
	meshData->tangents = tangents;

	// If blend shapes are present, interpolate new vertex data
	if(!blendShapes.empty()) {
		float aubValue;

		for(int i = 0; i < vertices.size(); i++) {
			for(int j = 0; j < blendShapes.size(); j++) {
				aubValue = blendShapes[j].actionUnitBinding->GetValue();
				meshData->vertices[i] += (*blendShapes[j].DeltaVertices())[j] * aubValue;
				meshData->normals[i]  += (*blendShapes[j].DeltaNormals())[j]  * aubValue;
				meshData->tangents[i] += (*blendShapes[j].DeltaTangents())[j] * aubValue;
			}
		}
	}
	return meshData;
}

