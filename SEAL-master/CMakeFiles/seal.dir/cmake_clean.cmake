file(REMOVE_RECURSE
  "lib/libseal-3.5.a"
  "lib/libseal-3.5.pdb"
)

# Per-language clean rules from dependency scanning.
foreach(lang C CXX)
  include(CMakeFiles/seal.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
