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
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/LindstromTurk_cost.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/LindstromTurk_placement.h>
#include <CGAL/Surface_mesh_simplification/Policies/Edge_collapse/Bounded_normal_change_filter.h>

#include <CGAL/boost/graph/Face_filtered_graph.h>
#include <CGAL/boost/graph/IO/PLY.h>
#include <boost/property_map/property_map.hpp>
#include <boost/lexical_cast.hpp>

#include <map>
#include <iostream>
#include <iterator>
#include <string>
#include <tuple>
#include <vector>
#include <cassert>

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

struct Dummy_placement {

    template <typename Profile>
    std::optional<typename Profile::Point> operator()(const Profile&) const
    {
        return std::nullopt;
    }

    template <typename Profile>
    std::optional<typename Profile::Point> operator()(const Profile&, const std::optional<typename Profile::Point>& op) const
    {
        return op;
    }


};

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

#define INITIAL_FILTER_FACE_COUNT 100
#define LARGEST_COMPONENT_NUMBER 10
#define RELATIVE_ALPHA 300.0
#define RELATIVE_OFFSET 750.0
#define MESH_SIMPLIFICATION_STOP_RATIO 0.1
#define FINAL_FILTER_FACE_COUNT 15
#define FINAL_FILTER_SIZE_RATIO 0.05

/*
* REGULARIZE BEFORE FINAL FILTERING
*/


int main(int argc, char* argv[])
{
    assert(argc>1 && "Pass at least the filename, and optionally a config file.");

    const std::string filename = argv[1];

    std::cout << "READING" << std::endl;
    std::cout << "Reading PLY input file " << filename << "." << std::endl;
    CGAL::Real_timer t;
    Mesh mesh;

    t.start();
    /*if (!PMP::IO::read_polygon_mesh(filename, mesh))
    {
        std::cerr << "Invalid input." << std::endl;
        return 1;
    }*/
    if (!CGAL::IO::read_PLY(filename, mesh,
        CGAL::parameters::use_binary_mode(true)            // binary PLY :contentReference[oaicite:2]{index=2}
        .vertex_point_map(get(CGAL::vertex_point, mesh))
        .vertex_index_map(get(boost::vertex_index, mesh))
        .face_index_map(get(boost::face_index, mesh))
        .verbose(false)))
    {
        std::cerr << "Invalid PLY file\n";
        return 1;
    }
    t.stop();

    std::cout << filename << "read in " << t.time() << "seconds" << std::endl;
    t.reset();

    // initial filtering
    std::cout << std::endl;
    std::cout << "INITIAL FILTERING" << std::endl;
    std::cout << "Filtering components of face count < " << INITIAL_FILTER_FACE_COUNT << "." << std::endl;
    t.start();
    PMP::keep_large_connected_components(mesh, INITIAL_FILTER_FACE_COUNT);
    t.stop();
    std::cout << "Done in " << t.time() << "seconds." << std::endl;
    t.reset();


    // copying mesh
    std::cout << std::endl;
    std::cout << "COPYING" << std::endl;
    std::cout << "Copying filtered mesh. "<< std::endl;
    t.start();
    Mesh mesh_copy(mesh);
    t.stop();
    std::cout << "Done in " << t.time() << "seconds." << std::endl;
    t.reset();


    // finding representative diagonal and size
    std::cout << std::endl;
    std::cout << "LARGE COMPONENT DIMENSION CALCULATION" << std::endl;
   
    std::cout << "Keeping "<<LARGEST_COMPONENT_NUMBER<<" components in the copied mesh." << std::endl;
    t.start();
    PMP::keep_largest_connected_components(mesh_copy, LARGEST_COMPONENT_NUMBER);
    t.stop();
    std::cout << "Done in " << t.time() << "seconds." << std::endl;
    t.reset();


    std::cout << "Iterating over " << LARGEST_COMPONENT_NUMBER << " components in the copied mesh, calculating their bounding box diags." << std::endl;
    t.start();

    FCCmap fccmap = mesh_copy.add_property_map<face_descriptor, faces_size_type>("f:CC").first;
    faces_size_type num = PMP::connected_components(mesh_copy, fccmap);

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

    t.stop();
    std::cout << "Done in " << t.time() << "seconds." << std::endl;
    t.reset();

    std::cout << std::endl;
    std::cout << "Average diagonal in " << LARGEST_COMPONENT_NUMBER << " components: " << avg_diagonal << std::endl;
    std::cout << "Max diagonal in " << LARGEST_COMPONENT_NUMBER << " components: " << max_diagonal << std::endl;
    std::cout << "Average num_faces in " << LARGEST_COMPONENT_NUMBER << " components: " << avg_num_faces << std::endl;
    std::cout << "Max num_faces in " << LARGEST_COMPONENT_NUMBER << " components: " << max_num_faces << std::endl;


    // hole filling
    std::cout << std::endl;
    std::cout << "HOLE FILLING" << std::endl;

    double max_hole_diam = max_diagonal;
    int max_num_hole_edges = max_num_faces;

    std::cout << "Max hole diameter: " << max_diagonal << std::endl;
    std::cout << "Max number of hole edges: " << max_num_faces << std::endl;

    t.start();

    unsigned int nb_holes = 0;
    std::vector<halfedge_descriptor> border_cycles;

    PMP::extract_boundary_cycles(mesh, std::back_inserter(border_cycles));

    for (halfedge_descriptor h : border_cycles)
    {
        if (max_hole_diam > 0 && max_num_hole_edges > 0 &&
            !is_small_hole(h, mesh, max_hole_diam, max_num_hole_edges))
            continue;

        PMP::triangulate_hole(mesh, h);

        ++nb_holes;
    }
    t.stop();

    std::cout << nb_holes << " holes have been filled." << std::endl;
    std::cout << "Done in " << t.time() << " seconds." << std::endl;
    t.reset();

    // alpha wrapping
    std::cout << std::endl;
    std::cout << "ALPHA WRAPPING" << std::endl;

    const double alpha = avg_diagonal / RELATIVE_ALPHA;
    const double offset = avg_diagonal / RELATIVE_OFFSET;

    std::cout << "params -> alpha: " << alpha << ", offset: " << offset << std::endl;    

    t.start();

    Mesh wrap;
    CGAL::alpha_wrap_3(mesh, alpha, offset, wrap);

    t.stop();
    std::cout << "Done in " << t.time() << " seconds." << std::endl;
    t.reset();


    // simplification
    std::cout << std::endl;
    std::cout << "MESH SIMPLIFICATION" << std::endl;
    std::cout << "Stop ratio: " << MESH_SIMPLIFICATION_STOP_RATIO << std::endl;


    t.start();
    SMS::Edge_count_ratio_stop_predicate<Mesh> stop(MESH_SIMPLIFICATION_STOP_RATIO);
    typedef SMS::LindstromTurk_placement<Mesh> Placement;
    SMS::Bounded_normal_change_filter<> filter;
    int r = SMS::edge_collapse(wrap, stop,
        CGAL::parameters::get_cost(SMS::LindstromTurk_cost<Mesh>())
        .filter(filter)
        .get_placement(Placement()));
    t.stop();
    std::cout << r << " edgegs removed." << std::endl;
    std::cout << "Done in " << t.time() << " seconds." << std::endl;
    t.reset();


    // final filtering
    // copying mesh
    std::cout << std::endl;
    std::cout << "COPYING" << std::endl;
    std::cout << "Copying filtered mesh. " << std::endl;
    t.start();
    Mesh wrap_copy(wrap);
    t.stop();
    std::cout << "Done in " << t.time() << "seconds." << std::endl;
    t.reset();


    // finding representative diagonal and size
    std::cout << std::endl;
    std::cout << "LARGE COMPONENT DIMENSION CALCULATION" << std::endl;

    std::cout << "Keeping " << LARGEST_COMPONENT_NUMBER << " components in the copied mesh." << std::endl;
    t.start();
    PMP::keep_largest_connected_components(wrap_copy, LARGEST_COMPONENT_NUMBER);
    t.stop();
    std::cout << "Done in " << t.time() << "seconds." << std::endl;
    t.reset();


    std::cout << "Iterating over " << LARGEST_COMPONENT_NUMBER << " components in the copied mesh, calculating their bounding box diags." << std::endl;
    t.start();

    FCCmap fccmap1 = wrap_copy.add_property_map<face_descriptor, faces_size_type>("f:CC").first;
    faces_size_type num1 = PMP::connected_components(wrap_copy, fccmap1);

    std::vector<Mesh> meshes1(num1);
    avg_num_faces = 0;

    for (int i = 0; i < num1; ++i)
    {
        Filtered_graph ffg(wrap_copy, i, fccmap1);
        CGAL::copy_face_graph(ffg, meshes1[i]);

        int n = num_faces(meshes1[i]);
        avg_num_faces += n;
    }

    avg_num_faces /= num1;

    t.stop();
    std::cout << "Done in " << t.time() << "seconds." << std::endl;
    t.reset();

    std::cout << std::endl;
    std::cout << "FINAL FILTERING" << std::endl;
    std::cout << "Filtering components of face count < " << (int)(avg_num_faces * FINAL_FILTER_SIZE_RATIO) << "." << std::endl;
    //std::cout << "Filtering components of face count < " << FINAL_FILTER_FACE_COUNT << "." << std::endl;
    t.start();
    PMP::keep_large_connected_components(wrap, (int)(avg_num_faces*FINAL_FILTER_SIZE_RATIO));
    //PMP::keep_large_connected_components(wrap, (int)(FINAL_FILTER_FACE_COUNT));
    t.stop();
    std::cout << "Done in " << t.time() << " seconds." << std::endl;
    t.reset();
    
    // Save the result
    std::cout << std::endl;
    std::cout << "WRITING OUTPUT MESH" << std::endl;
    t.start();
    const std::string output_name = filename+".off";
    std::cout << "Writing to " << output_name << std::endl;
    CGAL::IO::write_polygon_mesh(output_name, wrap, CGAL::parameters::stream_precision(17));
    t.stop();
    std::cout << "Done in " << t.time() << " seconds." << std::endl;
    t.reset();


    return 0;
}