if (NOT DEFINED CMAKE_TOOLCHAIN_FILE)
    message(FATAL_ERROR "CMAKE_TOOLCHAIN_FILE is not set.")
endif()

if (NOT DEFINED VCPKG_DEFAULT_TRIPLET)
    message(FATAL_ERROR "VCPKG_DEFAULT_TRIPLET is not set.")
endif()

if (NOT DEFINED VCPKG_INSTALL_PATH)
    message(WARNING "VCPKG_INSTALL_PATH is not set.")
endif()

function(getLibraryConfigPath libName resultVar)

    set(configPath "${VCPKG_INSTALL_PATH}/installed/${VCPKG_DEFAULT_TRIPLET}/share/${libName}")

    if (EXISTS ${configPath})
        set(${resultVar} ${configPath} PARENT_SCOPE)
    else()
        message(WARNING "Could not find config path for ${libName}")
    endif()

endfunction()