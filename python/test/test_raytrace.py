import VkInline as vki
import numpy as np
import glm

positions = np.array([ [0.0, -0.5, 0.5], [0.5, 0.5, 0.5], [-0.5, 0.5, 0.5] ], dtype = np.float32)
gpuPos = vki.device_vector_from_numpy(positions)

indices = np.array([0, 1, 2], dtype = np.uint32)
gpuIndices = vki.device_vector_from_numpy(indices)

# blas = vki.BaseLevelAS(gpuPos=gpuPos)
blas = vki.BaseLevelAS(gpuInd=gpuIndices, gpuPos=gpuPos)
trans =  glm.identity(glm.mat4)
tlas = vki.TopLevelAS([[(blas,trans)]])
