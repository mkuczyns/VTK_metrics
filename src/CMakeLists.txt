cmake_minimum_required(VERSION 2.8)

PROJECT(vtkMetrics)

find_package(VTK REQUIRED)
include(${VTK_USE_FILE})

add_executable(vtkMetrics MACOSX_BUNDLE vtkMetrics.cxx helperFunctions.cxx interactorStyler.cxx)

if(VTK_LIBRARIES)
  target_link_libraries(vtkMetrics ${VTK_LIBRARIES})
else()
  target_link_libraries(vtkMetrics vtkHybrid vtkWidgets)
endif()
