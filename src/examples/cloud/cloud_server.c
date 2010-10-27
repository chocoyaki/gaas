/****************************************************************************/
/* Example client for the Cloud Batch System                                */
/*                                                                          */
/* Author(s):                                                               */
/*    - Adrian Muresan (adrian.muresan@ens-lyon.fr)                         */
/*                                                                          */
/* $LICENSE$                                                                */
/****************************************************************************/
/* $Id$
 * $Log$
 * Revision 1.4  2010/10/27 10:53:00  amuresan
 * modified cloud examples to take config files as command line args
 *
 * Revision 1.3  2010/10/27 06:56:30  bdepardo
 * Solved compilation problems
 *
 * Revision 1.2  2010/10/27 06:41:25  amuresan
 * modified Eucalyptus_BatchSystem to be able to use existing VMs also
 *
 * Revision 1.1  2010/08/23 09:44:28  amuresan
 * Fixed cmake conflicting target problem
 *
 * Revision 1.2  2010/06/14 21:27:24  amuresan
 * Added header to cloud example files and client config file.
 *
 *****************************************************************************/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#include "DIET_server.h"

void usage(char* me)
{
    fprintf(stderr, "Usage: %s <file.cfg>\n"
            "\t ex: %s SeD_cloud.cfg\n",
            me, me);
}

char args(int argc, char**argv)
{
    if(argc <= 1)
    {
        usage(argv[0]);
        return 0;
    }
    return 1;
}

void make_perf(diet_profile_t *pb)
{
      pb->walltime = 125 ;
      pb->nbprocs = 2 ; 
      pb->nbprocess = pb->nbprocs ;
}

void mat_to_str(diet_profile_t*pb,int index, char*cumul)
{
  size_t m, n;
  diet_matrix_order_t o;
  char nb[100];
  double* M = NULL;
  int i, j;

  diet_matrix_get(diet_parameter(pb, index), &M, NULL, &m, &n, &o);
  strcat(cumul, "[");
  for(i=0;i<m;i++)
  {
      strcat(cumul, "[");
      for(j=0;j<n;j++)
      {
          sprintf(nb, "%.2lf", M[i*n+j]);
          strcat(cumul, nb);
          if(j < n - 1)
              strcat(cumul, ", ");
      }
      if(i < m - 1)
          strcat(cumul, "], ");
      else
          strcat(cumul, "]");
  }
  strcat(cumul, "]");
}

void read_all(char*buff, const char*in_file)
{
    char aux[500];
    /* Collect the result */
    FILE*fi = fopen(in_file, "r");
    while(fi != NULL && !feof(fi))
    {
	    fgets(aux, 500, fi);
        if(!feof(fi))
            strcat(buff, aux);
    }
    if(fi != NULL)
        fclose(fi);
}

int solve_cloud(diet_profile_t *pb)
{
    char * result = (char*)malloc(9000 * sizeof(char));
    char * C = (char*)malloc(9000 * sizeof(char));
    int res;

    char*aux, *strA, *strB;
    char*script_start =
        "#!/bin/bash\n\n"
        "for h in $DIET_CLOUD_VMS\n"
        "do\n"
        "ssh ec2-user@$h -i $PATH_TO_SSH_KEY -o StrictHostKeyChecking=no 'hostname ; uname -a ; ps aux ; export ; ls /' > info\n"

        "ssh ec2-user@$h -i $PATH_TO_SSH_KEY -o StrictHostKeyChecking=no '"
        "echo \""
        "def zero(m,n):\n"
        "    return [[0 for r in range(n)] for c in range(m)]\n"
        "def mult(m1,m2):\n"
        "    # Matrix multiplication\n"
        "    if len(m1[0]) != len(m2):\n"
        "        # Check matrix dimensions\n"
        "        print -1\n"
        "    else:\n"
        "        # Multiply if correct dimensions\n"
        "        r = zero(len(m1),len(m2[0]))\n"
        "        for i in range(len(m1)):\n"
        "            for j in range(len(m2[0])):\n"
        "                for k in range(len(m2)):\n"
        "                    r[i][j] += m1[i][k]*m2[k][j]\n"
        "        return r\n";

    char*script_end =
        "print mult(a, b)"
        "\" > mult.py\n"
        "python mult.py' > mult\n"
        "done";


    aux = (char*)malloc(9000*sizeof(char));
    strA = (char*)malloc(500*sizeof(char));
    strA[0] = 0;
    strB = (char*)malloc(500*sizeof(char));
    strB[0] = 0;
    mat_to_str(pb, 0, strA);
    mat_to_str(pb, 1, strB);
    /*    printf("A: %s\nB: %s\n", strA, strB); */
    sprintf(aux, "%sa=%s\nb=%s\n%s\0", script_start, strA, strB, script_end);

  /* Call performance prediction or not, but fields are to be fullfilled */ 
  make_perf(pb) ; 
         
  /* Submission */ 
  res = diet_submit_parallel(pb, 
			     NULL,
			     aux) ;

  /*  printf("submitted script '%s' and got result %d\n", aux, res); */
  result[0] = '\0';

  read_all(result, "info");
  printf("Info result: %s", result);
  read_all(C, "mult");
  printf("MatMult result: %s", C);

  /* Set OUT arguments */
  diet_string_set(diet_parameter(pb, 2), C, DIET_VOLATILE);
  diet_string_set(diet_parameter(pb, 3), result, DIET_VOLATILE);
  return 0;
}
int main(int argc, char* argv[])
{
  if(!args(argc, argv))
    return 1;
  diet_profile_desc_t *profile;
  diet_set_server_status( BATCH ) ;
  
  /* Initialize table with maximum 1 service */
  diet_service_table_init(1);
  
  /* Define smprod profile */
  profile = diet_profile_desc_alloc("cloud-demo", 1, 1, 3);
  diet_profile_desc_set_parallel(profile);

  /* info string */
  diet_generic_desc_set(diet_param_desc(profile,3), DIET_STRING, DIET_CHAR);
  
  /* mat-prod params */
  diet_generic_desc_set(diet_param_desc(profile,0), DIET_MATRIX, DIET_DOUBLE);
  diet_generic_desc_set(diet_param_desc(profile,1), DIET_MATRIX, DIET_DOUBLE);
  diet_generic_desc_set(diet_param_desc(profile,2), DIET_STRING, DIET_CHAR);
  
  /* Add the service (the profile descriptor is deep copied) */
  diet_service_table_add(profile, NULL, solve_cloud);
  
  /* Free the profile descriptor, since it was deep copied. */
  diet_profile_desc_free(profile);
  
  /* Launch the SeD: no return call */
  diet_SeD(argv[1], argc, argv);
  
  /* Dead code */
  return 0;
}

