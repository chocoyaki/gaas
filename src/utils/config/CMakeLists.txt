# internal DIET configuration parsers
set( config_SRCS CmdParser.cc
  CommonParser.cc
  EnvParser.cc
  FileParser.cc
  constants.cc 
  configuration.cc )

# static because we don't ship this
add_library( DietConfig STATIC ${config_SRCS} )