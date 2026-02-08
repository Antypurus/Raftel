function(configure_compiler_warnings
        target_name #input
)
    set(MSVC_WARNINGS
        /W4
        /WX
        /w14265 # class with virtual functions and non-virtual destructor
        /w14640 # thread un-safe static member initialization
        /permissive-
    )

    set(WARNINGS
        -Wall
        -Wextra
        -Wnon-virtual-dtor
        -Woverloaded-virtual
        -Wpedantic
        -Wconversion
        -Wsign-conversion
        -Wnull-dereference
        -Wformat=2
        -Werror
    )

    set(DISABLED_WARNINGS
        -Wno-error=unused-parameter
        -Wno-error=unused-private-field
        -Wno-error=unused-variable
        -Wno-error=include-angled-in-module-purview
        -Wno-error=language-extension-token
        -Wno-language-extension-token
    )

    set(GCC_EXTRA_WARNINGS
        -Wmisleading-indentation
        -Wduplicated-cond
        -Wduplicated-branches
        -Wlogical-op
    )

    if(MSVC)
        target_compile_options(${target_name} PRIVATE ${MSVC_WARNINGS})
    else()
        if(CMAKE_CXX_COMPILER_ID STREQUAL "GNU") # GCC Specific Warnings are added
            target_compile_options(${target_name} PRIVATE ${WARNINGS} ${DISABLED_WARNINGS} ${GCC_EXTRA_WARNINGS})
        else()
            target_compile_options(${target_name} PRIVATE ${WARNINGS} ${DISABLED_WARNINGS})
        endif()
    endif()
endfunction()
