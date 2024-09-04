cmake_minimum_required(VERSION 3.5)

project(Voxel VERSION 0.1 LANGUAGES C)

set(CMAKE_C_STANDARD 17)
set(CMAKE_C_STANDARD_REQUIRED ON)
set(CMAKE_C_FLAGS "${CMAKE_C_FLAGS} -DNODEBUG")

if (LINUX)
    include_directories("/usr/include/glib-2.0")
    include_directories("/usr/lib64/glib-2.0/include/")
elseif (WIN32)
    include_directories("C:/Program Files/cairo/include/glib-2.0")
    include_directories("C:/Program Files/cairo/lib/glib-2.0/include")
    include_directories("C:/Program Files/glfw/include")
    include_directories("C:/VulkanSDK/1.3.290.0/Include")
    include_directories("C:/Program Files/cairo/include")
    include_directories("C:/Program Files/glew/include")
endif()

find_program(GLSLC NAMES glslc REQUIRED)

set(SPIRV_SOURCES
    src/shaders/glsl/450/3D.vert.glsl
    src/shaders/glsl/450/3D.frag.glsl
    src/shaders/glsl/450/2D.vert.glsl
    src/shaders/glsl/450/2D.frag.glsl
)

make_directory(${CMAKE_CURRENT_BINARY_DIR}/shaders/spir-v)
set(SPIRV_BIN)

foreach(SPIRV_SOURCE IN LISTS SPIRV_SOURCES)
    cmake_path(GET SPIRV_SOURCE FILENAME SPIRV_NAME)
    string(REGEX REPLACE "\\.[^.]*$" "" SPIRV_NAME ${SPIRV_NAME})
    string(CONCAT SPIRV_NAME  ${SPIRV_NAME} ".spv")
    set(SPIRV_OUT shaders/spir-v/${SPIRV_NAME})
    string(FIND ${SPIRV_NAME} "vert" SPIRV_NAME_VERT)
    if(SPIRV_NAME_VERT GREATER_EQUAL 0)
        set(SPIRV_STAGE "vert")
    else()
        set(SPIRV_STAGE "frag")
    endif()
    add_custom_command(
        OUTPUT ${SPIRV_OUT}
        WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
        COMMAND ${GLSLC}
        ARGS
            -fshader-stage=${SPIRV_STAGE}
            -c ${SPIRV_SOURCE}
            -o ${CMAKE_CURRENT_BINARY_DIR}/${SPIRV_OUT}
        VERBATIM
        MAIN_DEPENDENCY ${SPIRV_SOURCE}
    )
    list(APPEND SPIRV_BIN ${CMAKE_CURRENT_BINARY_DIR}/${SPIRV_OUT})
endforeach()

add_custom_target(
    Voxel-SPIRV
    DEPENDS ${SPIRV_BIN}
)

find_program(GLIB_COMPILE_RESOURCES NAMES glib-compile-resources REQUIRED)
set(GRESOURCE_C   src/resources.c)
set(GRESOURCE_XML resources.xml)

add_custom_command(
    OUTPUT ${GRESOURCE_C}
    WORKING_DIRECTORY ${CMAKE_CURRENT_SOURCE_DIR}
    COMMAND ${GLIB_COMPILE_RESOURCES}
    ARGS
        --target=${CMAKE_CURRENT_BINARY_DIR}/${GRESOURCE_C}
        --generate-source
        ${GRESOURCE_XML}
    VERBATIM
    MAIN_DEPENDENCY ${GRESOURCE_XML}
    DEPENDS
        src/shaders/glsl/120/3D.vert.glsl
        src/shaders/glsl/120/3D.frag.glsl
        src/shaders/glsl/120/2D.vert.glsl
        src/shaders/glsl/120/2D.frag.glsl
        src/shaders/glsl/300/3D.vert.glsl
        src/shaders/glsl/300/3D.frag.glsl
        src/shaders/glsl/300/2D.vert.glsl
        src/shaders/glsl/300/2D.frag.glsl
        ${SPIRV_BIN}
        img/color-dropper-natural.png
        img/color-dropper-selected.png
        img/dialog.png
        img/eraser-natural.png
        img/eraser-selected.png
        img/move-natural.png
        img/move-selected.png
        img/pencil-natural.png
        img/pencil-selected.png
        img/select-natural.png
        img/select-selected.png
        img/stamp-natural.png
        img/stamp-selected.png
)

add_custom_target(
    Voxel-Resource
    DEPENDS ${CMAKE_CURRENT_BINARY_DIR}/${GRESOURCE_C}
)

make_directory(${CMAKE_CURRENT_BINARY_DIR}/src)

set(PROJECT_SOURCES
    CMakeLists.txt
    src/box.h
    src/window.h
    src/matrix.c
    src/linked_list.h
    src/ground.h
    src/application.c
    src/chunk.c
    src/block.h
    src/chunk_dao.c
    src/picker_panel.c
    src/vulkan_util.h
    src/chunk.h
    src/camera.h
    src/voxel.c
    src/global.c
    src/renderer.c
    src/block.c
    src/bp_tree.h
    src/application.h
    src/bp_tree.c
    src/shader.c
    src/mesh.h
    src/world.h
    src/heap.c
    src/box.c
    src/picker_panel.h
    src/undo_stack.c
    src/heap.h
    src/fps_panel.h
    src/internal/window.h
    src/internal/linked_list.h
    src/internal/block.h
    src/internal/chunk.h
    src/internal/bp_tree.h
    src/internal/mesh.h
    src/internal/world.h
    src/internal/picker_panel.h
    src/internal/heap.h
    src/internal/fps_panel.h
    src/internal/shader.h
    src/internal/panel.h
    src/internal/picker.h
    src/internal/voxel.h
    src/internal/renderer.h
    src/chunk_dao.h
    src/world.c
    src/undo_stack.h
    src/shader.h
    src/matrix.h
    src/mesh.c
    src/camera.c
    src/panel.h
    src/linked_list.c
    src/fps_panel.c
    src/picker.h
    src/ground.c
    src/picker.c
    src/vulkan_util.c
    src/commands/world_clear_region_command.c
    src/commands/world_cut_chunk_command.c
    src/commands/world_copy_chunk_command.c
    src/commands/world_clear_region_command.h
    src/commands/world_set_region_command.c
    src/commands/world_set_region_command.h
    src/commands/world_copy_chunk_command.h
    src/commands/world_cut_chunk_command.h
    src/voxel.h
    src/panel.c
    src/global.h
    src/window.c
    src/renderer.h
    src/main.c
)

add_executable(Voxel
    ${PROJECT_SOURCES}
    ${CMAKE_CURRENT_BINARY_DIR}/${GRESOURCE_C}
)

add_dependencies(Voxel-Resource Voxel-SPIRV)
add_dependencies(Voxel Voxel-Resource)

if (LINUX)
    target_link_libraries(Voxel PRIVATE vulkan GL glfw cairo gio-2.0 gobject-2.0 glib-2.0 m)
elseif (WIN32)
    target_link_libraries(Voxel PRIVATE
        "C:/Program Files/glew/lib/glew32.lib"
        "C:/Program Files (x86)/Windows Kits/10/Lib/10.0.26100.0/um/x64/GlU32.Lib"
        "C:/Program Files (x86)/Windows Kits/10/Lib/10.0.26100.0/um/x64/OpenGL32.Lib"
        "C:/Program Files/glfw/lib/glfw3dll.lib"
        "C:/Program Files/cairo/lib/gio-2.0.lib"
        "C:/Program Files/cairo/lib/gobject-2.0.lib"
        "C:/Program Files/cairo/lib/glib-2.0.lib"
        "C:/Program Files/cairo/lib/cairo.lib"
    )
endif()