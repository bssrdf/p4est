/*
  This file is part of p4est.
  p4est is a C library to manage a parallel collection of quadtrees and/or
  octrees.

  Copyright (C) 2007 Carsten Burstedde, Lucas Wilcox.

  This program is free software: you can redistribute it and/or modify
  it under the terms of the GNU General Public License as published by
  the Free Software Foundation, either version 3 of the License, or
  (at your option) any later version.

  This program is distributed in the hope that it will be useful,
  but WITHOUT ANY WARRANTY; without even the implied warranty of
  MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
  GNU General Public License for more details.

  You should have received a copy of the GNU General Public License
  along with this program.  If not, see <http://www.gnu.org/licenses/>.
*/

#include <p4est.h>
#include <p4est_algorithms.h>
#include <p4est_base.h>
#include <p4est_file.h>

#include <unistd.h>

typedef struct
{
  int32_t             a;
}
user_data_t;

static void
init_fn (p4est_t * p4est, int32_t which_tree, p4est_quadrant_t * quadrant)
{
  user_data_t        *data = quadrant->user_data;

  data->a = which_tree;
}

int
main (int argc, char **argv)
{
  int                 retval;
  int                 rank = 0;

#ifdef HAVE_MPI
  int                 use_mpi = 1;
  int                 mpiret;
#endif
  MPI_Comm            mpicomm;
  p4est_t            *p4est;
  p4est_connectivity_t *connectivity;

  mpicomm = MPI_COMM_NULL;
#ifdef HAVE_MPI
  if (use_mpi) {
    mpiret = MPI_Init (&argc, &argv);
    P4EST_CHECK_MPI (mpiret);
    mpicomm = MPI_COMM_WORLD;
  }
#endif

  char                template[] = "p4est_meshXXXXXX";
  char                mesh[] = "		[Forest Info] # ]] [[ ]]\n"
    "ver = 0.0.1  # Version of the forest file\n"
    "Nk  = 3      # Number of elements\n"
    "Nv  = 7      # Number of mesh vertices\n"
    "Net = 0      # Number of element tags\n"
    "Nft = 0      # Number of face tags\n"
    "Ncf = 0      # Number of curved faces\n"
    "Nct = 0      # Number of curved types\n"
    "\n"
    "                          [Coordinates of Element Vertices]\n"
    "1 -1.00000000000e+00 -1.00000000000e+00  0.00000000000e+00\n"
    "2  0.00000000000e+00 -1.00000000000e+00  0.00000000000e+00\n"
    "3  0.00000000000e+00  0.00000000000e+00  0.00000000000e+00\n"
    "4  1.00000000000e+00  0.00000000000e+00  0.00000000000e+00\n"
    "5  1.00000000000e+00  1.00000000000e+00  0.00000000000e+00\n"
    "6  0.00000000000e+00  1.00000000000e+00  0.00000000000e+00\n"
    "7 -1.00000000000e+00  0.00000000000e+00  0.00000000000e+00\n"
    "   [Element to Vertex]\n"
    "1     1   2   4   3\n"
    "2     1   3   6   7\n"
    "3     3   4   5   6\n"
    "  [Element to Element]\n"
    "1     1   1   3   2\n"
    "2     1   3   2   2\n"
    "3     1   3   3   2\n"
    "\n"
    "[Element to Face]\n"
    "1     1   2   1   1\n"
    "2     4   4   3   4\n"
    "3     3   2   3   2\n"
    "\n"
    "[Element Tags]\n" "[Face Tags]\n" "[Curved Faces]\n" "[Curved Types]\n";

  int                 fd;
  size_t              meshlength;
  size_t              templatelength;

#ifdef HAVE_MPI
  mpiret = MPI_Comm_rank (mpicomm, &rank);
  P4EST_CHECK_MPI (mpiret);
#endif

  if (rank == 0) {
    /* Make a temporary file to hold the mesh */
    fd = mkstemp (template);
    P4EST_CHECK_ABORT (fd != -1, "Unable to open temp mesh file.");

    /* Write out to the mesh to the temporary file */
    meshlength = strlen (mesh);
    retval = write (fd, mesh, meshlength);
    P4EST_CHECK_ABORT (retval != -1, "Unable to write to temp mesh file.");

    /* Close the temporary file */
    retval = close (fd);
    P4EST_CHECK_ABORT (!retval, "Unable to close the temp mesh file.");
  }

#ifdef HAVE_MPI
  templatelength = strlen (template);
  mpiret = MPI_Bcast (template, templatelength + 1, MPI_CHAR, 0, mpicomm);
  P4EST_CHECK_MPI (mpiret);
#endif

  /* Read in the mesh into connectivity information */
  retval = p4est_connectivity_read (template, &connectivity);
  P4EST_CHECK_ABORT (!retval, "Unable to read the mesh file.");

  /* Print the connectivity */

  if (rank == 0) {
    p4est_connectivity_print (connectivity, stdout);
  }

  p4est = p4est_new (mpicomm, stdout, connectivity,
                     sizeof (user_data_t), init_fn);
  p4est_tree_print (p4est_array_index (p4est->trees, 0),
                    p4est->mpirank, stdout);

  /* destroy the p4est and its connectivity structure */
  p4est_destroy (p4est);
  p4est_connectivity_destroy (connectivity);

  if (rank == 0) {
    /* unlink the temporary file */
    retval = unlink (template);
    P4EST_CHECK_ABORT (!retval, "Unable to close the temp mesh file.");
  }

  /* clean up and exit */
  p4est_memory_check ();

#ifdef HAVE_MPI
  if (use_mpi) {
    mpiret = MPI_Finalize ();
    P4EST_CHECK_MPI (mpiret);
  }
#endif

  return 0;
}

/* EOF read_forest.c */