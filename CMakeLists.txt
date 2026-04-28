cmake_minimum_required(VERSION 3.20.0)
project(of_core VERSION 1.0.1 DESCRIPTION "OpenFiles Core Library")

include_directories(
	${of_core_SOURCE_DIR}/include
        ${CMAKE_CURRENT_BINARY_DIR}
	${openfiles_SOURCE_DIR}/include
	${openfiles_SOURCE_DIR}/of_netbios/include
	)

set(SRCS
        src/app.c
        src/backtrace.c
        src/console.c
        src/core.c
	src/dce.c
        src/dom.c
        src/env.c
        src/event.c
        src/file.c
	src/file_call.c
        src/framework.c
        src/fs.c
        src/handle.c
        src/iovec.c
        src/heap.c
        src/libc.c
        src/lock.c
        src/message.c
        src/net.c
        src/ntop.c
        src/path.c
	src/perf.c
        src/persist.c
        src/process.c
        src/queue.c
        src/sax.c
        src/sched.c
        src/socket.c
        src/thread.c
        src/time.c
        src/timer.c
	src/utf.c
        src/waitq.c
        src/waitset.c
        )

if(OFC_NETMON)
    list(APPEND SRCS src/netmon.c)
endif()

if (OFC_FS_PIPE)
    message("Adding FS Pipe")
    list(APPEND fs_library_list of_core_fs_pipe)
endif ()

if (OFC_FS_DARWIN)
    message("Adding FS Darwin")
    list(APPEND fs_library_list of_core_fs_darwin)
endif ()

if (OFC_FS_WIN32)
    message("Adding FS Windows")
    list(APPEND fs_library_list of_core_fs_windows)
endif ()

if (OFC_FS_LINUX)
    message("Adding FS Linux")
    list(APPEND fs_library_list of_core_fs_linux)
endif ()

if (OFC_FS_ANDROID)
    message("Adding FS Android")
    list(APPEND fs_library_list of_core_fs_android)
endif ()

if (OFC_FS_BOOKMARKS)
    message("Adding FS Bookmarks")
    list(APPEND fs_library_list of_core_fs_bookmarks)
endif ()

#if (OFC_KERBEROS)
#    list(APPEND fs_library_list of_security ${KRB5_LIBS})
#    if (APPLE)
#      list(APPEND fs_library_list ${OPENSSL_CRYPTO_LIBRARY} ${OPENSSL_SSL_LIBRARY})
#    endif()
#endif()

add_library(of_core OBJECT ${SRCS})
set_property(TARGET of_core PROPERTY POSITION_INDEPENDENT_CODE ON)

add_library(of_core_static STATIC $<TARGET_OBJECTS:of_core>)
add_library(of_core_shared SHARED $<TARGET_OBJECTS:of_core>)

set_target_properties(of_core_shared PROPERTIES VERSION ${PROJECT_VERSION})
set_target_properties(of_core_static PROPERTIES VERSION ${PROJECT_VERSION})

set(OFC_HEADERS
    ${openfiles_BINARY_DIR}/ofc/config.h
    include/ofc/core.h
    include/ofc/file.h
    include/ofc/framework.h
    include/ofc/fstype.h
    include/ofc/handle.h
    include/ofc/net.h
    include/ofc/process.h
    include/ofc/queue.h
    include/ofc/types.h
    include/ofc/waitset.h
    include/ofc/path.h
    include/ofc/time.h
    )

set_target_properties(of_core_shared PROPERTIES
    SOVERSION 1
    PUBLIC_HEADER "${OFC_HEADERS}"
)
set_target_properties(of_core_static PROPERTIES
    SOVERSION 1
    PUBLIC_HEADER "${OFC_HEADERS}"
    )

target_include_directories(of_core PUBLIC include)

target_link_libraries(of_core_static PUBLIC
        ${fs_library_list}
        of_core_${OFC_HEAP_IMPL}
        of_core_${OFC_IMPL}
        )
target_link_libraries(of_core_shared PUBLIC
        ${fs_library_list}
        of_core_${OFC_HEAP_IMPL}
        of_core_${OFC_IMPL}
        )

add_subdirectory(test)

install(TARGETS of_core_shared of_core_static
        ARCHIVE DESTINATION ${CMAKE_INSTALL_LIBDIR}
        LIBRARY DESTINATION ${CMAKE_INSTALL_LIBDIR}
        RUNTIME DESTINATION ${CMAKE_INSTALL_BINDIR}/openfiles
        PUBLIC_HEADER DESTINATION ${CMAKE_INSTALL_INCLUDEDIR}/ofc
        )
