#define NOMINMAX

#include <Eigen/Core>

#include <igl/readOFF.h>
#include <igl/copyleft/cgal/mesh_boolean.h>

#include <iostream>
#include <vector>
#include <unordered_map>
#include <string>

int main(int argc, char* argv[]) {
	if (argc < 3) {
		std::cerr << "Please provide at least 2 input mesh files." << std::endl;
		return -1;
	}

	std::cout << "here" << std::endl;

	// index to filename mapping
	std::unordered_map<int, std::string> index_to_name;

	// read input meshes
	int N = argc - 1;

	std::cout << "Reading input meshes." << std::endl;

	std::vector<std::pair<Eigen::MatrixXd, Eigen::MatrixXi>> meshes(N);
	for (size_t i = 1; i <= static_cast<size_t>(N); ++i) {
		index_to_name[i - 1] = argv[i];
		std::cout << "Reading mesh: " << argv[i] << std::endl;
		igl::readOFF(argv[i], meshes[i - 1].first, meshes[i - 1].second);
		std::cout << "number of vertices: " << meshes[i - 1].first.rows() << ", number of faces: " << meshes[i - 1].second.rows() << std::endl;
	}

	std::cout << "Done." << std::endl;


	// mesh unions, intersections and IOU scores
	std::vector<std::vector<double>> iou_scores(N, std::vector<double>(N));

	std::cout << "Calculating iou scores." << std::endl;

	for (size_t i = 0; i < N; ++i)
		for (size_t j = i + 1; j < N; ++j) {
			Eigen::MatrixXd tempV1, tempV2;
			Eigen::MatrixXi tempF1, tempF2;

			std::cout << "number of vertices: " << meshes[i].first.rows() << ", number of faces: " << meshes[i].second.rows() << std::endl;
			std::cout << "number of vertices: " << meshes[j].first.rows() << ", number of faces: " << meshes[j].second.rows() << std::endl;


			std::cout << "Calculating intersections." << std::endl;

			igl::copyleft::cgal::mesh_boolean(
				meshes[i].first, meshes[i].second,
				meshes[j].first, meshes[j].second,
				igl::MESH_BOOLEAN_TYPE_INTERSECT,
				tempV1, tempF1
			);

			std::cout << "Done." << std::endl;
			int intersection_faces = tempF1.rows();

			std::cout << "Calculating union." << std::endl;

			igl::copyleft::cgal::mesh_boolean(
				meshes[i].first, meshes[i].second,
				meshes[j].first, meshes[j].second,
				igl::MESH_BOOLEAN_TYPE_UNION,
				tempV2, tempF2
			);

			int union_faces = tempF2.rows();

			iou_scores[i][j] = static_cast<double>(intersection_faces) / static_cast<double>(union_faces);
		}

	std::cout << "Done." << std::endl;


	std::cout << "IOU SCORES:" << std::endl;

	for (size_t i = 0; i < N; ++i) {
		for (size_t j = i + 1; j < N; ++j)
			std::cout << iou_scores[i][j] << " ";
		std::cout << std::endl;
	}

	return 0;
}