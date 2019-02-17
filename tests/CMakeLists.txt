cmake_minimum_required(VERSION 3.0)
project(h2napi_tests)
set_property(GLOBAL PROPERTY USE_FOLDERS ON)

enable_testing()

if( CMAKE_SIZEOF_VOID_P EQUAL 8 )
    MESSAGE( "64 bits compiler detected" )
    SET( EX_PLATFORM 64 )
    SET( EX_PLATFORM_NAME "x64" )
    SET( EX_PLATFORM_NAME2 "x64" )
else( CMAKE_SIZEOF_VOID_P EQUAL 8 ) 
    MESSAGE( "32 bits compiler detected" )
    SET( EX_PLATFORM 32 )
    SET( EX_PLATFORM_NAME "x86" )
    SET( EX_PLATFORM_NAME2 "x32" )
endif( CMAKE_SIZEOF_VOID_P EQUAL 8 )

set(BIN_ROOT ${CMAKE_SOURCE_DIR}/../bin/${EX_PLATFORM_NAME})
set(LIBS_ROOT ${CMAKE_SOURCE_DIR}/../lib/${EX_PLATFORM_NAME})
message(status "** BIN ROOT ${BIN_ROOT}")
message(status "** LIBS ROOT ${LIBS_ROOT}")

include_directories(
    ${CMAKE_SOURCE_DIR}/../include
)

link_directories(${BIN_ROOT} ${LIBS_ROOT})

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} /O2 /MT /EHsc -D_WIN32_WINNT=0x0601")
set(CMAKE_CXX_FLAGS_RELWITHDEBINFO "${CMAKE_CXX_FLAGS_RELWITHDEBINFO} /MT /EHsc -D_WIN32_WINNT=0x0601")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} /EHsc -D_WIN32_WINNT=0x0601")

add_library(catch_main OBJECT
   ${CMAKE_SOURCE_DIR}/../include/h2napi.h
   ${CMAKE_SOURCE_DIR}/../include/h2napi.hpp
   "unit.cpp"
)
set_property(TARGET catch_main PROPERTY FOLDER "test/h2napi")
source_group("" FILES "unit.cpp")

set_target_properties(catch_main PROPERTIES
   CXX_STANDARD 11
   CXX_STANDARD_REQUIRED ON
)

target_include_directories(
   catch_main 
   PRIVATE 
   ${CMAKE_SOURCE_DIR}/../include
)

set(RM_UNIT_TARGET_NAME "h2napi_unit")
set(RM_UNIT_TARGET_SRC
   unit-h2napi.cpp
)
add_executable(${RM_UNIT_TARGET_NAME}
    $<TARGET_OBJECTS:catch_main>
    ${RM_UNIT_TARGET_SRC}
)
set_property(TARGET ${RM_UNIT_TARGET_NAME} PROPERTY FOLDER "test/${RM_UNIT_TARGET_NAME}")
source_group("" FILES ${RM_UNIT_TARGET_SRC})
set_target_properties(${RM_UNIT_TARGET_NAME} PROPERTIES
    CXX_STANDARD 11
    CXX_STANDARD_REQUIRED ON
)
set_target_properties(${RM_UNIT_TARGET_NAME} PROPERTIES
    COMPILE_DEFINITIONS "_SCL_SECURE_NO_WARNINGS"
    COMPILE_OPTIONS "/EHsc;$<$<CONFIG:Release>:/Od>"
)
target_include_directories(
   ${RM_UNIT_TARGET_NAME} 
   PRIVATE 
   ${CMAKE_SOURCE_DIR}/../include
)

add_test(NAME "TestH2NTableName"
   COMMAND ${RM_UNIT_TARGET_NAME} "TestH2NTableName"
   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
add_test(NAME "TestActivityMonitor"
   COMMAND ${RM_UNIT_TARGET_NAME} "TestActivityMonitor"
   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
add_test(NAME "TestSendCompletedHandHistory"
   COMMAND ${RM_UNIT_TARGET_NAME} "TestSendCompletedHandHistory"
   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
add_test(NAME "TestSendDynamicHandHistory"
   COMMAND ${RM_UNIT_TARGET_NAME} "TestSendDynamicHandHistory"
   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
add_test(NAME "TestSendCloseHudCommand"
   COMMAND ${RM_UNIT_TARGET_NAME} "TestSendCloseHudCommand"
   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)
add_test(NAME "TestSendTableNeedReopenCommand"
   COMMAND ${RM_UNIT_TARGET_NAME} "TestSendTableNeedReopenCommand"
   WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
)



target_link_libraries(${RM_UNIT_TARGET_NAME} h2napi)

add_custom_command(TARGET ${RM_UNIT_TARGET_NAME} POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy "${BIN_ROOT}/h2napi.dll" "$<TARGET_FILE_DIR:${RM_UNIT_TARGET_NAME}>/h2napi.dll"
)