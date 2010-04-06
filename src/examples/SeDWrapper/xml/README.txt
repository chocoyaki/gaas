### EXAMPLE OF SeDWrapper

## Date: March 2010
## Version: 1.0
## Author: Benjamin Isnard (benjamin.isnard@ens-lyon.fr)

### FILES
- convert.gasw: description of the 'convert' executable (see 'man convert')
- convert_dag.xml: dag workflow description to test the "convert" SeD

### HOWTO
- launch a DIET hierarchy with two SeDs:
	1/ the "convert" SeD that uses the GASWServer binary with the convert.gasw description as parameter
	2/ the "file_manip" SeD available in the examples/workflow directory
- launch the workflow client in $DIET_INSTALL_DIR/bin/examples/workflow/client_file with the dag workflow description as parameter

