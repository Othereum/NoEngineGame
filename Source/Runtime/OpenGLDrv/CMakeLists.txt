# Source/Runtime/OpenGLDrv

add_module(OpenGLDrv)
oe_link_sdl2(OpenGLDrv)

target_link_libraries(${PROJECT_NAME}-OpenGLDrv
  PUBLIC
	${PROJECT_NAME}-RHI

  PRIVATE
	GLEW::glew
)

target_compile_definitions(${PROJECT_NAME}-OpenGLDrv
  PRIVATE
    OE_OPENGL_VERSION_MAJOR=${OE_OPENGL_VERSION_MAJOR}
    OE_OPENGL_VERSION_MINOR=${OE_OPENGL_VERSION_MINOR}
)
