macro(add_tests_in_dir dirname)
    file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/test/${dirname}/*.cc")
    foreach (file ${files})
        get_filename_component(name ${file} NAME_WE)
        add_simple_test(${dirname} ${name})
    endforeach ()
endmacro()

# add executables with project library
macro(add_simple_test dirname name)
    add_executable(${dirname}_${name} ${CMAKE_CURRENT_SOURCE_DIR}/test/${dirname}/${name}.cc)
    target_link_libraries(${dirname}_${name} ${PROJECT_NAME} gtest_main)
    add_test(NAME ${dirname}_${name} COMMAND ${dirname}_${name})
    #    install(TARGETS ${name} DESTINATION bin)
endmacro()

macro(add_simple_apps)
    file(GLOB files "${CMAKE_CURRENT_SOURCE_DIR}/app/*.cc")
    foreach (file ${files})
        get_filename_component(name ${file} NAME_WE)
        add_simple_app(${name})
    endforeach ()
endmacro()

# add executables with project library
macro(add_simple_app name)
    add_executable(${name} ${CMAKE_CURRENT_SOURCE_DIR}/app/${name}.cc)
    target_link_libraries(${name} ${PROJECT_NAME})
    install(TARGETS ${name}
        RUNTIME DESTINATION ${CATKIN_PACKAGE_BIN_DESTINATION}
        )
endmacro()
