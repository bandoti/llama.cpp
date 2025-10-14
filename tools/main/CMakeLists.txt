set(TARGET llama-cli)
add_executable(${TARGET} main.cpp conversation.cpp conversation.h display.cpp display.h display_renderer.h console_display_renderer.cpp console_display_renderer.h console_manager.cpp console_manager.h)
target_link_libraries(${TARGET} PRIVATE common llama ${CMAKE_THREAD_LIBS_INIT})
target_compile_features(${TARGET} PRIVATE cxx_std_17)

if(LLAMA_TOOLS_INSTALL)
    install(TARGETS ${TARGET} RUNTIME)
endif()
