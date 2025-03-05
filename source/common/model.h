/******************************************************************************
 * @file	model.h
 * @brief	load test model
 *          use AMD demo format
 *****************************************************************************/

#pragma once

/*
================================================================================
Model
================================================================================
*/
struct Batch {
	uint32_t	firstIndex;
	uint32_t	nIndices;
	uint32_t	firstVertex;
	uint32_t	nVertices;
};

struct ModelHeader {
	uint32_t	version;
	uint32_t	nBatches;
	uint32_t	nVertices;
	uint32_t	nIndices;
	uint32_t	vertexSize;
	uint32_t	indexSize;
};

struct Model {
	uint32_t	nBatches;
	uint32_t	nVertices;
	uint32_t	nIndices;
	uint32_t	vertexSize;
	uint32_t	indexSize;
	Batch *		batches;
	char *		vertices;
	char *		indices;
};

bool	Model_Load(const wchar_t *fileName, Model *m);
void	Model_Free(Model *m);
