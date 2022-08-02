# Edit following two lines to set component requirements (see docs)
set(COMPONENT_REQUIRES )
set(COMPONENT_PRIV_REQUIRES )

set(COMPONENT_SRCS "main.cpp" "Src/DefaultTask.cpp" "Src/TaskMonitor.cpp" "Src/Task1.cpp" "Src/Task2.cpp" "Src/Task3.cpp")
set(COMPONENT_ADD_INCLUDEDIRS "Inc")

register_component()
component_compile_options(-std=gnu++17) 