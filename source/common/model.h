//2016-01-17 Sun.

#pragma once

/*
================================================================================
Model
================================================================================
*/
struct Batch {
	uint32  firstIndex;
	uint32  nIndices;
	uint32  firstVertex;
	uint32  nVertices;
};

struct ModelHeader {
	uint32  version;
	uint32  nBatches;
	uint32  nVertices;
	uint32  nIndices;
	uint32  vertexSize;
	uint32  indexSize;
};

struct Model {
	uint32  nBatches;
	uint32  nVertices;
	uint32  nIndices;
	uint32  vertexSize;
	uint32  indexSize;
	Batch * batches;
	char *  vertices;
	char *  indices;
};

bool Model_Load(const wchar_t *fileName, Model *m);
void Model_Free(Model *m);
