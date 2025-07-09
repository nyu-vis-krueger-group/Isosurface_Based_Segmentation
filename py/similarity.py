import pymeshlab as ml
import argparse

ms = ml.MeshSet()

def load_calculate_iou(mesh1, mesh2):
	ms.load_new_mesh(mesh1)
	ms.load_new_mesh(mesh2)

	ms.generate_boolean_intersection(first_mesh=0, second_mesh=1)
	num_faces_intersection = ms.current_mesh().face_number()

	ms.generate_boolean_union(first_mesh=0, second_mesh=1)
	num_faces_union = ms.current_mesh().face_number()
	
	print(num_faces_intersection/num_faces_union)

def load_calculate_similarity(mesh1, mesh2):
	ms.load_new_mesh(mesh1)
	num_faces_mesh1 = ms.current_mesh().face_number()

	ms.load_new_mesh(mesh2)

	ms.generate_boolean_intersection(first_mesh=0, second_mesh=1)
	num_faces_intersection = ms.current_mesh().face_number()
	
	print(num_faces_intersection/num_faces_mesh1)

def load_calculate_similarity_based_on_volume(mesh1, mesh2):
	ms.load_new_mesh(mesh1)
	measures = ms.apply_filter('get_geometric_measures')
	v1 = measures['mesh_volume']

	ms.load_new_mesh(mesh2)
	measures = ms.apply_filter('get_geometric_measures')
	v2 = measures['mesh_volume']

	ms.generate_boolean_intersection(first_mesh=0, second_mesh=1)
	measures = ms.apply_filter('get_geometric_measures')
	vi = measures['mesh_volume']

	print(v1, v2, v1/vi, v2/vi)




if __name__=="__main__":
	parser = argparse.ArgumentParser("Similarity Calculator")
	parser.add_argument("-mesh1", "--mesh1", help="first mesh", required=True)
	parser.add_argument("-mesh2", "--mesh2", help="second mesh", required=True)
	args=parser.parse_args()

	load_calculate_similarity_based_on_volume(args.mesh1, args.mesh2)




