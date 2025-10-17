include(FetchContent)

function(istudio_enable_warnings target)
  if(MSVC)
    target_compile_options(${target} PRIVATE /W4 $<$<BOOL:${ISTUDIO_ENABLE_WARNINGS_AS_ERRORS}>:/WX>)
  else()
    target_compile_options(
      ${target}
      PRIVATE
        -Wall
        -Wextra
        -Wpedantic
        -Wconversion
        -Wsign-conversion
        -Wold-style-cast
        $<$<BOOL:${ISTUDIO_ENABLE_WARNINGS_AS_ERRORS}>:-Werror>
    )
  endif()
endfunction()

function(istudio_fetch_fmt)
  if(TARGET fmt::fmt)
    return()
  endif()

  FetchContent_Declare(
    fmt
    GIT_REPOSITORY https://github.com/fmtlib/fmt.git
    GIT_TAG 10.2.1
    GIT_SHALLOW ON
  )
  set(FMT_DOC OFF CACHE BOOL "" FORCE)
  set(FMT_TEST OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(fmt)
endfunction()

function(istudio_fetch_gtest)
  if(TARGET GTest::gtest)
    return()
  endif()

  FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.14.0
    GIT_SHALLOW ON
  )
  set(INSTALL_GTEST OFF CACHE BOOL "" FORCE)
  FetchContent_MakeAvailable(googletest)
endfunction()
