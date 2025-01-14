find_package(PythonInterp 3.2.0 REQUIRED)
include(CMakeParseArguments)

set(MINIMUM_QT5_VERSION 5.3.0)

if(DEFINED KUMIR2_DISABLED_SUBDIRS)
#    message(STATUS "Explicitly disabled subdirs: ${KUMIR2_DISABLED_SUBDIRS}")
else()
    # The script exists only if build from main sources tree, but not using SDK
    if(EXISTS "${CMAKE_SOURCE_DIR}/scripts/query_version_info.py")
        execute_process(
            COMMAND ${PYTHON_EXECUTABLE} "${CMAKE_SOURCE_DIR}/scripts/query_version_info.py" "--mode=cmake_disabled_modules" "--toplevel=${CMAKE_SOURCE_DIR}"
            WORKING_DIRECTORY ${CMAKE_SOURCE_DIR}
            OUTPUT_VARIABLE DISABLED_SUBDIRS
        )
    endif()
endif()

if(NOT DEFINED KUMIR2_INSTALL_PREFIX)
    set(KUMIR2_INSTALL_PREFIX "/usr")
endif(NOT DEFINED KUMIR2_INSTALL_PREFIX)

function(add_opt_subdirectory SUBDIR_NAME)
    if(EXISTS ${CMAKE_CURRENT_SOURCE_DIR}/${SUBDIR_NAME})
        string(FIND "${DISABLED_SUBDIRS}" "[${SUBDIR_NAME}]" SubdirDisableIndex)
        if(${SubdirDisableIndex} EQUAL -1)
            list(FIND KUMIR2_DISABLED_SUBDIRS "${SUBDIR_NAME}" ExplicitSubdirDisableIndex)
            if(ExplicitSubdirDisableIndex EQUAL -1)
                add_subdirectory(${SUBDIR_NAME})
            else()
                message(STATUS "Component ${SUBDIR_NAME} disabled for this build")
            endif()
        else()
            message(STATUS "Component ${SUBDIR_NAME} disabled for this version")
        endif()
    endif()
endfunction(add_opt_subdirectory)

if(NOT DEFINED USE_QT)
    set(USE_QT 4)
endif(NOT DEFINED USE_QT)

if(NOT USE_QT GREATER 4)
    message(WARNING "Support for Qt4 will be discontinued soon. Please upgrade to Qt5!")
endif()

include_directories(${CMAKE_CURRENT_BINARY_DIR})
include_directories(${CMAKE_CURRENT_SOURCE_DIR})

# Find lrelease, moc and uic tools from Qt
if(${USE_QT} GREATER 4)
    # Find Qt5
    find_package(Qt5 ${MINIMUM_QT5_VERSION} COMPONENTS Core Widgets REQUIRED)
    set(QT_LRELEASE_EXECUTABLE "${_qt5Core_install_prefix}/tools/qt5/bin/lrelease")
else()
    # Find Qt4
    find_package(Qt4 4.8.0 COMPONENTS QtCore REQUIRED)
    include(${QT_USE_FILE})
endif()


# Finds Qt modules by components name
function(kumir2_use_qt)
    set(_QT_COMPONENTS)
    set(_QT_LIBRARIES)
    set(_QT_INCLUDES)
    foreach(component ${ARGN})
        if(${USE_QT} GREATER 4)
            if(${component} MATCHES "Gui")
                list(APPEND _QT_COMPONENTS "Gui" "Widgets" "PrintSupport")
            else()
                list(APPEND _QT_COMPONENTS ${component})
            endif()
        else()
            list(APPEND _QT_COMPONENTS "Qt${component}")
        endif()
    endforeach(component)
    if(${USE_QT} GREATER 4)        
        find_package(Qt5 ${MINIMUM_QT5_VERSION} COMPONENTS ${_QT_COMPONENTS} REQUIRED)
        foreach(component ${_QT_COMPONENTS})
            include_directories(${Qt5${component}_INCLUDE_DIRS})
            list(APPEND _QT_LIBRARIES ${Qt5${component}_LIBRARIES})
            list(APPEND _QT_INCLUDES ${Qt5${component}_INCLUDE_DIRS})
        endforeach(component)
    else()
        set(QT_USE_QTMAIN 1)
        find_package(Qt4 4.8.0 COMPONENTS ${_QT_COMPONENTS} REQUIRED)
        include(${QT_USE_FILE})
        include_directories(${QT_INCLUDE_DIR})
        include_directories("${QT_INCLUDE_DIR}/Qt")
        list(APPEND _QT_INCLUDES ${QT_INCLUDE_DIR})
        foreach(component ${_QT_COMPONENTS})
            string(TOUPPER "${component}" COMPONENT)
            include_directories(${QT_${COMPONENT}_INCLUDE_DIR})
            list(APPEND _QT_LIBRARIES ${QT_${COMPONENT}_LIBRARY})
            list(APPEND _QT_INCLUDES ${QT_${COMPONENT}_INCLUDE_DIR})
        endforeach(component)
        # Export all QT_* variables to parent scope
        get_cmake_property(ALL_VARS VARIABLES)
        string(REGEX MATCHALL "(^|;)QT_[A-Za-z0-9_]+" Q_VARS "${ALL_VARS}")
        foreach(_q_var ${Q_VARS})
            set(${_q_var} ${${_q_var}} PARENT_SCOPE)
        endforeach()
    endif()
    set(QT_LIBRARIES ${_QT_LIBRARIES} PARENT_SCOPE)
    set(QT_INCLUDES ${_QT_INCLUDES} PARENT_SCOPE)
endfunction(kumir2_use_qt)

function(kumir2_qt_moc INFILE FILE_LOCATION)
    set(PARAMS_FILE_NAME "${CMAKE_CURRENT_BINARY_DIR}/${INFILE}_qt_moc.params")
    set(OF "${CMAKE_CURRENT_BINARY_DIR}/${INFILE}_qt_moc.cpp")
    file(WRITE ${PARAMS_FILE_NAME} "")
    get_directory_property(INC_LIST INCLUDE_DIRECTORIES)
    get_directory_property(DEF_LIST COMPILE_DEFINITIONS)
    if(KUMIR2_ROOT)
        list(APPEND INC_LIST "${KUMIR2_ROOT}/include")
    endif(KUMIR2_ROOT)
    list(REMOVE_DUPLICATES INC_LIST)
    foreach(inc ${INC_LIST})
        file(APPEND ${PARAMS_FILE_NAME} "-I${inc}\n")
    endforeach()
    foreach(def ${DEF_LIST})
        file(APPEND ${PARAMS_FILE_NAME} "-D${def}\n")
    endforeach()
	if(APPLE AND ${USE_QT} GREATER 4)
		file(APPEND ${PARAMS_FILE_NAME} "-F${Qt5_DIR}/../../\n")
	endif()
    file(APPEND ${PARAMS_FILE_NAME} "-o\n")
    file(APPEND ${PARAMS_FILE_NAME} "${OF}\n")
    file(APPEND ${PARAMS_FILE_NAME} "${FILE_LOCATION}/${INFILE}\n")
    add_custom_command(
        OUTPUT "${OF}"
        COMMAND ${QT_MOC_EXECUTABLE} "@${PARAMS_FILE_NAME}"
        DEPENDS ${INFILE}
    )    
    set(QT_MOC_OUT_FILE_NAME ${OF} PARENT_SCOPE)
endfunction(kumir2_qt_moc)

function(kumir2_wrap_cpp OUTVAR)
    set(RESULT)
    foreach(source ${ARGN})
        kumir2_qt_moc(${source} "${CMAKE_CURRENT_SOURCE_DIR}")
        list(APPEND RESULT ${QT_MOC_OUT_FILE_NAME})
    endforeach()
    set_source_files_properties(${RESULT} PROPERTIES GENERATED ON)
    set(${OUTVAR} ${RESULT} PARENT_SCOPE)
endfunction(kumir2_wrap_cpp)

macro(kumir2_wrap_ui)
    if(${USE_QT} GREATER 4)
        qt5_wrap_ui(${ARGN})
    else()
        qt4_wrap_ui(${ARGN})
    endif()
endmacro(kumir2_wrap_ui)

macro(kumir2_add_resources)
    if(${USE_QT} GREATER 4)
        qt5_add_resources(${ARGN})
    else()
        qt4_add_resources(${ARGN})
    endif()
endmacro(kumir2_add_resources)

function(kumir2_run_lrelease NAME FROM TO)
    add_custom_command(
        TARGET ${NAME} POST_BUILD
        COMMAND ${QT_LRELEASE_EXECUTABLE} -qm ${TO} ${FROM}
    )
    install(
        FILES ${TO}
        DESTINATION ${KUMIR2_RESOURCES_DIR}/translations/
    )
endfunction(kumir2_run_lrelease)

function(kumir2_handle_translation Name Lang)
    set(CANDIDATE_1 "${CMAKE_SOURCE_DIR}/share/kumir2/translations/${Name}_${Lang}.ts")
    set(CANDIDATE_2 "${CMAKE_CURRENT_SOURCE_DIR}/${Name}_${Lang}.ts")
    file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/${KUMIR2_RESOURCES_DIR}/translations")
    set(QM "${CMAKE_BINARY_DIR}/${KUMIR2_RESOURCES_DIR}/translations/${Name}_${Lang}.qm")
    if(EXISTS ${CANDIDATE_1})
        kumir2_run_lrelease(${Name} ${CANDIDATE_1} ${QM})
    elseif(EXISTS ${CANDIDATE_2})
        kumir2_run_lrelease(${Name} ${CANDIDATE_2} ${QM})
    endif()
endfunction(kumir2_handle_translation)

function(kumir2_copy_resources ComponentName)
    string(TOLOWER ${ComponentName} pluginDir)
    set(SRC_RES ${CMAKE_SOURCE_DIR}/share/kumir2/${pluginDir}/)
    if(EXISTS ${SRC_RES})
        file(GLOB resfiles RELATIVE ${SRC_RES} ${SRC_RES}/*)
        foreach(res IN ITEMS ${resfiles})
            get_filename_component(subdir ${res} PATH)
            file(MAKE_DIRECTORY "${CMAKE_BINARY_DIR}/${KUMIR2_RESOURCES_DIR}/${pluginDir}/${subdir}")
            file(COPY "${CMAKE_SOURCE_DIR}/share/kumir2/${pluginDir}/${res}" DESTINATION "${CMAKE_BINARY_DIR}/${KUMIR2_RESOURCES_DIR}/${pluginDir}/${subdir}")
            install(DIRECTORY "${CMAKE_SOURCE_DIR}/share/kumir2/${pluginDir}/${subdir}" DESTINATION "${KUMIR2_RESOURCES_DIR}/${pluginDir}")
        endforeach(res)
    endif()
endfunction(kumir2_copy_resources)

function(kumir2_add_library)
    if(KUMIR2_ROOT)
        include_directories("${KUMIR2_ROOT}/include")
    endif(KUMIR2_ROOT)
    cmake_parse_arguments(PARSED_ARGS "" "NAME" "SOURCES;LIBRARIES" ${ARGN})
    string(TOUPPER "${PARSED_ARGS_NAME}_LIBRARY" LIB_DEF)
    add_library(${PARSED_ARGS_NAME} STATIC SHARED ${PARSED_ARGS_SOURCES})
    if(PARSED_ARGS_LIBRARIES)
        target_link_libraries(${PARSED_ARGS_NAME} ${PARSED_ARGS_LIBRARIES})
    endif(PARSED_ARGS_LIBRARIES)
    set_property(TARGET ${PARSED_ARGS_NAME} APPEND PROPERTY COMPILE_DEFINITIONS ${LIB_DEF})
    set_property(TARGET ${PARSED_ARGS_NAME} APPEND PROPERTY COMPILE_FLAGS "${KUMIR2_CXXFLAGS} ${KUMIR2_CXXFLAGS_${CMAKE_BUILD_TYPE}}")
    set_property(TARGET ${PARSED_ARGS_NAME} APPEND PROPERTY LINK_FLAGS "${KUMIR2_LIBRARY_LINKER_FLAGS}")
    set_property(TARGET ${PARSED_ARGS_NAME} APPEND PROPERTY LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${KUMIR2_LIBS_DIR}")
    set_property(TARGET ${PARSED_ARGS_NAME} APPEND PROPERTY RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${KUMIR2_LIBS_DIR}")
    set_property(TARGET ${PARSED_ARGS_NAME} APPEND PROPERTY LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/${KUMIR2_LIBS_DIR}")
    set_property(TARGET ${PARSED_ARGS_NAME} APPEND PROPERTY RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/${KUMIR2_LIBS_DIR}")
    set_property(TARGET ${PARSED_ARGS_NAME} APPEND PROPERTY LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/${KUMIR2_LIBS_DIR}")
    set_property(TARGET ${PARSED_ARGS_NAME} APPEND PROPERTY RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/${KUMIR2_LIBS_DIR}")
    kumir2_handle_translation(${PARSED_ARGS_NAME} "ru")
    kumir2_copy_resources(${PARSED_ARGS_NAME})
    install(TARGETS ${PARSED_ARGS_NAME} DESTINATION ${KUMIR2_LIBS_DIR})
endfunction(kumir2_add_library)

function(kumir2_add_plugin)
    if(KUMIR2_ROOT)
        include_directories("${KUMIR2_ROOT}/include")
    endif(KUMIR2_ROOT)
    cmake_parse_arguments(PARSED_ARGS "" "NAME" "SOURCES;LIBRARIES" ${ARGN})    
    add_library(${PARSED_ARGS_NAME} SHARED ${PARSED_ARGS_SOURCES})
    if(PARSED_ARGS_LIBRARIES)
        target_link_libraries(${PARSED_ARGS_NAME} ${PARSED_ARGS_LIBRARIES})
    endif(PARSED_ARGS_LIBRARIES)
    set_property(TARGET ${PARSED_ARGS_NAME} APPEND PROPERTY COMPILE_FLAGS "${KUMIR2_CXXFLAGS} ${KUMIR2_CXXFLAGS_${CMAKE_BUILD_TYPE}}")
    set_property(TARGET ${PARSED_ARGS_NAME} APPEND PROPERTY LINK_FLAGS "${KUMIR2_PLUGIN_LINKER_FLAGS}")
    set_property(TARGET ${PARSED_ARGS_NAME} APPEND PROPERTY LIBRARY_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${KUMIR2_PLUGINS_DIR}")
    set_property(TARGET ${PARSED_ARGS_NAME} APPEND PROPERTY RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${KUMIR2_PLUGINS_DIR}")
    set_property(TARGET ${PARSED_ARGS_NAME} APPEND PROPERTY LIBRARY_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/${KUMIR2_PLUGINS_DIR}")
    set_property(TARGET ${PARSED_ARGS_NAME} APPEND PROPERTY RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/${KUMIR2_PLUGINS_DIR}")
    set_property(TARGET ${PARSED_ARGS_NAME} APPEND PROPERTY LIBRARY_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/${KUMIR2_PLUGINS_DIR}")
    set_property(TARGET ${PARSED_ARGS_NAME} APPEND PROPERTY RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/${KUMIR2_PLUGINS_DIR}")
    kumir2_handle_translation(${PARSED_ARGS_NAME} "ru")
    kumir2_copy_resources(${PARSED_ARGS_NAME})    
    install(TARGETS ${PARSED_ARGS_NAME} DESTINATION ${KUMIR2_PLUGINS_DIR})
endfunction(kumir2_add_plugin)

function(kumir2_add_actor)
    if(KUMIR2_ROOT)
        include_directories("${KUMIR2_ROOT}/include")
    endif(KUMIR2_ROOT)
    cmake_parse_arguments(PARSED_ARGS "" "NAME" "SOURCES;LIBRARIES" ${ARGN})
    string(TOLOWER "${PARSED_ARGS_NAME}.json" JSON_FILE_NAME)
    string(TOLOWER "${PARSED_ARGS_NAME}modulebase" MODULEBASE)
    string(TOLOWER "${PARSED_ARGS_NAME}plugin" PLUGIN)
    set(PLUGIN_NAME "Actor${PARSED_ARGS_NAME}")
    if(EXISTS "${CMAKE_SOURCE_DIR}/scripts/gen_actor_source.py")
        set(GEN_ACTOR_SOURCE_SCRIPT "${CMAKE_SOURCE_DIR}/scripts/gen_actor_source.py")
        set(PLUGIN_NAME_PREFIX "Actor")
    else()
        set(GEN_ACTOR_SOURCE_SCRIPT "${KUMIR2_SDK_SCRIPTS_DIR}/gen_actor_source.py")
        set(PLUGIN_NAME_PREFIX "AddonActor")
    endif()
    set(MOC_PARAMS)
    if(${USE_QT} GREATER 4)
        foreach(IDIR ${Qt5Core_INCLUDE_DIRS})
            list(APPEND MOC_PARAMS "-I${IDIR}")
        endforeach()
    endif()
    set(SOURCES_GENERATED ${PLUGIN}.cpp ${MODULEBASE}.cpp)
    set(HEADERS_GENERATED ${PLUGIN}.h ${MODULEBASE}.h)
    add_custom_command(
        OUTPUT
            ${SOURCES_GENERATED}
            ${HEADERS_GENERATED}
        COMMAND ${PYTHON_EXECUTABLE}
            ${GEN_ACTOR_SOURCE_SCRIPT}
            "--update"
            "${CMAKE_CURRENT_SOURCE_DIR}/${JSON_FILE_NAME}"
        DEPENDS
            "${GEN_ACTOR_SOURCE_SCRIPT}"
            "${CMAKE_CURRENT_SOURCE_DIR}/${JSON_FILE_NAME}"
    )
    set(MOC_SOURCES_GENERATED)
    kumir2_qt_moc("${MODULEBASE}.h" ${CMAKE_CURRENT_BINARY_DIR})
    list(APPEND MOC_SOURCES_GENERATED ${QT_MOC_OUT_FILE_NAME})
    kumir2_qt_moc("${PLUGIN}.h" ${CMAKE_CURRENT_BINARY_DIR})
    list(APPEND MOC_SOURCES_GENERATED ${QT_MOC_OUT_FILE_NAME})

    kumir2_add_plugin(
        NAME        "${PLUGIN_NAME_PREFIX}${PARSED_ARGS_NAME}"
        SOURCES     ${PARSED_ARGS_SOURCES}
                    ${SOURCES_GENERATED}
                    ${MOC_SOURCES_GENERATED}
        LIBRARIES   ${PARSED_ARGS_LIBRARIES}
                    ${QT_LIBRARIES}
                    ExtensionSystem Widgets
    )

    kumir2_copy_resources("actors/${PARSED_ARGS_NAME}")

endfunction(kumir2_add_actor)

function(kumir2_add_launcher)    
    cmake_parse_arguments(PARSED_ARGS "" "NAME;SPLASHSCREEN;CONFIGURATION;WINDOW_ICON;APP_ICON_NAME;X_ICONS_DIR;WIN_ICONS_DIR;X_NAME;X_NAME_ru;X_CATEGORIES;APP_NAME;APP_NAME_ru;VENDOR_NAME;VENDOR_NAME_ru;APP_VERSION;APP_LICENSE;APP_LICENSE_ru;APP_ABOUT;APP_ABOUT_ru" "" ${ARGN})
    if(EXISTS "${CMAKE_SOURCE_DIR}/src/app/kumir2-launcher.cpp")
        set(LAUNCHER_SOURCES "${CMAKE_SOURCE_DIR}/src/app/kumir2-launcher.cpp")
    else()
        set(LAUNCHER_SOURCES "${KUMIR2_SDK_SRC_DIR}/kumir2-launcher.cpp")
    endif()
    if(PARSED_ARGS_APP_ICON_NAME)
        if(WIN32)
            if(PARSED_ARGS_WIN_ICONS_DIR)
                set(ICONS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${PARSED_ARGS_WIN_ICONS_DIR}/")
            else()
                set(ICONS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/")
            endif()
        elseif(NOT APPLE)
            if(PARSED_ARGS_X_ICONS_DIR)
                set(ICONS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/${PARSED_ARGS_X_ICONS_DIR}/")
            else()
                set(ICONS_DIR "${CMAKE_CURRENT_SOURCE_DIR}/")
            endif()
        endif()
    endif(PARSED_ARGS_APP_ICON_NAME)
    kumir2_use_qt(Core Gui)
    set(QRC_FILE "${CMAKE_CURRENT_BINARY_DIR}/${PARSED_ARGS_NAME}.qrc")
    file(WRITE ${QRC_FILE} "<RCC>\n")
    file(APPEND ${QRC_FILE} "<qresource prefix=\"/kumir2-launcher\">\n")
    if(PARSED_ARGS_SPLASHSCREEN)
        file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/${PARSED_ARGS_SPLASHSCREEN}" DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
        file(APPEND ${QRC_FILE} "<file>")
        file(APPEND ${QRC_FILE} "${PARSED_ARGS_SPLASHSCREEN}")
        file(APPEND ${QRC_FILE} "</file>\n")
    endif()
    if(PARSED_ARGS_WINDOW_ICON)
        file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/${PARSED_ARGS_WINDOW_ICON}" DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
        file(APPEND ${QRC_FILE} "<file>")
        file(APPEND ${QRC_FILE} "${PARSED_ARGS_WINDOW_ICON}")
        file(APPEND ${QRC_FILE} "</file>\n")
    endif()
    if(PARSED_ARGS_APP_LICENSE)
        file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/${PARSED_ARGS_APP_LICENSE}" DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
        file(APPEND ${QRC_FILE} "<file>")
        get_filename_component(LOCAL_APP_LICENSE ${PARSED_ARGS_APP_LICENSE} NAME)
        file(APPEND ${QRC_FILE} "${LOCAL_APP_LICENSE}")
        file(APPEND ${QRC_FILE} "</file>\n")
    endif()
    if(PARSED_ARGS_APP_LICENSE_ru)
        file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/${PARSED_ARGS_APP_LICENSE_ru}" DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
        file(APPEND ${QRC_FILE} "<file>")
        get_filename_component(LOCAL_APP_LICENSE_ru ${PARSED_ARGS_APP_LICENSE_ru} NAME)
        file(APPEND ${QRC_FILE} "${LOCAL_APP_LICENSE_ru}")
        file(APPEND ${QRC_FILE} "</file>\n")
    endif()
    if(PARSED_ARGS_APP_ABOUT)
        file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/${PARSED_ARGS_APP_ABOUT}" DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
        file(APPEND ${QRC_FILE} "<file>")
        get_filename_component(LOCAL_APP_ABOUT ${PARSED_ARGS_APP_ABOUT} NAME)
        file(APPEND ${QRC_FILE} "${LOCAL_APP_ABOUT}")
        file(APPEND ${QRC_FILE} "</file>\n")
    endif()
    if(PARSED_ARGS_APP_ABOUT_ru)
        file(COPY "${CMAKE_CURRENT_SOURCE_DIR}/${PARSED_ARGS_APP_ABOUT_ru}" DESTINATION ${CMAKE_CURRENT_BINARY_DIR})
        file(APPEND ${QRC_FILE} "<file>")
        get_filename_component(LOCAL_APP_ABOUT_ru ${PARSED_ARGS_APP_ABOUT_ru} NAME)
        file(APPEND ${QRC_FILE} "${LOCAL_APP_ABOUT_ru}")
        file(APPEND ${QRC_FILE} "</file>\n")
    endif()
    file(APPEND ${QRC_FILE} "</qresource>\n</RCC>\n")
    set_source_files_properties(${QRC_FILE} PROPERTIES GENERATED ON)
    kumir2_add_resources(QRC_SRC ${QRC_FILE})
    if(WIN32 AND EXISTS "${ICONS_DIR}/${PARSED_ARGS_APP_ICON_NAME}.ico")
        set(WIN_RC_FILE "${CMAKE_CURRENT_BINARY_DIR}/${PARSED_ARGS_NAME}.rc")
        set_source_files_properties(${WIN_RC_FILE} PROPERTIES GENERATED ON)
        set(ICON_PATH "${ICONS_DIR}/${PARSED_ARGS_APP_ICON_NAME}.ico")
        string(REPLACE "/" "\\\\" ICON_NATIVE_PATH ${ICON_PATH})
        file(WRITE ${WIN_RC_FILE} "IDI_ICON1 ICON DISCARDABLE \"${ICON_NATIVE_PATH}\"\r\n")
        list(APPEND LAUNCHER_SOURCES ${WIN_RC_FILE})
        add_executable(${PARSED_ARGS_NAME} WIN32 ${LAUNCHER_SOURCES} ${QRC_SRC})
        if(${USE_QT} GREATER 4)
            list(APPEND QT_LIBRARIES ${Qt5Core_QTMAIN_LIBRARIES})
        else()
            list(APPEND QT_LIBRARIES ${QT_QTMAIN_LIBRARY})
        endif()
    else()
        add_executable(${PARSED_ARGS_NAME} ${LAUNCHER_SOURCES} ${QRC_SRC})
    endif()
    target_link_libraries(${PARSED_ARGS_NAME} ${QT_LIBRARIES} ExtensionSystem)
    set_property(TARGET ${PARSED_ARGS_NAME} APPEND PROPERTY COMPILE_FLAGS "${KUMIR2_CXXFLAGS} ${KUMIR2_CXXFLAGS_${CMAKE_BUILD_TYPE}}")
    set_property(TARGET ${PARSED_ARGS_NAME} APPEND PROPERTY LINK_FLAGS ${KUMIR2_LAUNCHER_LINKER_FLAGS})
    set_property(TARGET ${PARSED_ARGS_NAME} APPEND PROPERTY COMPILE_DEFINITIONS CONFIGURATION_TEMPLATE="${PARSED_ARGS_CONFIGURATION}")
    set_property(TARGET ${PARSED_ARGS_NAME} APPEND PROPERTY COMPILE_DEFINITIONS GIT_HASH="${GIT_HASH}")
    set_property(TARGET ${PARSED_ARGS_NAME} APPEND PROPERTY COMPILE_DEFINITIONS GIT_TIMESTAMP="${GIT_TIMESTAMP}")
    set_property(TARGET ${PARSED_ARGS_NAME} APPEND PROPERTY COMPILE_DEFINITIONS GIT_TAG="${GIT_TAG}")
    set_property(TARGET ${PARSED_ARGS_NAME} APPEND PROPERTY COMPILE_DEFINITIONS GIT_BRANCH="${GIT_BRANCH}")
    if(PARSED_ARGS_SPLASHSCREEN)
        set_property(TARGET ${PARSED_ARGS_NAME} APPEND PROPERTY COMPILE_DEFINITIONS SPLASHSCREEN="${PARSED_ARGS_SPLASHSCREEN}")
    endif(PARSED_ARGS_SPLASHSCREEN)
    if(PARSED_ARGS_WINDOW_ICON)
        set_property(TARGET ${PARSED_ARGS_NAME} APPEND PROPERTY COMPILE_DEFINITIONS WINDOW_ICON="${PARSED_ARGS_WINDOW_ICON}")
    endif(PARSED_ARGS_WINDOW_ICON)
    if(PARSED_ARGS_APP_NAME)
        set_property(TARGET ${PARSED_ARGS_NAME} APPEND PROPERTY COMPILE_DEFINITIONS APP_NAME="${PARSED_ARGS_APP_NAME}")
    endif()
    if(PARSED_ARGS_APP_NAME_ru)
        set_property(TARGET ${PARSED_ARGS_NAME} APPEND PROPERTY COMPILE_DEFINITIONS APP_NAME_ru="${PARSED_ARGS_APP_NAME_ru}")
    endif()
    if(PARSED_ARGS_APP_VENDOR)
        set_property(TARGET ${PARSED_ARGS_NAME} APPEND PROPERTY COMPILE_DEFINITIONS APP_VENDOR="${PARSED_ARGS_APP_VENDOR}")
    endif()
    if(PARSED_ARGS_APP_VENDOR_ru)
        set_property(TARGET ${PARSED_ARGS_NAME} APPEND PROPERTY COMPILE_DEFINITIONS APP_VENDOR_ru="${PARSED_ARGS_APP_VENDOR_ru}")
    endif()
    if(PARSED_ARGS_APP_VERSION)
        set_property(TARGET ${PARSED_ARGS_NAME} APPEND PROPERTY COMPILE_DEFINITIONS APP_VERSION="${PARSED_ARGS_APP_VERSION}")
    endif()
    if(LOCAL_APP_LICENSE)
        set_property(TARGET ${PARSED_ARGS_NAME} APPEND PROPERTY COMPILE_DEFINITIONS APP_LICENSE="${LOCAL_APP_LICENSE}")
    endif()
    if(LOCAL_APP_LICENSE_ru)
        set_property(TARGET ${PARSED_ARGS_NAME} APPEND PROPERTY COMPILE_DEFINITIONS APP_LICENSE_ru="${LOCAL_APP_LICENSE_ru}")
    endif()
    if(LOCAL_APP_ABOUT)
        set_property(TARGET ${PARSED_ARGS_NAME} APPEND PROPERTY COMPILE_DEFINITIONS APP_ABOUT="${LOCAL_APP_ABOUT}")
    endif()
    if(LOCAL_APP_ABOUT_ru)
        set_property(TARGET ${PARSED_ARGS_NAME} APPEND PROPERTY COMPILE_DEFINITIONS APP_ABOUT_ru="${LOCAL_APP_ABOUT_ru}")
    endif()
    set_property(TARGET ${PARSED_ARGS_NAME} APPEND PROPERTY COMPILE_DEFINITIONS KUMIR2_PLUGINS_DIR="${KUMIR2_PLUGINS_DIR}")
    set_property(TARGET ${PARSED_ARGS_NAME} APPEND PROPERTY COMPILE_DEFINITIONS KUMIR2_RESOURCES_DIR="${KUMIR2_RESOURCES_DIR}")
    set_property(TARGET ${PARSED_ARGS_NAME} APPEND PROPERTY COMPILE_DEFINITIONS KUMIR2_EXEC_DIR="${KUMIR2_EXEC_DIR}")
    set_property(TARGET ${PARSED_ARGS_NAME} APPEND PROPERTY COMPILE_DEFINITIONS KUMIR2_LIBS_DIR="${KUMIR2_LIBS_DIR}")
    set_property(TARGET ${PARSED_ARGS_NAME} APPEND PROPERTY RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${KUMIR2_EXEC_DIR}")
    set_property(TARGET ${PARSED_ARGS_NAME} APPEND PROPERTY RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/${KUMIR2_EXEC_DIR}")
    set_property(TARGET ${PARSED_ARGS_NAME} APPEND PROPERTY RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/${KUMIR2_EXEC_DIR}")
    install(TARGETS ${PARSED_ARGS_NAME} DESTINATION ${KUMIR2_EXEC_DIR})
    if(KUMIR2_XDG_APPLICATIONS_DIR AND PARSED_ARGS_X_NAME)
        set(DESKTOP_FILE "${CMAKE_CURRENT_BINARY_DIR}/${PARSED_ARGS_NAME}.desktop")
        file(WRITE ${DESKTOP_FILE} "[Desktop Entry]\n")
        file(APPEND ${DESKTOP_FILE} "Type=Application\n")
        file(APPEND ${DESKTOP_FILE} "Exec=${PARSED_ARGS_NAME} %U\n")
        if(PARSED_ARGS_APP_ICON_NAME)
            file(APPEND ${DESKTOP_FILE} "Icon=${PARSED_ARGS_APP_ICON_NAME}\n")
        endif(PARSED_ARGS_APP_ICON_NAME)
        file(APPEND ${DESKTOP_FILE} "Name=${PARSED_ARGS_X_NAME}\n")
        if(PARSED_ARGS_X_NAME_ru)  # TODO make support for several languages
            file(APPEND ${DESKTOP_FILE} "Name[ru]=${PARSED_ARGS_X_NAME_ru}\n")
        endif(PARSED_ARGS_X_NAME_ru)
        if(PARSED_ARGS_X_CATEGORIES)
            string(REPLACE "," ";" X_CATS ${PARSED_ARGS_X_CATEGORIES})
            file(APPEND ${DESKTOP_FILE} "Categories=${X_CATS};\n")
        endif(PARSED_ARGS_X_CATEGORIES)
        set_source_files_properties(${DESKTOP_FILE} PROPERTIES GENERATED ON)
        install(FILES ${DESKTOP_FILE} DESTINATION ${KUMIR2_XDG_APPLICATIONS_DIR})
    endif(KUMIR2_XDG_APPLICATIONS_DIR AND PARSED_ARGS_X_NAME)
    if(PARSED_ARGS_APP_ICON_NAME AND NOT WIN32 AND NOT APPLE)
        if(KUMIR2_XDG_ICONS_DIR)
            file(GLOB_RECURSE icons RELATIVE ${ICONS_DIR} "${ICONS_DIR}/*/apps/${PARSED_ARGS_APP_ICON_NAME}.*")
            foreach(icon IN ITEMS ${icons})
                get_filename_component(subdir ${icon} PATH)
                install(FILES "${ICONS_DIR}/${icon}" DESTINATION "${KUMIR2_XDG_ICONS_DIR}/hicolor/${subdir}")
            endforeach(icon)
        endif(KUMIR2_XDG_ICONS_DIR)
        if(KUMIR2_XDG_PIXMAPS_DIR AND EXISTS "${ICONS_DIR}/${PARSED_ARGS_APP_ICON_NAME}.png")
            install(FILES "${ICONS_DIR}/${PARSED_ARGS_APP_ICON_NAME}.png" DESTINATION "${KUMIR2_XDG_PIXMAPS_DIR}")
        endif()
        if(KUMIR2_XDG_PIXMAPS_DIR AND EXISTS "${ICONS_DIR}/${PARSED_ARGS_APP_ICON_NAME}.svg")
            install(FILES "${ICONS_DIR}/${PARSED_ARGS_APP_ICON_NAME}.svg" DESTINATION "${KUMIR2_XDG_PIXMAPS_DIR}")
        endif()
    endif(PARSED_ARGS_APP_ICON_NAME AND NOT WIN32 AND NOT APPLE)
endfunction(kumir2_add_launcher)

function(kumir2_add_tool)
    cmake_parse_arguments(PARSED_ARGS "" "NAME" "SOURCES;LIBRARIES;CXXFLAGS" ${ARGN})
    kumir2_use_qt(Core Gui)
    if(WIN32 AND EXISTS "${CMAKE_CURRENT_SOURCE_DIR}/${PARSED_ARGS_NAME}.rc")
        list(APPEND PARSED_ARGS_SOURCES "${CMAKE_CURRENT_SOURCE_DIR}/${PARSED_ARGS_NAME}.rc")
        add_executable(${PARSED_ARGS_NAME} WIN32 ${PARSED_ARGS_SOURCES})
        if(${USE_QT} GREATER 4)
            list(APPEND PARSED_ARGS_LIBRARIES ${Qt5Core_QTMAIN_LIBRARIES})
        else()
            list(APPEND PARSED_ARGS_LIBRARIES ${QT_QTMAIN_LIBRARY})
        endif()
    else()
        add_executable(${PARSED_ARGS_NAME} ${PARSED_ARGS_SOURCES})
    endif()
    if(PARSED_ARGS_LIBRARIES)
        target_link_libraries(${PARSED_ARGS_NAME} ${PARSED_ARGS_LIBRARIES})
    endif(PARSED_ARGS_LIBRARIES)
    if(PARSED_ARGS_CXXFLAGS)
        set_property(TARGET ${PARSED_ARGS_NAME} APPEND PROPERTY COMPILE_FLAGS "${PARSED_ARGS_CXXFLAGS} ${KUMIR2_CXXFLAGS_${CMAKE_BUILD_TYPE}}")
    else()
        set_property(TARGET ${PARSED_ARGS_NAME} APPEND PROPERTY COMPILE_FLAGS "${KUMIR2_CXXFLAGS} ${KUMIR2_CXXFLAGS_${CMAKE_BUILD_TYPE}}")
    endif(PARSED_ARGS_CXXFLAGS)
    set_property(TARGET ${PARSED_ARGS_NAME} APPEND PROPERTY LINK_FLAGS "${KUMIR2_TOOL_LINKER_FLAGS}")
    set_property(TARGET ${PARSED_ARGS_NAME} APPEND PROPERTY RUNTIME_OUTPUT_DIRECTORY "${CMAKE_BINARY_DIR}/${KUMIR2_LIBEXECS_DIR}")
    set_property(TARGET ${PARSED_ARGS_NAME} APPEND PROPERTY RUNTIME_OUTPUT_DIRECTORY_DEBUG "${CMAKE_BINARY_DIR}/${KUMIR2_LIBEXECS_DIR}")
    set_property(TARGET ${PARSED_ARGS_NAME} APPEND PROPERTY RUNTIME_OUTPUT_DIRECTORY_RELEASE "${CMAKE_BINARY_DIR}/${KUMIR2_LIBEXECS_DIR}")
    kumir2_handle_translation(${PARSED_ARGS_NAME} "ru")
    kumir2_copy_resources(${PARSED_ARGS_NAME})
    install(TARGETS ${PARSED_ARGS_NAME} DESTINATION ${KUMIR2_LIBEXECS_DIR})
endfunction(kumir2_add_tool)
