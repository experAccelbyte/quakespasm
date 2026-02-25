
####### Expanded from @PACKAGE_INIT@ by configure_package_config_file() #######
####### Any changes to this file will be overwritten by the next CMake run ####
####### The input file was accelbyte-gamesdkConfig.cmake.in                            ########

get_filename_component(PACKAGE_PREFIX_DIR "${CMAKE_CURRENT_LIST_DIR}/../../../" ABSOLUTE)

macro(set_and_check _var _file)
  set(${_var} "${_file}")
  if(NOT EXISTS "${_file}")
    message(FATAL_ERROR "File or directory ${_file} referenced by variable ${_var} does not exist !")
  endif()
endmacro()

macro(check_required_components _NAME)
  foreach(comp ${${_NAME}_FIND_COMPONENTS})
    if(NOT ${_NAME}_${comp}_FOUND)
      if(${_NAME}_FIND_REQUIRED_${comp})
        set(${_NAME}_FOUND FALSE)
      endif()
    endif()
  endforeach()
endmacro()

####################################################################################

get_filename_component(_ACCELBYTE_GAMESDK_ROOT "${CMAKE_CURRENT_LIST_DIR}/../../.." ABSOLUTE)

# Resolve the consumer OS and platform-specific library conventions.
if(WIN32)
  set(_AB_SDK_LIB_PREFIX "")
else()
  set(_AB_SDK_LIB_PREFIX "lib")
endif()


# Helper macro to create IMPORTED shared library targets for bundled dependencies
macro(_accelbyte_create_imported_target _target_name _lib_name)
  if(NOT TARGET ${_target_name})
    add_library(${_target_name} SHARED IMPORTED)
    set_target_properties(${_target_name} PROPERTIES
      INTERFACE_INCLUDE_DIRECTORIES "${_ACCELBYTE_GAMESDK_ROOT}/include"
    )
  endif()
  if(WIN32)
    set_target_properties(${_target_name} PROPERTIES
      IMPORTED_IMPLIB   "${_ACCELBYTE_GAMESDK_ROOT}/lib/${_lib_name}.lib"
      IMPORTED_LOCATION "${_ACCELBYTE_GAMESDK_ROOT}/bin/${_lib_name}.dll"
    )
  elseif(APPLE)
    set_target_properties(${_target_name} PROPERTIES
      IMPORTED_LOCATION "${_ACCELBYTE_GAMESDK_ROOT}/lib/${_AB_SDK_LIB_PREFIX}${_lib_name}.dylib"
    )
  else()
    set_target_properties(${_target_name} PROPERTIES
      INTERFACE_COMPILE_DEFINITIONS "_GLIBCXX_USE_CXX11_ABI=0"
    )
    set_target_properties(${_target_name} PROPERTIES
      IMPORTED_LOCATION "${_ACCELBYTE_GAMESDK_ROOT}/lib/${_AB_SDK_LIB_PREFIX}${_lib_name}.so"
      IMPORTED_SONAME   "${_AB_SDK_LIB_PREFIX}${_lib_name}.so"
      )
  endif()
endmacro()

# Create IMPORTED targets for abcnl dependencies
_accelbyte_create_imported_target(abcnl::common "abcnl-common")
_accelbyte_create_imported_target(abcnl::memory "abcnl-memory")
_accelbyte_create_imported_target(abcnl::http "abcnl-http")
_accelbyte_create_imported_target(abcnl::curl_http_executor "abcnl-curl_http_executor")
_accelbyte_create_imported_target(abcnl::utils "abcnl-utils")
_accelbyte_create_imported_target(abcnl::tls "abcnl-tls")
_accelbyte_create_imported_target(abcnl::web_socket "abcnl-web_socket")
_accelbyte_create_imported_target(abcnl::platform_connection "abcnl-platform_connection")
_accelbyte_create_imported_target(abcnl::p2p_connection "abcnl-p2p_connection")
_accelbyte_create_imported_target(abcnl::network_utility "abcnl-network_utility")
_accelbyte_create_imported_target(abcnl::logger "abcnl-logger")

# Create IMPORTED targets for abcnl-codegen dependencies
_accelbyte_create_imported_target(abcnl-codegen::iam "abcnl-iam")
_accelbyte_create_imported_target(abcnl-codegen::cloudsave "abcnl-cloudsave")
_accelbyte_create_imported_target(abcnl-codegen::turn_manager "abcnl-turn_manager")
_accelbyte_create_imported_target(abcnl-codegen::chat "abcnl-chat")
_accelbyte_create_imported_target(abcnl-codegen::lobby "abcnl-lobby")
_accelbyte_create_imported_target(abcnl-codegen::achievement "abcnl-achievement")
_accelbyte_create_imported_target(abcnl-codegen::audit "abcnl-audit")
_accelbyte_create_imported_target(abcnl-codegen::basic "abcnl-basic")
_accelbyte_create_imported_target(abcnl-codegen::session "abcnl-session")
_accelbyte_create_imported_target(abcnl-codegen::match2 "abcnl-match2")
_accelbyte_create_imported_target(abcnl-codegen::group "abcnl-group")
_accelbyte_create_imported_target(abcnl-codegen::social "abcnl-social")
_accelbyte_create_imported_target(abcnl-codegen::ugc "abcnl-ugc")
_accelbyte_create_imported_target(abcnl-codegen::leaderboard "abcnl-leaderboard")
_accelbyte_create_imported_target(abcnl-codegen::legal "abcnl-legal")
_accelbyte_create_imported_target(abcnl-codegen::login_queue "abcnl-login_queue")
_accelbyte_create_imported_target(abcnl-codegen::reporting "abcnl-reporting")
_accelbyte_create_imported_target(abcnl-codegen::platform "abcnl-platform")
_accelbyte_create_imported_target(abcnl-codegen::gametelemetry "abcnl-gametelemetry")
_accelbyte_create_imported_target(abcnl-codegen::gdpr "abcnl-gdpr")
_accelbyte_create_imported_target(abcnl-codegen::challenge "abcnl-challenge")
_accelbyte_create_imported_target(abcnl-codegen::inventory "abcnl-inventory")
_accelbyte_create_imported_target(abcnl-codegen::seasonpass "abcnl-seasonpass")

# Create IMPORTED target for JsonCpp (bundled)
if(NOT TARGET JsonCpp::JsonCpp)
  add_library(JsonCpp::JsonCpp INTERFACE IMPORTED)
  # JsonCpp is statically linked into the SDK, no additional linking needed
endif()

# Create IMPORTED target for MbedTLS (bundled)
if(NOT TARGET MbedTLS::mbedtls)
  add_library(MbedTLS::mbedtls INTERFACE IMPORTED)
  # MbedTLS is statically linked into the SDK, no additional linking needed
endif()

include("${CMAKE_CURRENT_LIST_DIR}/accelbyte-gamesdkTargets.cmake")

check_required_components(accelbyte-gamesdk)
