file(REMOVE_RECURSE
  "../pathtracer.pdb"
  "../pathtracer"
)

# Per-language clean rules from dependency scanning.
foreach(lang )
  include(CMakeFiles/pathtracer.dir/cmake_clean_${lang}.cmake OPTIONAL)
endforeach()
