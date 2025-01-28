cmake_minimum_required(VERSION 3.9...3.26)
project (3d-demo2)

find_package (32BLIT CONFIG REQUIRED PATHS ../..)
find_package (THR3E CONFIG REQUIRED PATHS ../tile-based-high-resolution-3d-engine)

blit_executable (3d-demo2 3d-demo.cpp)
target_compile_definitions (3d-demo2 PRIVATE THR3E_MAX_TEXTURES=8 THR3E_MAX_OUTPUT_VERTICES=3000)
target_link_libraries (${PROJECT_NAME} THR3E)

blit_metadata (3d-demo2 metadata.yml)
blit_assets_yaml (3d-demo2 assets.yml)
