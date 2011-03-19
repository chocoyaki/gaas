INCLUDE_DIRECTORIES( ${DIET_SOURCE_DIR}/src/utils/DAGDA )

if (DIET_USE_WORKFLOW) 
  diet_test( DIETDataInoutSinkScalarsTests )
#  diet_test( DIETDataInoutSinkVectorsTests )
  diet_test( DIETDataInoutSinkStringsTests )
  diet_test( DIETDataInoutSinkPStringsTests )
endif (DIET_USE_WORKFLOW)
