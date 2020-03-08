CMAKE_MINIMUM_REQUIRED ( VERSION 3.0 )
PROJECT ( BEngine )

# option(Windwos "MyOption" OFF)

if (Windwos)
  # Make window visual studio project
  add_executable( engine 
       src/main.cpp)
elseif(Android)
  # Make android studio project
 add_library( engine
     # Sets the library as a shared library.
     #SHARED
     # Provides a relative path to your source file(s).
     src/main.cpp )

endif()
