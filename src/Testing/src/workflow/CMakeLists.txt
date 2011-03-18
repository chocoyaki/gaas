INCLUDE_DIRECTORIES( ${DIET_SOURCE_DIR}/src/utils/DAGDA )

if (DIET_USE_WORKFLOW) 
  diet_test( DIETDataInoutSinkScalarsTests )
endif (DIET_USE_WORKFLOW)
