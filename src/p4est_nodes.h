/*
  This file is part of p4est.
  p4est is a C library to manage a parallel collection of quadtrees and/or
  octrees.

  Copyright (C) 2007,2008 Carsten Burstedde, Lucas Wilcox.

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

#ifndef P4EST_NODES_H
#define P4EST_NODES_H

#include <p4est.h>
#include <p4est_ghost.h>

SC_EXTERN_C_BEGIN;

/** Store an independent node.
 * Keep this in sync with the p4est_t data structure.
 */
typedef struct p4est_indep
{
  p4est_qcoord_t      x, y;
  int8_t              level, pad8;
  int16_t             pad16;
  union p4est_indep_data
  {
    void               *unused;
    p4est_topidx_t      which_tree;
    struct
    {
      p4est_topidx_t      which_tree;
      int                 owner_rank;
    }
    piggy1;
    struct
    {
      p4est_topidx_t      which_tree;
      p4est_topidx_t      from_tree;
    }
    piggy_unused2;
    struct
    {
      p4est_topidx_t      which_tree;
      p4est_locidx_t      local_num;
    }
    piggy3;
  }
  p;
}
p4est_indep_t;

/** Store a hanging node that depends on two independent nodes.
 * Keep this in sync with the p4est_t data structure.
 */
typedef struct p4est_hang2
{
  p4est_qcoord_t      x, y;
  int8_t              level, pad8;
  int16_t             pad16;
  union p4est_hang2_data
  {
    void               *unused;
    p4est_topidx_t      which_tree;
    struct
    {
      p4est_topidx_t      which_tree;
      int                 owner_rank;
    }
    piggy_unused1;
    struct
    {
      p4est_topidx_t      which_tree;
      p4est_topidx_t      from_tree;
    }
    piggy_unused2;
    struct
    {
      p4est_topidx_t      which_tree;
      p4est_locidx_t      local_num;
    }
    piggy_unused3;
    struct
    {
      p4est_topidx_t      which_tree;
      p4est_locidx_t      depends[2];
    }
    piggy;
  }
  p;
}
p4est_hang2_t;

/** This structure holds complete parallel node information.
 *
 * Nodes are unique and either independent or face hanging.
 * Independent nodes store their owner's tree id in piggy3.which_tree.
 * The index in their owner's ordering is stored in piggy3.local_num.
 * Hanging nodes store their owner's tree id in piggy.which_tree.
 * The numbers of their associated independent nodes are in piggy.depends[].
 *
 * The local_nodes table is of dimension 4 * num_local_quadrants
 * and encodes the node indexes for all corners of all quadrants.  Let
 * ni := indep_nodes.elem_count,
 * fi := face_hangings.elem_count,
 * If for l := local_nodes[k]
 * l >= 0 && l < ni: l indexes into indep_nodes.
 * l >= ni && l < ni + fi: l - ni indexes into face_hangings.
 * No other values for l are permitted.
 *
 * The array shared_indeps holds lists of node sharers (not including rank).
 * The entry shared_indeps[i] is of type sc_recycle_array_t
 * and holds the list of nodes with i + 1 sharers.
 * For each independent node, its member pad8 holds the number of sharers
 * and its member pad16 holds the position in the assigned recycle array
 * if this number fits into an int16_t.  If this limit is exceeded, the
 * array shared_offsets is filled with these positions as one p4est_locidx_t
 * per independent node, and all pad16 members are set to -1.  To recognize
 * the latter situation you can check for shared_offsets != NULL.
 *
 * Each processor owns num_owned_indeps of the stored independent nodes.
 * The first independent owned node is at index offset_owned_indeps.
 * The table nonlocal_ranks contains the ranks of all stored non-owned nodes.
 * The table global_owned_nodes holds the number of owned nodes for each rank.
 */
typedef struct p4est_nodes
{
  p4est_locidx_t      num_local_quadrants;
  p4est_locidx_t      num_owned_indeps, num_owned_shared;
  p4est_locidx_t      offset_owned_indeps;
  sc_array_t          indep_nodes;
  sc_array_t          face_hangings;
  p4est_locidx_t     *local_nodes;
  sc_array_t          shared_indeps;
  p4est_locidx_t     *shared_offsets;
  int                *nonlocal_ranks;
  p4est_locidx_t     *global_owned_indeps;
}
p4est_nodes_t;

/** Determine unique ordering of vertices for each quadrant.
 *
 * \param [in]  p4est              The forest whose vertices will be ordered.
 * \param [in]  identify_periodic  Boolean flag to switch on periodic b.c.
 * \param [out] num_uniq_local_vertices will be filled with the total number
 *                                      of unique vertices.
 * \param [out] quadrant_to_local_vertex an array that for each vertex of each
 *                                       quadrant holds the value of the
 *                                       corresponding unique local vertex.
 *                                       The array needs to be size (total
 *                                       number of local quadrants x 4).  It
 *                                       will be filled
 *                                       [0][0]..[0][3]..
 *                                       [num_local_quads-1][0]..
 *                                       [num_local_quads-1][3]
 */
void                p4est_order_local_vertices (p4est_t * p4est,
                                                bool identify_periodic,
                                                p4est_locidx_t *
                                                num_uniq_local_vertices,
                                                p4est_locidx_t *
                                                quadrant_to_local_vertex);

/** Create node information. */
p4est_nodes_t      *p4est_nodes_new (p4est_t * p4est, p4est_ghost_t * ghost);

/** Destroy node information. */
void                p4est_nodes_destroy (p4est_nodes_t * nodes);

/** Check node information for internal consistency. */
bool                p4est_nodes_is_valid (p4est_t * p4est,
                                          p4est_nodes_t * nodes);

SC_EXTERN_C_END;

#endif /* !P4EST_NODES_H */
