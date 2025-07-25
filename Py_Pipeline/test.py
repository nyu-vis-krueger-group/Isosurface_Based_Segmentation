import pyvista as pv

mesh = pv.read("0.off")

mask = mesh.voxelize_binary_mask(
    background_value=0,
    foreground_value=1,
    dimensions=(1024, 1024, 194),                      
    reference_volume=None            
)

from vtkmodules.vtkIOXML import vtkXMLImageDataWriter


writer = vtkXMLImageDataWriter()
writer.SetFileName("mask0.vti")
writer.SetInputData(mask)                     
writer.SetCompressorTypeToZLib()              
writer.Write()