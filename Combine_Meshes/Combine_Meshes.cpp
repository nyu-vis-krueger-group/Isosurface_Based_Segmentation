#include <CGAL/Exact_predicates_inexact_constructions_kernel.h>
#include <CGAL/Surface_mesh.h>

#include <CGAL/Polygon_mesh_processing/IO/polygon_mesh_io.h>
#include <CGAL/Polygon_mesh_processing/bbox.h>
#include <CGAL/Polygon_mesh_processing/corefinement.h>
#include <CGAL/Polygon_mesh_processing/transform.h>
#include <CGAL/Polygon_mesh_processing/mer>

#include <CGAL/boost/graph/IO/PLY.h>
#include <CGAL/boost/graph/IO/OFF.h>

#include <iostream>
#include <string>
#include <vector>
#include <cassert>

typedef CGAL::Exact_predicates_inexact_constructions_kernel  Kernel;
typedef Kernel::Point_3                                      Point;
typedef CGAL::Surface_mesh<Point>                            Mesh;

using Vector_3 = typename Kernel::Vector_3;
using Affine_transformation_3 = typename Kernel::Aff_transformation_3;

namespace PMP = CGAL::Polygon_mesh_processing;


int main(int argc, char* argv[])
{
    assert(argc == 5 && "mesh1.off mesh2.off X-offset Y-offset");

    const std::vector<std::string> filenames = { argv[1], argv[2] };

    std::cout << "READING" << std::endl;

    std::vector<Mesh> inputMeshes(2);

    for (int i = 0; i < 2; ++i) {
        std::cout << "Reading OFF input file " << filenames[i] << "." << std::endl;

        if (!CGAL::IO::read_OFF(filenames[i], inputMeshes[i],
            CGAL::parameters::use_binary_mode(true)
            .vertex_point_map(get(CGAL::vertex_point, inputMeshes[i]))
            .vertex_index_map(get(boost::vertex_index, inputMeshes[i]))
            .face_index_map(get(boost::face_index, inputMeshes[i]))
            .verbose(false)))
        {
            std::cerr << "Invalid OFF file\n";
            return 1;
        }

        std::cout << filenames[i] << " read.\n" << std::endl;
    }

    // transformation (translation) of tile into correct position
    int x_off(std::stoi(argv[3])), y_off(std::stoi(argv[4]));

    CGAL::Bbox_3 bbox = CGAL::Polygon_mesh_processing::bbox(inputMeshes[0]);
    PMP::transform(Affine_transformation_3(CGAL::Translation(),
        Vector_3(x_off, y_off, 0)), inputMeshes[1]);

    // take mesh union to combine them
    Mesh out;
    bool valid_union = PMP::corefine_and_compute_union(inputMeshes[0], inputMeshes[1], out);

    if (valid_union)
    {
        std::cout << "Union was successfully computed\n";
        CGAL::IO::write_polygon_mesh("union.off", out, CGAL::parameters::stream_precision(17));
    }

    return 0;
}