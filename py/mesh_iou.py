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

if __name__=="__main__":
	parser = argparse.ArgumentParser("IOU Calculator")
	parser.add_argument("-mesh1", "--mesh1", help="first mesh", required=True)
	parser.add_argument("-mesh2", "--mesh2", help="second mesh", required=True)
	args=parser.parse_args()

	load_calculate_iou(args.mesh1, args.mesh2)




