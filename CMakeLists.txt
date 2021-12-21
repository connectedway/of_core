cmake_minimum_required(VERSION 3.21.1)
project(of_core VERSION 1.0.1 DESCRIPTION "OpenFiles Core Library")

include(configs/default)
configure_file(include/ofc/config.h.in ofc/config.h @ONLY)
include_directories(${CMAKE_CURRENT_BINARY_DIR})

set(SRCS
    src/app.c
    src/console.c
    src/core.c
    src/dom.c
    src/env.c
    src/event.c
    src/file.c
    src/framework.c
    src/fs.c
    src/handle.c
    src/heap.c
    src/libc.c
    src/lock.c
    src/message.c
    src/net.c
    src/ntop.c
    src/path.c
    src/persist.c
    src/process.c
    src/queue.c
    src/sax.c
    src/sched.c
    src/socket.c
    src/thread.c
    src/time.c
    src/timer.c
    src/waitq.c
    src/waitset.c
    )

include(FetchContent)

FetchContent_Declare(of_core_${OFC_HEAP_IMPL}
  GIT_REPOSITORY git@github.com:connectedway/of_core_${OFC_HEAP_IMPL}
  GIT_TAG main
  )

FetchContent_MakeAvailable(of_core_${OFC_HEAP_IMPL})

FetchContent_Declare(of_core_${OFC_IMPL}
  GIT_REPOSITORY git@github.com:connectedway/of_core_${OFC_IMPL}
  GIT_TAG main
  )

FetchContent_MakeAvailable(of_core_${OFC_IMPL})

add_library(of_core_static STATIC ${SRCS})
add_library(of_core_shared SHARED ${SRCS})

set_target_properties(of_core_shared PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(of_core_static PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(of_core_shared PROPERTIES SOVERSION 1)
set_target_properties(of_core_static PROPERTIES SOVERSION 1)

target_include_directories (of_core_static PUBLIC include)
target_include_directories (of_core_shared PUBLIC include)
target_link_libraries (of_core_static PRIVATE
		      of_core_${OFC_HEAP_IMPL}
		      of_core_${OFC_IMPL}
		      )
target_link_libraries (of_core_shared PRIVATE
		      of_core_${OFC_HEAP_IMPL}
		      of_core_${OFC_IMPL}
		      )