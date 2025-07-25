from cx_Freeze import setup, Executable

# Dependencies are automatically detected, but it might need
# fine tuning.
build_options = {'packages': [], 'excludes': []}

base = 'console'

executables = [
    Executable('similarity.py', base=base)
]

setup(name='Similarity_Index',
      version = '1.0',
      description = 'Num_Tris(intersection)/Num_tris(region)',
      options = {'build_exe': build_options},
      executables = executables)
