# Original Author: Siddhartha Chaudhuri
# Maintainer: Rodrigo Hernandez

# Detect MSYS2 environment
if [[ -n "$MSYSTEM" ]]; then
    _msys2=true
    _mingw_prefix="${MINGW_PREFIX:-/mingw64}"
    _pkgprefix="mingw-w64-${MSYSTEM_CARCH}-"
else
    _msys2=false
    _mingw_prefix=""
    _pkgprefix=""
fi

pkgbase=${_pkgprefix}thea
pkgname=("${_pkgprefix}thea" "${_pkgprefix}thea-debug")
# Version is computed dynamically by pkgver() from git
pkgver=0.0.r819.gc8bfc9d
pkgrel=1
pkgdesc="A toolkit for visual computing with a focus on geometry processing"
arch=('x86_64')
url="https://github.com/Kwizatz/Thea.git"
license=('BSD')

if [[ "$_msys2" == true ]]; then
    # MSYS2 dependencies
    makedepends=(
        "${MINGW_PACKAGE_PREFIX}-cmake"
        "${MINGW_PACKAGE_PREFIX}-gcc"
        "${MINGW_PACKAGE_PREFIX}-make"
        "${MINGW_PACKAGE_PREFIX}-eigen3"
        "${MINGW_PACKAGE_PREFIX}-cgal"
        "${MINGW_PACKAGE_PREFIX}-boost"
        "${MINGW_PACKAGE_PREFIX}-gmp"
        "${MINGW_PACKAGE_PREFIX}-mpfr"
        "${MINGW_PACKAGE_PREFIX}-wxwidgets3.2-msw"
        "${MINGW_PACKAGE_PREFIX}-opencl-headers"
        "${MINGW_PACKAGE_PREFIX}-opencl-icd"
        'git'
    )
    depends=(
        "${MINGW_PACKAGE_PREFIX}-eigen3"
        "${MINGW_PACKAGE_PREFIX}-gcc-libs"
    )
    optdepends=(
        "${MINGW_PACKAGE_PREFIX}-cgal: Additional computational geometry algorithms"
        "${MINGW_PACKAGE_PREFIX}-wxwidgets3.2-msw: GUI tools support"
    )
else
    # Arch Linux dependencies
    makedepends=(
        'cmake'
        'gcc'
        'make'
        'eigen'
        'cgal'
        'boost'
        'gmp'
        'mpfr'
        'mesa'
        'glu'
        'libgl'
        'wxwidgets-gtk3'
        'opencl-headers'
        'ocl-icd'
        'git'
    )
    depends=(
        'eigen'
    )
    optdepends=(
        'cgal: Additional computational geometry algorithms'
        'mesa: OpenGL plugin support'
        'wxwidgets-gtk3: GUI tools support'
    )
fi
# Use local source - no remote fetching
source=()
sha256sums=()

# Build options
_with_tools=ON
_with_tests=OFF

# Get the directory where the PKGBUILD is located (the repo root)
_srcroot="$(cd "$(dirname "${BASH_SOURCE[0]}")" && pwd)"

# Compute version from git: 0.0.rCOMMIT_COUNT.gSHORT_HASH (date-based fallback)
pkgver() {
    cd "$_srcroot"
    if git rev-parse --git-dir > /dev/null 2>&1; then
        # Format: 0.0.r<commit_count>.g<short_hash>
        printf "0.0.r%s.g%s" "$(git rev-list --count HEAD)" "$(git rev-parse --short HEAD)"
    else
        # Fallback if not a git repo
        echo "0.0.0"
    fi
}

prepare() {
    # Create build directories in the source directory
    mkdir -p "$_srcroot/build-release"
    mkdir -p "$_srcroot/build-debug"
}

_build_common() {
    local build_type=$1
    local build_dir=$2
    
    cd "$_srcroot/$build_dir"
    
    # Platform-specific settings
    if [[ "$_msys2" == true ]]; then
        local _install_prefix="$_mingw_prefix"
        local _eigen_include="$_mingw_prefix/include/eigen3"
        local _generator="MinGW Makefiles"
        local _make_cmd="mingw32-make"
        local _gl_plugin=OFF  # OpenGL plugin may not work well on MSYS2
    else
        local _install_prefix="/usr"
        local _eigen_include="/usr/include/eigen3"
        local _generator="Unix Makefiles"
        local _make_cmd="make"
        local _gl_plugin=ON
    fi
    
    cmake "$_srcroot/Code/Build" \
        -G "$_generator" \
        -DCMAKE_BUILD_TYPE=$build_type \
        -DCMAKE_INSTALL_PREFIX="$_install_prefix" \
        -DTHEA_DEPS_ROOT="$_install_prefix" \
        -DEIGEN3_INCLUDE_DIR="$_eigen_include" \
        -DWITH_PLUGIN_ARPACK=OFF \
        -DWITH_PLUGIN_CSPARSE=OFF \
        -DWITH_PLUGIN_GL=$_gl_plugin \
        -DWITH_TOOLS=${_with_tools} \
        -DWITH_TESTS=${_with_tests} \
        -DWITH_CGAL=ON \
        -DWITH_FREEIMAGE=OFF \
        -DWITH_LIB3DS=OFF \
        -DWITH_CLUTO=OFF \
        -DLITE=OFF
    
    $_make_cmd -j$(nproc)
}

build() {
    # Build Release version
    _build_common "Release" "build-release"
    
    # Build Debug version
    _build_common "Debug" "build-debug"
}

check() {
    if [[ "${_with_tests}" == "ON" ]]; then
        cd "$_srcroot/build-release"
        if [[ "$_msys2" == true ]]; then
            mingw32-make test || true
        else
            make test || true
        fi
    fi
}

# Get the install prefix based on platform
_get_prefix() {
    if [[ "$_msys2" == true ]]; then
        echo "$_mingw_prefix"
    else
        echo "/usr"
    fi
}

_package_common() {
    local build_dir=$1
    local suffix=$2
    local _prefix=$(_get_prefix)
    
    cd "$_srcroot/$build_dir"
    
    if [[ "$_msys2" == true ]]; then
        mingw32-make DESTDIR="$pkgdir" install
    else
        make DESTDIR="$pkgdir" install
    fi
    
    # Install license
    install -Dm644 "$_srcroot/LICENSE.txt" "$pkgdir${_prefix}/share/licenses/${pkgname}/LICENSE"
    
    # Install documentation
    install -Dm644 "$_srcroot/README.md" "$pkgdir${_prefix}/share/doc/${pkgname}/README.md"
    
    # Create CMake config files
    _install_cmake_config "$suffix"
}

_install_cmake_config() {
    local suffix=$1
    local _prefix=$(_get_prefix)
    local cmake_dir="$pkgdir${_prefix}/lib/cmake/Thea${suffix}"
    
    install -dm755 "$cmake_dir"
    
    # Generate TheaConfig.cmake
    cat > "$cmake_dir/TheaConfig${suffix}.cmake" << 'EOF'
# Thea CMake Configuration File
# 
# This file provides CMake configuration for the Thea library.
# 
# Usage:
#   find_package(Thea REQUIRED)
#   target_link_libraries(your_target Thea::Thea)
#
# Variables defined:
#   Thea_FOUND        - True if Thea was found
#   Thea_INCLUDE_DIRS - Include directories for Thea
#   Thea_LIBRARIES    - Libraries to link against
#   Thea_VERSION      - Version of Thea

@PACKAGE_INIT@

set(Thea_VERSION "@PROJECT_VERSION@")

# Find required dependencies
include(CMakeFindDependencyMacro)
find_dependency(Eigen3 REQUIRED)

# Determine the suffix based on the config file name
get_filename_component(_THEA_CONFIG_NAME "${CMAKE_CURRENT_LIST_FILE}" NAME_WE)
if(_THEA_CONFIG_NAME MATCHES "Debug$")
    set(_THEA_BUILD_TYPE "Debug")
    set(_THEA_LIB_SUFFIX "d")
else()
    set(_THEA_BUILD_TYPE "Release")
    set(_THEA_LIB_SUFFIX "")
endif()

# Set include directories
set(Thea_INCLUDE_DIRS "${CMAKE_CURRENT_LIST_DIR}/../../../include/Thea")
get_filename_component(Thea_INCLUDE_DIRS "${Thea_INCLUDE_DIRS}" ABSOLUTE)

# Set library directory
set(Thea_LIBRARY_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../lib")
get_filename_component(Thea_LIBRARY_DIR "${Thea_LIBRARY_DIR}" ABSOLUTE)

# Find the main library
find_library(Thea_LIBRARY
    NAMES "Thea${_THEA_LIB_SUFFIX}" "Thea"
    PATHS "${Thea_LIBRARY_DIR}"
    NO_DEFAULT_PATH
)

if(NOT Thea_LIBRARY)
    find_library(Thea_LIBRARY
        NAMES "Thea${_THEA_LIB_SUFFIX}" "Thea"
    )
endif()

set(Thea_LIBRARIES ${Thea_LIBRARY})

# Create imported target
if(NOT TARGET Thea::Thea)
    add_library(Thea::Thea STATIC IMPORTED)
    set_target_properties(Thea::Thea PROPERTIES
        IMPORTED_LOCATION "${Thea_LIBRARY}"
        INTERFACE_INCLUDE_DIRECTORIES "${Thea_INCLUDE_DIRS}"
        INTERFACE_LINK_LIBRARIES "Eigen3::Eigen"
    )
endif()

# Optional plugin libraries
foreach(_plugin ARPACK CSPARSE GL)
    string(TOLOWER ${_plugin} _plugin_lower)
    find_library(Thea_${_plugin}_LIBRARY
        NAMES "TheaPlugin${_plugin}${_THEA_LIB_SUFFIX}" "TheaPlugin${_plugin}"
        PATHS "${Thea_LIBRARY_DIR}"
        NO_DEFAULT_PATH
    )
    
    if(Thea_${_plugin}_LIBRARY)
        list(APPEND Thea_LIBRARIES ${Thea_${_plugin}_LIBRARY})
        
        if(NOT TARGET Thea::${_plugin})
            add_library(Thea::${_plugin} STATIC IMPORTED)
            set_target_properties(Thea::${_plugin} PROPERTIES
                IMPORTED_LOCATION "${Thea_${_plugin}_LIBRARY}"
                INTERFACE_LINK_LIBRARIES "Thea::Thea"
            )
        endif()
        
        set(Thea_${_plugin}_FOUND TRUE)
    else()
        set(Thea_${_plugin}_FOUND FALSE)
    endif()
endforeach()

# Check if everything was found
include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Thea
    REQUIRED_VARS Thea_LIBRARY Thea_INCLUDE_DIRS
    VERSION_VAR Thea_VERSION
    HANDLE_COMPONENTS
)

mark_as_advanced(
    Thea_INCLUDE_DIRS
    Thea_LIBRARY
    Thea_LIBRARIES
)
EOF

    # Generate TheaConfigVersion.cmake
    cat > "$cmake_dir/TheaConfigVersion${suffix}.cmake" << 'EOF'
# Thea CMake Version Configuration File

set(PACKAGE_VERSION "1.0.0")

if(PACKAGE_VERSION VERSION_LESS PACKAGE_FIND_VERSION)
    set(PACKAGE_VERSION_COMPATIBLE FALSE)
else()
    set(PACKAGE_VERSION_COMPATIBLE TRUE)
    if(PACKAGE_FIND_VERSION STREQUAL PACKAGE_VERSION)
        set(PACKAGE_VERSION_EXACT TRUE)
    endif()
endif()

# Check architecture compatibility
if(CMAKE_SIZEOF_VOID_P EQUAL 8)
    set(_THEA_ARCH "x86_64")
else()
    set(_THEA_ARCH "i686")
endif()

# This package is built for x86_64
if(NOT _THEA_ARCH STREQUAL "x86_64")
    set(PACKAGE_VERSION_COMPATIBLE FALSE)
endif()
EOF
}

# Dynamic package function for thea
# This handles the MSYS2 prefix in the package name
eval "package_${_pkgprefix}thea() {
    pkgdesc=\"A toolkit for visual computing with a focus on geometry processing (Release)\"
    local _prefix=\$(_get_prefix)
    
    _package_common \"build-release\" \"\"
    
    # Rename/adjust CMake config for release
    local cmake_dir=\"\$pkgdir\${_prefix}/lib/cmake/Thea\"
    mv \"\$cmake_dir/TheaConfig.cmake\" \"\$cmake_dir/TheaConfig.cmake.tmp\" 2>/dev/null || true
    mv \"\$cmake_dir/TheaConfigVersion.cmake\" \"\$cmake_dir/TheaConfigVersion.cmake.tmp\" 2>/dev/null || true
    
    # Create proper CMake config
    cat > \"\$cmake_dir/TheaConfig.cmake\" << EOFCMAKE
# Thea CMake Configuration File (Release)
#
# This file provides CMake configuration for the Thea library.
#
# Usage:
#   find_package(Thea REQUIRED)
#   target_link_libraries(your_target Thea::Thea)
#
# For debug builds, use:
#   find_package(Thea REQUIRED CONFIG)
#   set(CMAKE_BUILD_TYPE Debug)

include(CMakeFindDependencyMacro)
find_dependency(Eigen3 REQUIRED)

set(Thea_VERSION \"\$pkgver\")
set(Thea_INCLUDE_DIRS \"\${_prefix}/include/Thea\")
set(Thea_LIBRARY_DIR \"\${_prefix}/lib\")

# Find the main library
find_library(Thea_LIBRARY
    NAMES Thea
    PATHS \"\\\${Thea_LIBRARY_DIR}\"
)

set(Thea_LIBRARIES \\\${Thea_LIBRARY})

# Create imported target
if(NOT TARGET Thea::Thea)
    add_library(Thea::Thea STATIC IMPORTED)
    set_target_properties(Thea::Thea PROPERTIES
        IMPORTED_LOCATION \"\\\${Thea_LIBRARY}\"
        INTERFACE_INCLUDE_DIRECTORIES \"\\\${Thea_INCLUDE_DIRS}\"
        INTERFACE_LINK_LIBRARIES \"Eigen3::Eigen\"
    )
endif()

# Optional plugin libraries
foreach(_plugin ARPACK CSPARSE GL)
    find_library(Thea_\\\${_plugin}_LIBRARY
        NAMES \"TheaPlugin\\\${_plugin}\"
        PATHS \"\\\${Thea_LIBRARY_DIR}\"
    )
    
    if(Thea_\\\${_plugin}_LIBRARY)
        list(APPEND Thea_LIBRARIES \\\${Thea_\\\${_plugin}_LIBRARY})
        
        if(NOT TARGET Thea::\\\${_plugin})
            add_library(Thea::\\\${_plugin} STATIC IMPORTED)
            set_target_properties(Thea::\\\${_plugin} PROPERTIES
                IMPORTED_LOCATION \"\\\${Thea_\\\${_plugin}_LIBRARY}\"
                INTERFACE_LINK_LIBRARIES \"Thea::Thea\"
            )
        endif()
        
        set(Thea_\\\${_plugin}_FOUND TRUE)
    endif()
endforeach()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(Thea
    REQUIRED_VARS Thea_LIBRARY Thea_INCLUDE_DIRS
    VERSION_VAR Thea_VERSION
)
EOFCMAKE

    cat > \"\$cmake_dir/TheaConfigVersion.cmake\" << EOFVER
set(PACKAGE_VERSION \"\$pkgver\")

if(PACKAGE_VERSION VERSION_LESS PACKAGE_FIND_VERSION)
    set(PACKAGE_VERSION_COMPATIBLE FALSE)
else()
    set(PACKAGE_VERSION_COMPATIBLE TRUE)
    if(PACKAGE_FIND_VERSION STREQUAL PACKAGE_VERSION)
        set(PACKAGE_VERSION_EXACT TRUE)
    endif()
endif()
EOFVER

    # Remove temp files
    rm -f \"\$cmake_dir/TheaConfig.cmake.tmp\" \"\$cmake_dir/TheaConfigVersion.cmake.tmp\"
}"

# Dynamic package function for thea-debug
# This handles the MSYS2 prefix in the package name
eval "package_${_pkgprefix}thea-debug() {
    pkgdesc=\"A toolkit for visual computing with a focus on geometry processing (Debug)\"
    depends=(\"${_pkgprefix}thea\")
    local _prefix=\$(_get_prefix)
    
    cd \"\$_srcroot/build-debug\"
    
    # Only install debug libraries (with 'd' suffix)
    install -dm755 \"\$pkgdir\${_prefix}/lib\"
    
    # Find and install debug libraries
    if [[ \"\$_msys2\" == true ]]; then
        find \"\$_srcroot/Code/Build/Output/lib\" -name \"*d.a\" -exec install -Dm644 {} \"\$pkgdir\${_prefix}/lib/\" \\;
    else
        find \"\$_srcroot/Code/Build/Output/lib\" -name \"*d.a\" -exec install -Dm644 {} \"\$pkgdir\${_prefix}/lib/\" \\;
    fi
    
    # Install debug CMake config
    local cmake_dir=\"\$pkgdir\${_prefix}/lib/cmake/TheaDebug\"
    install -dm755 \"\$cmake_dir\"
    
    cat > \"\$cmake_dir/TheaDebugConfig.cmake\" << EOFCMAKE
# Thea CMake Configuration File (Debug)
#
# This file provides CMake configuration for the Thea library debug build.
#
# Usage:
#   find_package(TheaDebug REQUIRED)
#   target_link_libraries(your_target Thea::TheaDebug)

include(CMakeFindDependencyMacro)
find_dependency(Eigen3 REQUIRED)

set(TheaDebug_VERSION \"\$pkgver\")
set(TheaDebug_INCLUDE_DIRS \"\${_prefix}/include/Thea\")
set(TheaDebug_LIBRARY_DIR \"\${_prefix}/lib\")

# Find the debug library
find_library(TheaDebug_LIBRARY
    NAMES Thead
    PATHS \"\\\${TheaDebug_LIBRARY_DIR}\"
)

set(TheaDebug_LIBRARIES \\\${TheaDebug_LIBRARY})

# Create imported target
if(NOT TARGET Thea::TheaDebug)
    add_library(Thea::TheaDebug STATIC IMPORTED)
    set_target_properties(Thea::TheaDebug PROPERTIES
        IMPORTED_LOCATION \"\\\${TheaDebug_LIBRARY}\"
        INTERFACE_INCLUDE_DIRECTORIES \"\\\${TheaDebug_INCLUDE_DIRS}\"
        INTERFACE_LINK_LIBRARIES \"Eigen3::Eigen\"
        INTERFACE_COMPILE_DEFINITIONS \"DEBUG;_DEBUG\"
    )
endif()

# Optional plugin debug libraries
foreach(_plugin ARPACK CSPARSE GL)
    find_library(TheaDebug_\\\${_plugin}_LIBRARY
        NAMES \"TheaPlugin\\\${_plugin}d\"
        PATHS \"\\\${TheaDebug_LIBRARY_DIR}\"
    )
    
    if(TheaDebug_\\\${_plugin}_LIBRARY)
        list(APPEND TheaDebug_LIBRARIES \\\${TheaDebug_\\\${_plugin}_LIBRARY})
        
        if(NOT TARGET Thea::\\\${_plugin}Debug)
            add_library(Thea::\\\${_plugin}Debug STATIC IMPORTED)
            set_target_properties(Thea::\\\${_plugin}Debug PROPERTIES
                IMPORTED_LOCATION \"\\\${TheaDebug_\\\${_plugin}_LIBRARY}\"
                INTERFACE_LINK_LIBRARIES \"Thea::TheaDebug\"
            )
        endif()
        
        set(TheaDebug_\\\${_plugin}_FOUND TRUE)
    endif()
endforeach()

include(FindPackageHandleStandardArgs)
find_package_handle_standard_args(TheaDebug
    REQUIRED_VARS TheaDebug_LIBRARY TheaDebug_INCLUDE_DIRS
    VERSION_VAR TheaDebug_VERSION
)
EOFCMAKE

    cat > \"\$cmake_dir/TheaDebugConfigVersion.cmake\" << EOFVER
set(PACKAGE_VERSION \"\$pkgver\")

if(PACKAGE_VERSION VERSION_LESS PACKAGE_FIND_VERSION)
    set(PACKAGE_VERSION_COMPATIBLE FALSE)
else()
    set(PACKAGE_VERSION_COMPATIBLE TRUE)
    if(PACKAGE_FIND_VERSION STREQUAL PACKAGE_VERSION)
        set(PACKAGE_VERSION_EXACT TRUE)
    endif()
endif()
EOFVER

    # Install license
    install -Dm644 \"\$_srcroot/LICENSE.txt\" \"\$pkgdir\${_prefix}/share/licenses/\${pkgname}/LICENSE\"
}"
