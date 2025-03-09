/******************************************************************************
 * @file	model.cpp
 * @brief
 *****************************************************************************/

#include "common.h"

/*
================================================================================
Model
================================================================================
*/
bool Model_Load(const wchar_t *fileName, Model *m) {
	wchar_t fullFileName[MAX_PATH];

#if defined(_MSC_VER)
	swprintf_s(fullFileName, MAX_PATH, L"%s/%s", MODEL_DIR, fileName);
#endif

#if defined(__GNUC__)
	swprintf_s(fullFileName, MAX_PATH, L"%ls/%ls", MODEL_DIR, fileName);
#endif

	FILE *file = nullptr;
	_wfopen_s(&file, fullFileName, L"rb");

	if (!file) {
		return false;
	}

	ModelHeader header;
	fread(&header, sizeof(header), 1, file);
	if (header.version != 0) {
		fclose(file);
		return false;
	}

	m->nBatches = header.nBatches;
	m->nVertices = header.nVertices;
	m->nIndices = header.nIndices;
	m->vertexSize = header.vertexSize;
	m->indexSize = header.indexSize;

	m->batches = (Batch*)malloc(sizeof(Batch) * m->nBatches);
	fread(m->batches, sizeof(Batch), m->nBatches, file);
	m->vertices = (char*)malloc(m->vertexSize * m->nVertices);
	fread(m->vertices, m->vertexSize, m->nVertices, file);
	if (m->nIndices) {
		m->indices = (char*)malloc(m->indexSize * m->nIndices);
		fread(m->indices, m->indexSize, m->nIndices, file);
	}
	else {
		m->indices = nullptr;
	}
	
	fclose(file);
	return true;
}

void Model_Free(Model *m) {
	if (m->indices) {
		free(m->indices);
		m->indices = nullptr;
	}

	if (m->vertices) {
		free(m->vertices);
		m->vertices = nullptr;
	}

	if (m->batches) {
		free(m->batches);
		m->batches = nullptr;
	}
}
