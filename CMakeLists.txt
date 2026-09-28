cmake_minimum_required(VERSION 3.21)
project(MosuanPlayer VERSION 0.1.0 LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_AUTOMOC ON)
set(CMAKE_AUTOUIC ON)
set(CMAKE_AUTORCC ON)

find_package(Qt6 REQUIRED COMPONENTS Widgets)

if(DEFINED LIBVLC_ROOT)
    set(LIBVLC_INCLUDE_DIR "${LIBVLC_ROOT}/include")
    if(WIN32)
        set(LIBVLC_LIBRARY "${LIBVLC_ROOT}/lib/libvlc.lib")
    elseif(APPLE)
        set(LIBVLC_LIBRARY "${LIBVLC_ROOT}/lib/libvlc.dylib")
    endif()
else()
    find_path(LIBVLC_INCLUDE_DIR vlc/vlc.h)
    find_library(LIBVLC_LIBRARY NAMES vlc libvlc)
endif()

if(NOT LIBVLC_INCLUDE_DIR OR NOT LIBVLC_LIBRARY)
    message(WARNING "LibVLC SDK was not found. Configure LIBVLC_ROOT before building the player.")
endif()

add_executable(MosuanPlayer
    src/main.cpp
    src/core/vlc/VLCInstance.h
    src/core/vlc/VLCInstance.cpp
    src/core/vlc/VLCPlayer.h
    src/core/vlc/VLCPlayer.cpp
    src/features/crop/CropRect.h
    src/features/crop/CropRect.cpp
    src/features/playlist/PlaylistModel.h
    src/features/playlist/PlaylistModel.cpp
    src/features/profile/VideoProfile.h
    src/features/profile/VideoProfileStore.h
    src/features/profile/VideoProfileStore.cpp
    src/features/display/SmartInvertController.h
    src/features/display/SmartInvertController.cpp
    src/features/display/SmartInvertEngine.h
    src/features/display/SmartInvertEngine.cpp
    src/features/display/VideoFramePipeline.h
    src/features/display/VideoFramePipeline.cpp
    src/ui/VideoWidget.h
    src/ui/VideoWidget.cpp
    src/ui/PlayerToolbar.h
    src/ui/PlayerToolbar.cpp
    src/ui/MainWindow.h
    src/ui/MainWindow.cpp
)

target_include_directories(MosuanPlayer PRIVATE src ${LIBVLC_INCLUDE_DIR})

target_link_libraries(MosuanPlayer PRIVATE Qt6::Widgets ${LIBVLC_LIBRARY})

if(WIN32)
    set_target_properties(MosuanPlayer PROPERTIES WIN32_EXECUTABLE TRUE)
elseif(APPLE)
    set_target_properties(MosuanPlayer PROPERTIES MACOSX_BUNDLE TRUE)
endif()
