set(SEP_NAME SREES_2026_Cavcic_StateEstimatorPolar)				

file(GLOB SEP_SOURCES  ${CMAKE_CURRENT_LIST_DIR}/src/*.cpp)
file(GLOB SEP_INCS  ${CMAKE_CURRENT_LIST_DIR}/src/*.h)
set(SEP_PLIST  ${CMAKE_CURRENT_LIST_DIR}/src/Info.plist)
file(GLOB SEP_INC_TD  ${NATID_SDK_INC}/td/*.h)
file(GLOB SEP_INC_GUI ${NATID_SDK_INC}/gui/*.h)

# add executable
add_executable(${SEP_NAME} ${SEP_INCS} ${SEP_SOURCES} ${SEP_INC_TD}  ${SEP_INC_GUI})

source_group("inc"            FILES ${SEP_INCS})
source_group("src"            FILES ${SEP_SOURCES})
source_group("inc\\td"        FILES ${SEP_INC_TD})
source_group("inc\\gui"        FILES ${SEP_INC_GUI})

target_link_libraries(${SEP_NAME} 
    debug ${MU_LIB_DEBUG} 
    debug ${NATGUI_LIB_DEBUG} 
    debug ${DP_LIB_DEBUG}
    debug ${MATRIX_LIB_DEBUG}
    optimized ${MU_LIB_RELEASE}
    optimized ${NATGUI_LIB_RELEASE}
    optimized ${DP_LIB_RELEASE}
    optimized ${MATRIX_LIB_RELEASE}
)

setTargetPropertiesForGUIApp(${SEP_NAME} ${SEP_PLIST})

setIDEPropertiesForGUIExecutable(${SEP_NAME} ${CMAKE_CURRENT_LIST_DIR})

setPlatformDLLPath(${SEP_NAME})