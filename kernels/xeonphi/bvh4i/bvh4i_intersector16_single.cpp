// ======================================================================== //
// Copyright 2009-2013 Intel Corporation                                    //
//                                                                          //
// Licensed under the Apache License, Version 2.0 (the "License");          //
// you may not use this file except in compliance with the License.         //
// You may obtain a copy of the License at                                  //
//                                                                          //
//     http://www.apache.org/licenses/LICENSE-2.0                           //
//                                                                          //
// Unless required by applicable law or agreed to in writing, software      //
// distributed under the License is distributed on an "AS IS" BASIS,        //
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied. //
// See the License for the specific language governing permissions and      //
// limitations under the License.                                           //
// ======================================================================== //

#include "bvh4i_intersector16_single.h"

namespace embree
{
  namespace isa
  {

    static unsigned int BVH4I_LEAF_MASK = BVH4i::leaf_mask; // needed due to compiler efficiency bug
    static unsigned int M_LANE_7777 = 0x7777;               // needed due to compiler efficiency bug
    static __aligned(64) int zlc4[4] = {0xffffffff,0xffffffff,0xffffffff,0};

    // ============================================================================================
    // ============================================================================================
    // ============================================================================================

    struct Triangle1LeafIntersector
    {
      static __forceinline bool intersect(BVH4i::NodeRef curNode,
					  const size_t rayIndex, 
					  const mic_f &dir_xyz,
					  const mic_f &org_xyz,
					  const mic_f &min_dist_xyz,
					  mic_f &max_dist_xyz,
					  Ray16& ray16, 
					  const void *__restrict__ const accel,
					  const Scene*__restrict__ const geometry)
      {
	const Triangle1* __restrict__ const tptr  = (Triangle1*) curNode.leaf(accel);	      
	const mic_i and_mask = broadcast4to16i(zlc4);
	return Triangle1Intersector16MoellerTrumbore::intersect1(rayIndex,
								 dir_xyz,
								 org_xyz,
								 min_dist_xyz,
								 max_dist_xyz,
								 and_mask,
								 ray16,
								 geometry,
								 tptr);	
      }


      static __forceinline bool occluded(BVH4i::NodeRef curNode,
					 const size_t rayIndex, 
					 const mic_f &dir_xyz,
					 const mic_f &org_xyz,
					 const mic_f &min_dist_xyz,
					 const mic_f &max_dist_xyz,
					 const Ray16& ray16, 
					 mic_m &m_terminated,
					 const void *__restrict__ const accel,
					 const Scene*__restrict__ const geometry)
      {
	const Triangle1* __restrict__ const tptr  = (Triangle1*) curNode.leaf(accel);	      
	const mic_i and_mask = broadcast4to16i(zlc4);
	return Triangle1Intersector16MoellerTrumbore::occluded1(rayIndex,
								dir_xyz,
								org_xyz,
								min_dist_xyz,
								max_dist_xyz,
								and_mask,
								ray16,
								m_terminated,
								geometry,
								tptr);	
      }

    };


    // ============================================================================================
    // ============================================================================================
    // ============================================================================================

    struct Triangle1mcLeafIntersector
    {
      static __forceinline bool intersect(BVH4i::NodeRef curNode,
					  const size_t rayIndex, 
					  const mic_f &dir_xyz,
					  const mic_f &org_xyz,
					  const mic_f &min_dist_xyz,
					  mic_f &max_dist_xyz,
					  Ray16& ray16, 
					  const void *__restrict__ const accel,
					  const Scene*__restrict__ const geometry)
      {
	unsigned int items = curNode.items();
	unsigned int index = curNode.offsetIndex();
	const Triangle1mc *__restrict__ const tptr = (Triangle1mc*)accel + index;

	const mic_i and_mask = broadcast4to16i(zlc4);
	return Triangle1mcIntersector16MoellerTrumbore::intersect1(rayIndex,
								   dir_xyz,
								   org_xyz,
								   min_dist_xyz,
								   max_dist_xyz,
								   and_mask,
								   ray16,
								   geometry,
								   tptr);	
      }


      static __forceinline bool occluded(BVH4i::NodeRef curNode,
					 const size_t rayIndex, 
					 const mic_f &dir_xyz,
					 const mic_f &org_xyz,
					 const mic_f &min_dist_xyz,
					 const mic_f &max_dist_xyz,
					 const Ray16& ray16, 
					 mic_m &m_terminated,
					 const void *__restrict__ const accel,
					 const Scene*__restrict__ const geometry)
      {
	unsigned int items = curNode.items();
	unsigned int index = curNode.offsetIndex();
	const Triangle1mc *__restrict__ const tptr = (Triangle1mc*)accel + index;

	const mic_i and_mask = broadcast4to16i(zlc4);
	return Triangle1mcIntersector16MoellerTrumbore::occluded1(rayIndex,
								  dir_xyz,
								  org_xyz,
								  min_dist_xyz,
								  max_dist_xyz,
								  and_mask,
								  ray16,
								  m_terminated,
								  geometry,
								  tptr);	
      }

    };

    // ============================================================================================
    // ============================================================================================
    // ============================================================================================

    template<typename LeafIntersector>
    void BVH4iIntersector16Single<LeafIntersector>::intersect(mic_i* valid_i, BVH4i* bvh, Ray16& ray16)
    {
      /* near and node stack */
      __aligned(64) float   stack_dist[3*BVH4i::maxDepth+1];
      __aligned(64) NodeRef stack_node[3*BVH4i::maxDepth+1];

      /* setup */
      const mic_m m_valid    = *(mic_i*)valid_i != mic_i(0);
      const mic3f rdir16     = rcp_safe(ray16.dir);
      const mic_f inf        = mic_f(pos_inf);
      const mic_f zero       = mic_f::zero();

      store16f(stack_dist,inf);

      const Node      * __restrict__ nodes = (Node     *)bvh->nodePtr();
      const Triangle1 * __restrict__ accel = (Triangle1*)bvh->triPtr();

      stack_node[0] = BVH4i::invalidNode;
      long rayIndex = -1;
      while((rayIndex = bitscan64(rayIndex,toInt(m_valid))) != BITSCAN_NO_BIT_SET_64)	    
        {
	  stack_node[1] = bvh->root;
	  size_t sindex = 2;

	  const mic_f org_xyz      = loadAOS4to16f(rayIndex,ray16.org.x,ray16.org.y,ray16.org.z);
	  const mic_f dir_xyz      = loadAOS4to16f(rayIndex,ray16.dir.x,ray16.dir.y,ray16.dir.z);
	  const mic_f rdir_xyz     = loadAOS4to16f(rayIndex,rdir16.x,rdir16.y,rdir16.z);
	  const mic_f org_rdir_xyz = org_xyz * rdir_xyz;
	  const mic_f min_dist_xyz = broadcast1to16f(&ray16.tnear[rayIndex]);
	  mic_f       max_dist_xyz = broadcast1to16f(&ray16.tfar[rayIndex]);

	  const unsigned int leaf_mask = BVH4I_LEAF_MASK;

	  while (1)
	    {

	      NodeRef curNode = stack_node[sindex-1];
	      sindex--;

	      traverse_single_intersect(curNode,
					sindex,
					rdir_xyz,
					org_rdir_xyz,
					min_dist_xyz,
					max_dist_xyz,
					stack_node,
					stack_dist,
					nodes,
					leaf_mask);
		   


	      /* return if stack is empty */
	      if (unlikely(curNode == BVH4i::invalidNode)) break;

	      STAT3(normal.trav_leaves,1,1,1);
	      STAT3(normal.trav_prims,4,4,4);

	      /* intersect one ray against four triangles */

	      //////////////////////////////////////////////////////////////////////////////////////////////////

	      const bool hit = LeafIntersector::intersect(curNode,
							  rayIndex,
							  dir_xyz,
							  org_xyz,
							  min_dist_xyz,
							  max_dist_xyz,
							  ray16,
							  accel,
							  (Scene*)bvh->geometry);
									   
	      if (hit)
		compactStack(stack_node,stack_dist,sindex,max_dist_xyz);

	      // ------------------------
	    }	  
	}
    }

    template<typename LeafIntersector>    
    void BVH4iIntersector16Single<LeafIntersector>::occluded(mic_i* valid_i, BVH4i* bvh, Ray16& ray16)
    {
      /* near and node stack */
      __aligned(64) NodeRef stack_node[3*BVH4i::maxDepth+1];

      /* setup */
      const mic_m m_valid = *(mic_i*)valid_i != mic_i(0);
      const mic3f rdir16  = rcp_safe(ray16.dir);
      mic_m terminated    = !m_valid;
      const mic_f inf     = mic_f(pos_inf);
      const mic_f zero    = mic_f::zero();

      const Node      * __restrict__ nodes = (Node     *)bvh->nodePtr();
      const Triangle1 * __restrict__ accel = (Triangle1*)bvh->triPtr();

      stack_node[0] = BVH4i::invalidNode;

      long rayIndex = -1;
      while((rayIndex = bitscan64(rayIndex,toInt(m_valid))) != BITSCAN_NO_BIT_SET_64)	    
        {
	  stack_node[1] = bvh->root;
	  size_t sindex = 2;

	  const mic_f org_xyz      = loadAOS4to16f(rayIndex,ray16.org.x,ray16.org.y,ray16.org.z);
	  const mic_f dir_xyz      = loadAOS4to16f(rayIndex,ray16.dir.x,ray16.dir.y,ray16.dir.z);
	  const mic_f rdir_xyz     = loadAOS4to16f(rayIndex,rdir16.x,rdir16.y,rdir16.z);
	  const mic_f org_rdir_xyz = org_xyz * rdir_xyz;
	  const mic_f min_dist_xyz = broadcast1to16f(&ray16.tnear[rayIndex]);
	  const mic_f max_dist_xyz = broadcast1to16f(&ray16.tfar[rayIndex]);
	  const mic_i v_invalidNode(BVH4i::invalidNode);
	  const unsigned int leaf_mask = BVH4I_LEAF_MASK;

	  while (1)
	    {
	      NodeRef curNode = stack_node[sindex-1];
	      sindex--;

	      traverse_single_occluded(curNode,
				       sindex,
				       rdir_xyz,
				       org_rdir_xyz,
				       min_dist_xyz,
				       max_dist_xyz,
				       stack_node,
				       nodes,
				       leaf_mask);

	      /* return if stack is empty */
	      if (unlikely(curNode == BVH4i::invalidNode)) break;

	      STAT3(shadow.trav_leaves,1,1,1);
	      STAT3(shadow.trav_prims,4,4,4);

	      /* intersect one ray against four triangles */

	      //////////////////////////////////////////////////////////////////////////////////////////////////

	      const mic_i and_mask = broadcast4to16i(zlc4);
	      const bool hit = LeafIntersector::occluded(curNode,
							 rayIndex,
							 dir_xyz,
							 org_xyz,
							 min_dist_xyz,
							 max_dist_xyz,
							 ray16,
							 terminated,
							 accel,
							 (Scene*)bvh->geometry);

	      if (unlikely(hit)) break;
	      //////////////////////////////////////////////////////////////////////////////////////////////////

	    }


	  if (unlikely(all(toMask(terminated)))) break;
	}


      store16i(m_valid & toMask(terminated),&ray16.geomID,0);
    }


    
    DEFINE_INTERSECTOR16    (BVH4iTriangle1Intersector16SingleMoeller, BVH4iIntersector16Single<Triangle1LeafIntersector>);
    DEFINE_INTERSECTOR16    (BVH4mcTriangle1Intersector16SingleMoeller, BVH4iIntersector16Single<Triangle1mcLeafIntersector>);

  }
}
