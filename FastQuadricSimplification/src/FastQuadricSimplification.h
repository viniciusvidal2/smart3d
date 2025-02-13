#pragma once
#ifdef FastQuadricSimplification_EXPORTS
#define FASTQUADRICSIMPLIFICATION_API __declspec(dllexport)
#else
#define FASTQUADRICSIMPLIFICATION_API __declspec(dllimport)
#endif

#include <vector>
#include <algorithm>
#include "SymetricMatrix.h"


namespace Simplify
{

	class FASTQUADRICSIMPLIFICATION_API FastQuadricSimplification
	{
	public:
		FastQuadricSimplification();
		~FastQuadricSimplification();

		//
		// Main simplification function 
		//
		// target_count  : target nr. of triangles
		// agressiveness : sharpness to increase the threshold.
		//                 5..8 are good numbers
		//                 more iterations yield higher quality
		//
		void simplify_mesh(int target_count, double agressiveness = 7);

		// Global Variables & Strctures
		struct Triangle { int v[3]; double err[4]; int deleted, dirty; vec3f n; };
		struct Vertex { vec3f p; vec3f n; vec3f color; vec3f tCoord; size_t tstart, tcount; SymetricMatrix q; int border; };
		struct Ref { int tid, tvertex; };
		std::vector<Triangle> triangles;
		std::vector<Vertex> vertices;
		std::vector<Ref> refs;
	private:

		// Helper functions
		// Error between vertex and Quadric
		double vertex_error(SymetricMatrix q, double x, double y, double z);
		// Error for one edge
		double calculate_error(int id_v1, int id_v2, vec3f &p_result);
		// Check if a triangle flips when this edge is removed
		bool flipped(vec3f p, int i0, int i1, Vertex &v0, Vertex &v1, std::vector<int> &deleted);
		// Update triangle connections and edge error after a edge is collapsed
		void update_triangles(int i0, Vertex &v, std::vector<int> &deleted, int &deleted_triangles);
		// compact triangles, compute edge error and build reference list
		void update_mesh(int iteration);
		// Finally compact mesh before exiting
		void compact_mesh();

	};
};
