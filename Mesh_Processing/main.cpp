#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>

#include <CGAL/alpha_wrap_3.h>

#include <CGAL/Polygon_mesh_processing/triangulate_hole.h>
#include <CGAL/Polygon_mesh_processing/border.h>
#include <CGAL/Polygon_mesh_processing/connected_components.h>
#include <CGAL/Polygon_mesh_processing/IO/polygon_mesh_io.h>
#include <CGAL/Polygon_mesh_processing/bbox.h>

#include <CGAL/Surface_mesh_simplification/edge_collapse.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Edge_count_ratio_stop_predicate.h>

#include <CGAL/boost/graph/Face_filtered_graph.h>
#include <boost/property_map/property_map.hpp>
#include <boost/lexical_cast.hpp>

#include <map>
#include <iostream>
#include <iterator>
#include <string>
#include <tuple>
#include <vector>

typedef CGAL::Exact_predicates_inexact_constructions_kernel  Kernel;
typedef Kernel::Point_3                                      Point;

typedef CGAL::Surface_mesh<Point>                            Mesh;
typedef boost::graph_traits<Mesh>::vertex_descriptor        vertex_descriptor;
typedef boost::graph_traits<Mesh>::halfedge_descriptor      halfedge_descriptor;
typedef boost::graph_traits<Mesh>::face_descriptor          face_descriptor;
typedef boost::graph_traits<Mesh>::faces_size_type           faces_size_type;

typedef Mesh::Property_map<face_descriptor, faces_size_type> FCCmap;
typedef CGAL::Face_filtered_graph<Mesh>                      Filtered_graph;

namespace PMP = CGAL::Polygon_mesh_processing;
namespace SMS = CGAL::Surface_mesh_simplification;

static bool is_small_hole(halfedge_descriptor h, Mesh& mesh,
    double max_hole_diam, int max_num_hole_edges)
{
    int num_hole_edges = 0;
    CGAL::Bbox_3 hole_bbox;
    for (halfedge_descriptor hc : CGAL::halfedges_around_face(h, mesh))
    {
        const Point& p = mesh.point(target(hc, mesh));

        hole_bbox += p.bbox();
        ++num_hole_edges;

        // Exit early, to avoid unnecessary traversal of large holes
        if (num_hole_edges > max_num_hole_edges) return false;
        if (hole_bbox.xmax() - hole_bbox.xmin() > max_hole_diam) return false;
        if (hole_bbox.ymax() - hole_bbox.ymin() > max_hole_diam) return false;
        if (hole_bbox.zmax() - hole_bbox.zmin() > max_hole_diam) return false;
    }

    return true;
}

int main(int argc, char* argv[])
{
    const std::string filename = (argc > 1) ? argv[1] : CGAL::data_file_path("channel_42_res_0_iso_788.obj");

    Mesh mesh;
    if (!PMP::IO::read_polygon_mesh(filename, mesh))
    {
        std::cerr << "Invalid input." << std::endl;
        return 1;
    }

    PMP::keep_large_connected_components(mesh, 100);

    Mesh mesh_copy(mesh);

    // keep 10 largest connected components
    PMP::keep_largest_connected_components(mesh_copy, 10);

    // iterate over the 10 largest 
    FCCmap fccmap = mesh_copy.add_property_map<face_descriptor, faces_size_type>("f:CC").first;
    faces_size_type num = PMP::connected_components(mesh_copy, fccmap);
    std::cerr << "- The graph has " << num << " connected components (face connectivity)" << std::endl;

    std::vector<Mesh> meshes(num);
    double avg_diagonal = 0.0;
    double max_diagonal = 0.0;
    int avg_num_faces = 0;
    int max_num_faces = 0;

    for (int i = 0; i < num; ++i)
    {
        Filtered_graph ffg(mesh_copy, i, fccmap);
        CGAL::copy_face_graph(ffg, meshes[i]);

        CGAL::Bbox_3 bbox = CGAL::Polygon_mesh_processing::bbox(mesh);
        const double diag_length = std::sqrt(CGAL::square(bbox.xmax() - bbox.xmin()) +
            CGAL::square(bbox.ymax() - bbox.ymin()) +
            CGAL::square(bbox.zmax() - bbox.zmin()));

        int n = num_faces(meshes[i]);
        avg_diagonal += diag_length;
        avg_num_faces += n;
        max_diagonal = std::max(diag_length, max_diagonal);
        max_num_faces = std::max(n, max_num_faces);
    }

    avg_diagonal /= num;
    avg_num_faces /= num;

    std::cout << "Average diag length in 10 largest connected components: " << avg_diagonal << std::endl;

    const double relative_alpha = 300.0;
    const double relative_offset = 900.0;
    const double alpha = avg_diagonal / relative_alpha;
    const double offset = avg_diagonal / relative_offset;
    std::cout << "alpha: " << alpha << ", offset: " << offset << std::endl;

    // hole filling
    double max_hole_diam = max_diagonal;
    int max_num_hole_edges = max_num_faces;

    unsigned int nb_holes = 0;
    std::vector<halfedge_descriptor> border_cycles;

    // collect one halfedge per boundary cycle
    PMP::extract_boundary_cycles(mesh, std::back_inserter(border_cycles));
    std::cout << "borders: " << border_cycles.size() << std::endl;

    for (halfedge_descriptor h : border_cycles)
    {
        if (max_hole_diam > 0 && max_num_hole_edges > 0 &&
            !is_small_hole(h, mesh, max_hole_diam, max_num_hole_edges))
            continue;

        PMP::triangulate_hole(mesh, h);

        ++nb_holes;
    }

    std::cout << std::endl;
    std::cout << nb_holes << " holes have been filled" << std::endl;

    // Construct the wrap
    CGAL::Real_timer t;
    t.start();

    Mesh wrap;
    CGAL::alpha_wrap_3(mesh, alpha, offset, wrap);

    t.stop();
    std::cout << "Result: " << num_vertices(wrap) << " vertices, " << num_faces(wrap) << " faces" << std::endl;
    std::cout << "Took " << t.time() << " s." << std::endl;

    // simplification
    double stop_ratio = 0.1;
    SMS::Edge_count_ratio_stop_predicate<Mesh> stop(stop_ratio);
    int r = SMS::edge_collapse(wrap, stop);
    std::cout << r << " edgegs removed." << std::endl;
    
    // Save the result
    const std::string output_name = "channel_42_res_0_iso_788_a300_o900_hole3.off";
    std::cout << "Writing to " << output_name << std::endl;
    CGAL::IO::write_polygon_mesh(output_name, wrap, CGAL::parameters::stream_precision(17));


    return 0;
}