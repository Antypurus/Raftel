function(extract_platform_dependent_sources
        all_sources # input
        platform_independent_sources # output
        windows_sources #output
        linux_sources #output
        macos_sources #output
        posix_sources #output
)
    set(platform_independent_result)
    set(windows_result)
    set(linux_result)
    set(macos_result)
    set(posix_result)

    foreach(source ${all_sources})
        if(source MATCHES ".*[W|w]indows.*") #windows source file
            list(APPEND windows_result ${source})
        elseif(source MATCHES ".*[L|l]inux.*") #linux source file
            list(APPEND linux_result ${source})
        elseif(source MATCHES ".*[M|m][A|a][C|c][O|o][S|s].*") #macos source file
            list(APPEND macos_result ${source})
        elseif(source MATCHES ".*[P|p][O|o][S|s][I|i][X|x].*") #posix source file
            list(APPEND posix_result ${source})
        elseif(source MATCHES ".*tests.*") # ignore tests files
        elseif(source MATCHES ".*ThirdParty.*") # ignore third party files
        else()
            list(APPEND platform_independent_result ${source})
        endif()
    endforeach()

    set(${platform_independent_sources} ${platform_independent_result} PARENT_SCOPE)
    set(${windows_sources} ${windows_result} PARENT_SCOPE)
    set(${linux_sources} ${linux_result} PARENT_SCOPE)
    set(${macos_sources} ${macos_result} PARENT_SCOPE)
    set(${posix_sources} ${posix_result} PARENT_SCOPE)
endfunction()

