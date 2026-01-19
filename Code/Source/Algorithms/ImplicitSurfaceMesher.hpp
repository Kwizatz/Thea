//============================================================================
//
// This file is part of the Thea toolkit.
//
// This software is distributed under the BSD license, as detailed in the
// accompanying LICENSE.txt file. Portions are derived from other works:
// their respective licenses and copyright information are reproduced in
// LICENSE.txt and/or in the relevant source files.
//
// Author: Siddhartha Chaudhuri
// First version: 2009
//
//============================================================================

#ifndef __Thea_Algorithms_ImplicitSurfaceMesher_hpp__
#define __Thea_Algorithms_ImplicitSurfaceMesher_hpp__

#include "../Common.hpp"
#include "../Array.hpp"
#include "../Ball3.hpp"
#include "../Math.hpp"
#include "../UnorderedMap.hpp"
#include "../Graphics/IncrementalMeshBuilder.hpp"
#include "../Graphics/MeshType.hpp"
#include "../ThirdParty/BloomenthalPolygonizer/polygonizer.h"

#if THEA_ENABLE_CGAL
#if defined(__clang__)
// sprintf in boost::lexical_cast triggers a deprecation warning
#  pragma clang diagnostic push
#  pragma clang diagnostic ignored "-Wdeprecated-declarations"
#endif
#    include <CGAL/Mesh_triangulation_3.h>
#    include <CGAL/Mesh_complex_3_in_triangulation_3.h>
#    include <CGAL/Mesh_criteria_3.h>
#    include <CGAL/Labeled_mesh_domain_3.h>
#    include <CGAL/make_mesh_3.h>
#if defined(__clang__)
#  pragma clang diagnostic pop
#endif
#endif

namespace Thea {
namespace Algorithms {

/**
 * Mesh an implicit surface defined as the zero level set of a 3D function. The function type <code>F</code> must be
 *
 * \code
 * <real-number-type> (*F)(Vector3 const & p)
 * \endcode
 *
 * or (if it is a functor class) must have the public member function
 *
 * \code
 * <real-number-type> operator()(Vector3 const & p) const
 * \endcode
 */
class THEA_API ImplicitSurfaceMesher
{
  private:
    /** Evaluates function as required by Bloomenthal polygonizer. */
    template <typename FunctorT> struct BloomenthalEval : public BloomenthalPolygonizer::ImplicitFunction
    {
      FunctorT * func;

      /** Constructor. */
      BloomenthalEval(FunctorT * func_) : func(func_)
      {
        alwaysAssertM(func, "ImplicitSurfaceMesher: Surface evaluator function cannot be null");
      }

      /** Evaluate the function. */
      float eval(float x, float y, float z) { return static_cast<float>((*func)(Vector3(x, y, z))); }

    }; // BloomenthalEval

#if THEA_ENABLE_CGAL

    /** Evaluates function as required by CGAL implicit surface wrapper. */
    template <typename FunctorT> struct CgalEval
    {
      typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
      typedef K::Point_3 Point_3;
      typedef K::FT FT;

      FunctorT * func;

      /** Constructor. */
      CgalEval(FunctorT * func_) : func(func_) {}

      /** Evaluate the function. */
      FT operator()(Point_3 const & p) const { return static_cast<FT>((*func)(Vector3(p.x(), p.y(), p.z()))); }

    }; // CgalEval

#endif // THEA_ENABLE_CGAL

  public:
    /** %Options controlling mesh generation by polygonizing the implicit surface. */
    struct THEA_API Options
    {
      /** %Options for meshing the implicit surface via the method of Bloomenthal [1994]. */
      struct THEA_API Bloomenthal
      {
        double cell_size;           ///< Size of the polygonizing cell (negative to select default).
        int max_search_steps;       /**< Limit to how far away we will look for components of the implicit surface (negative to
                                         select default). */
        bool tetrahedralize_cubes;  /**< If true, cubes are divided into tetrahedra and polygonized. Else, cubes are polygonized
                                         directly. (Default: false) */

        /** Constructor. */
        Bloomenthal(double cell_size_ = -1, int max_search_steps_ = -1, bool tetrahedralize_cubes_ = false)
        : cell_size(cell_size_), max_search_steps(max_search_steps_), tetrahedralize_cubes(tetrahedralize_cubes_) {}

        /** Default options. */
        static Bloomenthal const & defaults() { static Bloomenthal const options; return options; }
      };

#if THEA_ENABLE_CGAL

      /**
       * %Options for meshing the implicit surface via the method of Boissonnat and Oudot [2005].
       *
       * @see http://www.cgal.org/Manual/last/doc_html/cgal_manual/Surface_mesher_ref/Class_Surface_mesh_default_criteria_3.html
       */
      struct THEA_API BoissonnatOudot
      {
        double min_facet_angle;        ///< Minimum facet angle, in radians (negative to select default).
        double min_delaunay_radius;    ///< Minimum radius of surface Delaunay balls (negative to select default).
        double min_center_separation;  ///< Minimum center-center distance (negative to select default).

        /** Constructor. */
        BoissonnatOudot(double min_facet_angle_ = -1, double min_delaunay_radius_ = -1, double min_center_separation_ = -1)
        : min_facet_angle(min_facet_angle_),
          min_delaunay_radius(min_delaunay_radius_),
          min_center_separation(min_center_separation_)
        {}

        /** Default options. */
        static BoissonnatOudot const & defaults() { static BoissonnatOudot const options; return options; }
      };

#endif // THEA_ENABLE_CGAL

      /** Constructor. */
      Options(Bloomenthal const & bloomenthal_ = Bloomenthal::defaults()

#if THEA_ENABLE_CGAL
              , BoissonnatOudot const & boissonnat_oudot_ = BoissonnatOudot::defaults()
#endif // THEA_ENABLE_CGAL

      )
      : bloomenthal(bloomenthal_)

#if THEA_ENABLE_CGAL
      , boissonnat_oudot(boissonnat_oudot_)
#endif // THEA_ENABLE_CGAL

      {}

      /** Default options. */
      static Options const & defaults() { static Options const options; return options; }

      Bloomenthal bloomenthal;           ///< %Options for meshing via Bloomenthal [1994].

#if THEA_ENABLE_CGAL
      BoissonnatOudot boissonnat_oudot;  ///< %Options for meshing via Boissonnat and Oudot [2005].
#endif // THEA_ENABLE_CGAL

    }; // struct Options

    /**
     * Polygonize the zero level set of a 3D function to a mesh, using the method of Bloomenthal [1994].
     *
     *   Jules Bloomenthal, "An implicit surface polygonizer", Graphics Gems IV (P. Heckbert, ed.), Academic Press, New York,
     *   1994.
     *
     * @param surface_functor The function whose zero level set defines the surface.
     * @param bounding_ball Bounds the surface.
     * @param pt_near_surface A point on or near the zero level set.
     * @param options %Options controlling mesh generation.
     * @param result The output mesh will be stored here (any prior data will <b>not</b> be removed from the mesh).
     */
    template <typename FunctorT, typename MeshT>
    static void meshBloomenthal(FunctorT * surface_functor, Ball3 const & bounding_ball, Vector3 const & pt_near_surface,
                                Options::Bloomenthal const & options, MeshT & result)
    {
      BloomenthalEval<FunctorT> func(surface_functor);
      float size = (float)(options.cell_size < 0 ? bounding_ball.getRadius() / 10.0 : options.cell_size);
      int bounds = options.max_search_steps < 0 ? 10 : options.max_search_steps;
      BloomenthalPolygonizer::Polygonizer polygonizer(&func, size, bounds);
      polygonizer.march(options.tetrahedralize_cubes,
                        (float)pt_near_surface.x(), (float)pt_near_surface.y(), (float)pt_near_surface.z());

      THEA_LOG << "ImplicitSurfaceMesher: " << polygonizer.no_triangles() << " triangles generated via Bloomenthal";

      exportMesh(polygonizer, result);
    }

#if THEA_ENABLE_CGAL

    /**
     * Polygonize the zero level set of a 3D function to a mesh, using the method of Boissonnat and Oudot [2005] as implemented
     * by the CGAL library (http://www.cgal.org/Manual/latest/doc_html/cgal_manual/Mesh_3/Chapter_main.html)
     *
     *   Jean-Daniel Boissonnat and Steve Oudot, "Provably good sampling and meshing of surfaces", Graphical Models, 67:405-451,
     *   2005.
     *
     * @param surface_functor The function whose zero level set defines the surface.
     * @param bounding_ball Bounds the surface. The functor <em>must</em> evaluate to a negative value at its center.
     * @param options %Options controlling mesh generation.
     * @param result The output mesh will be stored here (any prior data will <b>not</b> be removed from the mesh).
     */
    template <typename FunctorT, typename MeshT>
    static void meshBoissonnatOudot(FunctorT * surface_functor, Ball3 const & bounding_ball,
                                    Options::BoissonnatOudot const & options, MeshT & result)
    {
      typedef CGAL::Exact_predicates_inexact_constructions_kernel K;
      typedef K::FT                                               FT;
      typedef K::Point_3                                          Point_3;
      typedef K::Sphere_3                                         Sphere_3;
      typedef CGAL::Labeled_mesh_domain_3<K>                      Mesh_domain;
      typedef CGAL::Mesh_triangulation_3<Mesh_domain>::type       Tr;
      typedef CGAL::Mesh_complex_3_in_triangulation_3<Tr>         C3t3;
      typedef CGAL::Mesh_criteria_3<Tr>                           Mesh_criteria;

      CgalEval<FunctorT> func(surface_functor);  // CGAL-style wrapper to evaluate the function

      // Define the bounding sphere
      Vector3 const & center = bounding_ball.getCenter();
      Real radius = bounding_ball.getRadius();
      Sphere_3 bounding_sphere(Point_3(static_cast<FT>(center.x()), static_cast<FT>(center.y()), static_cast<FT>(center.z())),
                                static_cast<FT>(radius * radius));

      // Create domain from implicit function
      Mesh_domain domain = Mesh_domain::create_implicit_mesh_domain(func, bounding_sphere);

      // Define meshing criteria
      namespace params = CGAL::parameters;
      Mesh_criteria criteria(
          params::facet_angle(static_cast<FT>(options.min_facet_angle       < 0 ? 30  : Math::radiansToDegrees(options.min_facet_angle))),
          params::facet_size(static_cast<FT>(options.min_delaunay_radius   < 0 ? 0.1 : options.min_delaunay_radius)),
          params::facet_distance(static_cast<FT>(options.min_center_separation < 0 ? 0.1 : options.min_center_separation)),
          params::cell_radius_edge_ratio(2.0));

      // Generate mesh
      C3t3 c3t3 = CGAL::make_mesh_3<C3t3>(domain, criteria, params::no_perturb(), params::no_exude());

      THEA_LOG << "ImplicitSurfaceMesher: " << c3t3.number_of_facets_in_complex() << " faces generated via Boissonnat-Oudot";

      exportMesh(c3t3, result);
    }

#endif // THEA_ENABLE_CGAL

  private:
    /** Export from Bloomenthal polygonizer to another mesh type. */
    template <typename MeshT> static void exportMesh(BloomenthalPolygonizer::Polygonizer & polygonizer, MeshT & dst)
    {
      typedef Graphics::IncrementalMeshBuilder<MeshT> Builder;
      Builder builder(&dst);

      Array<typename Builder::VertexHandle> vertices;

      builder.begin();
        for (size_t i = 0; i < polygonizer.no_vertices(); ++i)
        {
          BloomenthalPolygonizer::VERTEX const & bp_p = polygonizer.get_vertex(i);
          BloomenthalPolygonizer::NORMAL const & bp_n = polygonizer.get_normal(i);
          Vector3 p(bp_p.x, bp_p.y, bp_p.z), n(bp_n.x, bp_n.y, bp_n.z);

          typename Builder::VertexHandle vx = builder.addVertex(p, (intx)i, &n);
          if (!builder.isValidVertexHandle(vx))
            throw Error("ImplicitSurfaceMesher: Could not add vertex from Bloomenthal polygonizer to mesh");

          vertices.push_back(vx);
        }

        for (size_t i = 0; i < polygonizer.no_triangles(); ++i)
        {
          BloomenthalPolygonizer::TRIANGLE const & tri = polygonizer.get_triangle(i);
          alwaysAssertM(tri.v0 >= 0 && (size_t)tri.v0 < vertices.size()
                     && tri.v1 >= 0 && (size_t)tri.v1 < vertices.size()
                     && tri.v2 >= 0 && (size_t)tri.v2 < vertices.size(),
                        "ImplicitSurfaceMesher: Vertex index in triangle from Bloomenthal polygonizer is out of bounds");

          typename Builder::VertexHandle vx[3] = { vertices[(size_t)tri.v0],
                                                   vertices[(size_t)tri.v1],
                                                   vertices[(size_t)tri.v2] };
          if (!builder.isValidFaceHandle(builder.addFace(vx, vx + 3)))
            throw Error("ImplicitSurfaceMesher: Could not add triangle from Bloomenthal polygonizer to mesh");
        }
      builder.end();
    }

#if THEA_ENABLE_CGAL

    /** Export C3t3 to another mesh type. */
    template <typename C3t3T, typename MeshT> static void exportMesh(C3t3T & src, MeshT & dst)
    {
      typedef typename C3t3T::Triangulation Tr;
      typedef typename Tr::Point Point_3;

      typedef Graphics::IncrementalMeshBuilder<MeshT> Builder;
      Builder builder(&dst);

      typedef UnorderedMap<typename Tr::Vertex_handle, typename Builder::VertexHandle> VertexMap;
      VertexMap vmap;

      builder.begin();
        // Add vertices from the triangulation
        for (typename Tr::Finite_vertices_iterator vit = src.triangulation().finite_vertices_begin();
             vit != src.triangulation().finite_vertices_end(); ++vit)
        {
          Point_3 const & p = vit->point();
          typename Builder::VertexHandle vout = builder.addVertex(Vector3((Real)p.x(), (Real)p.y(), (Real)p.z()));
          if (!builder.isValidVertexHandle(vout))
            throw Error("ImplicitSurfaceMesher: Could not add vertex from Boissonnat-Oudot polygonizer to mesh");

          vmap[vit] = vout;
        }

        // Add facets from the complex
        typename Builder::VertexHandle face_vertices[3];
        for (typename C3t3T::Facets_in_complex_iterator fit = src.facets_in_complex_begin();
             fit != src.facets_in_complex_end(); ++fit)
        {
          typename Tr::Cell_handle cell = fit->first;
          int facet_index = fit->second;

          // Get the three vertices of the facet (the vertex opposite to facet_index is excluded)
          int vertex_indices[3];
          int j = 0;
          for (int i = 0; i < 4; ++i)
          {
            if (i != facet_index)
              vertex_indices[j++] = i;
          }

          for (int i = 0; i < 3; ++i)
          {
            typename Tr::Vertex_handle vh = cell->vertex(vertex_indices[i]);
            typename VertexMap::const_iterator existing = vmap.find(vh);
            if (existing == vmap.end())
              throw Error("ImplicitSurfaceMesher: Mesh created by Boissonnat-Oudot polygonizer refers to unmapped vertex");

            face_vertices[i] = existing->second;
          }

          if (!builder.isValidFaceHandle(builder.addFace(face_vertices, face_vertices + 3)))
            throw Error("ImplicitSurfaceMesher: Could not add triangle from Boissonnat-Oudot polygonizer to mesh");
        }
      builder.end();
    }

#endif // THEA_ENABLE_CGAL

}; // class ImplicitSurfaceMesher

} // namespace Algorithms
} // namespace Thea

#endif
