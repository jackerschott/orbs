#include "glWrapper/glwrap.hpp"

namespace gl {

	mesh::mesh(uint nVerts, vertex* verts) : verts(verts), nVerts(nVerts) {
		glGenVertexArrays(1, &vertexArrays);
		glBindVertexArray(vertexArrays);

		glGenBuffers(N_MESH_ARRAYS, vertexArrayBuffers);
		glBindBuffer(GL_ARRAY_BUFFER, vertexArrayBuffers[VP_POSITION]);
		glBufferData(GL_ARRAY_BUFFER, nVerts * sizeof(vertex), verts, GL_DYNAMIC_DRAW);

		glEnableVertexAttribArray(0);
		glVertexAttribPointer(0, 3, GL_FLOAT, false, 0, 0);

		glBindVertexArray(0);
	}

	mesh::~mesh() {
		glDeleteVertexArrays(1, &vertexArrays);
	}

	void mesh::draw() {
		glBindVertexArray(vertexArrays);
		glDrawArrays(GL_TRIANGLES, 0, nVerts);
		glBindVertexArray(0);
	}

}
