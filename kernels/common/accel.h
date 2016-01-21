// ======================================================================== //
// Copyright 2009-2015 Intel Corporation                                    //
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

#pragma once

#include "default.h"

namespace embree
{
  /*! Base class for the acceleration structure data. */
  class AccelData : public RefCount 
  {
  public:
    enum Type { TY_UNKNOWN = 0, TY_ACCELN = 1, TY_ACCEL_INSTANCE = 2, TY_BVH4 = 3, TY_BVH8 = 4 };

  public:
    AccelData (const Type type) : type(type) {
      bounds[0] = bounds[1] = empty;
    }

    /*! notifies the acceleration structure about the deletion of some geometry */
    virtual void deleteGeometry(size_t geomID) {};
   
    /*! clears the acceleration structure data */
    virtual void clear() = 0;

    /*! checks if the object has empty bounds */
    __forceinline bool isEmpty() const {
      return bounds[0].empty() || bounds[1].empty();
    }

    /*! calculate full bounding box */
    __forceinline BBox3fa getBounds() const {
      return merge(bounds[0],bounds[1]);
    }

    /*! set single bounding box */
    __forceinline void setBounds(const BBox3fa& a) {
      bounds[0] = bounds[1] = a;
    }

    /*! set two bounding boxes */
    __forceinline void setBounds(const BBox3fa& a, const BBox3fa& b) {
      bounds[0] = a;
      bounds[1] = b;
    }

  public:
    Type type;         //!< type of the acceleration structure
    BBox3fa bounds[2]; //!< bounds for timestep 0 and 1
  };

  /*! Base class for all intersectable and buildable acceleration structures. */
  class Accel : public AccelData
  {
    ALIGNED_CLASS;
  public:

    /*! Type of intersect function pointer for single rays. */
    typedef void (*IntersectFunc)(void* ptr,           /*!< pointer to user data */
                                  RTCRay& ray          /*!< ray to intersect */);
    
    /*! Type of intersect function pointer for ray packets of size 4. */
    typedef void (*IntersectFunc4)(const void* valid,  /*!< pointer to valid mask */
                                   void* ptr,          /*!< pointer to user data */
                                   RTCRay4& ray        /*!< ray packet to intersect */);
    
    /*! Type of intersect function pointer for ray packets of size 8. */
    typedef void (*IntersectFunc8)(const void* valid,  /*!< pointer to valid mask */
                                   void* ptr,          /*!< pointer to user data */
                                   RTCRay8& ray        /*!< ray packet to intersect */);
    
    /*! Type of intersect function pointer for ray packets of size 16. */
    typedef void (*IntersectFunc16)(const void* valid, /*!< pointer to valid mask */
                                    void* ptr,         /*!< pointer to user data */
                                    RTCRay16& ray      /*!< ray packet to intersect */);

    /*! Type of intersect function pointer for ray packets of size N. */
    typedef void (*IntersectFuncN)(const size_t N,    /*!< number of rays in stream */   
                                   void* ptr,         /*!< pointer to user data */
                                   void* ray          /*!< ray stream to intersect */);
    
    
    /*! Type of occlusion function pointer for single rays. */
    typedef void (*OccludedFunc) (void* ptr,           /*!< pointer to user data */ 
                                  RTCRay& ray          /*!< ray to test occlusion */);
    
    /*! Type of occlusion function pointer for ray packets of size 4. */
    typedef void (*OccludedFunc4) (const void* valid,  /*! pointer to valid mask */
                                   void* ptr,          /*!< pointer to user data */
                                   RTCRay4& ray        /*!< Ray packet to test occlusion. */);
    
    /*! Type of occlusion function pointer for ray packets of size 8. */
    typedef void (*OccludedFunc8) (const void* valid,  /*! pointer to valid mask */
                                   void* ptr,          /*!< pointer to user data */
                                   RTCRay8& ray        /*!< Ray packet to test occlusion. */);
    
    /*! Type of occlusion function pointer for ray packets of size 16. */
    typedef void (*OccludedFunc16) (const void* valid, /*! pointer to valid mask */
                                    void* ptr,         /*!< pointer to user data */
                                    RTCRay16& ray      /*!< Ray packet to test occlusion. */);

    /*! Type of intersect function pointer for ray packets of size N. */
    typedef void (*OccludedFuncN)(const size_t N,    /*!< number of rays in stream */   
                                  void* ptr,         /*!< pointer to user data */
                                  void* ray          /*!< ray stream to intersect */);

    typedef void (*ErrorFunc) ();

    struct Intersector1
    {
      Intersector1 (ErrorFunc error = nullptr) 
      : intersect((IntersectFunc)error), occluded((OccludedFunc)error), name(nullptr) {}

      Intersector1 (IntersectFunc intersect, OccludedFunc occluded, const char* name)
      : intersect(intersect), occluded(occluded), name(name) {}

      operator bool() const { return name; }

    public:
      static const char* type;
      const char* name;
      IntersectFunc intersect;
      OccludedFunc occluded;  
    };
    
    struct Intersector4 
    {
       Intersector4 (ErrorFunc error = nullptr) 
       : intersect((IntersectFunc4)error), occluded((OccludedFunc4)error), name(nullptr) {}

      Intersector4 (IntersectFunc4 intersect, OccludedFunc4 occluded, const char* name)
      : intersect(intersect), occluded(occluded), name(name) {}

      operator bool() const { return name; }
      
    public:
      static const char* type;
      const char* name;
      IntersectFunc4 intersect;
      OccludedFunc4 occluded;
    };
    
    struct Intersector8 
    {
      Intersector8 (ErrorFunc error = nullptr) 
      : intersect((IntersectFunc8)error), occluded((OccludedFunc8)error), name(nullptr) {}

      Intersector8 (IntersectFunc8 intersect, OccludedFunc8 occluded, const char* name)
      : intersect(intersect), occluded(occluded), name(name) {}

      operator bool() const { return name; }
      
    public:
      static const char* type;
      const char* name;
      IntersectFunc8 intersect;
      OccludedFunc8 occluded;
    };
    
    struct Intersector16 
    {
      Intersector16 (ErrorFunc error = nullptr) 
      : intersect((IntersectFunc16)error), occluded((OccludedFunc16)error), name(nullptr) {}

      Intersector16 (IntersectFunc16 intersect, OccludedFunc16 occluded, const char* name)
      : intersect(intersect), occluded(occluded), name(name) {}

      operator bool() const { return name; }
      
    public:
      static const char* type;
      const char* name;
      IntersectFunc16 intersect;
      OccludedFunc16 occluded;
    };

     struct IntersectorN 
    {
      IntersectorN (ErrorFunc error = nullptr) 
      : intersect((IntersectFuncN)error), occluded((OccludedFuncN)error), name(nullptr) {}

      IntersectorN (IntersectFuncN intersect, OccludedFuncN occluded, const char* name)
      : intersect(intersect), occluded(occluded), name(name) {}

      operator bool() const { return name; }
      
    public:
      static const char* type;
      const char* name;
      IntersectFuncN intersect;
      OccludedFuncN occluded;
    };
   
    struct Intersectors 
    {
      Intersectors() 
        : ptr(nullptr) {}

      Intersectors (ErrorFunc error) 
      : ptr(nullptr), intersector1(error), intersector4(error), intersector8(error), intersector16(error),intersectorN(error) {}

      void print(size_t ident) 
      {
        if (intersector1.name) {
          for (size_t i=0; i<ident; i++) std::cout << " ";
          std::cout << "intersector1  = " << intersector1.name << std::endl;
        }
        if (intersector4.name) {
          for (size_t i=0; i<ident; i++) std::cout << " ";
          std::cout << "intersector4  = " << intersector4.name << std::endl;
        }
        if (intersector8.name) {
          for (size_t i=0; i<ident; i++) std::cout << " ";
          std::cout << "intersector8  = " << intersector8.name << std::endl;
        }
        if (intersector16.name) {
          for (size_t i=0; i<ident; i++) std::cout << " ";
          std::cout << "intersector16 = " << intersector16.name << std::endl;
        }
        if (intersectorN.name) {
          for (size_t i=0; i<ident; i++) std::cout << " ";
          std::cout << "intersectorN = " << intersectorN.name << std::endl;
        }        
      }

      void select(bool filter4, bool filter8, bool filter16, bool filterN)
      {
	if (intersector4_filter) {
	  if (filter4) intersector4 = intersector4_filter;
	  else         intersector4 = intersector4_nofilter;
	}
	if (intersector8_filter) {
	  if (filter8) intersector8 = intersector8_filter;
	  else         intersector8 = intersector8_nofilter;
	}
	if (intersector16_filter) {
	  if (filter16) intersector16 = intersector16_filter;
	  else          intersector16 = intersector16_nofilter;
	}
	if (intersectorN_filter) {
	  if (filterN) intersectorN = intersectorN_filter;
	  else         intersectorN = intersectorN_nofilter;
	}
        
      }

    public:
      AccelData* ptr;
      Intersector1 intersector1;
      Intersector4 intersector4;
      Intersector4 intersector4_filter;
      Intersector4 intersector4_nofilter;
      Intersector8 intersector8;
      Intersector8 intersector8_filter;
      Intersector8 intersector8_nofilter;
      Intersector16 intersector16;
      Intersector16 intersector16_filter;
      Intersector16 intersector16_nofilter;
      IntersectorN intersectorN;
      IntersectorN intersectorN_filter;
      IntersectorN intersectorN_nofilter;      
    };
  
  public:

    /*! Construction */
    Accel (const AccelData::Type type) 
      : AccelData(type) {}
    
    /*! Construction */
    Accel (const AccelData::Type type, const Intersectors& intersectors) 
      : AccelData(type), intersectors(intersectors) {}

    /*! Virtual destructor */
    virtual ~Accel() {}

    /*! makes the acceleration structure immutable */
    virtual void immutable () {}
    
    /*! build acceleration structure */
    virtual void build (size_t threadIndex, size_t threadCount) = 0;

    /*! Intersects a single ray with the scene. */
    __forceinline void intersect (RTCRay& ray) {
      assert(intersectors.intersector1.intersect);
      intersectors.intersector1.intersect(intersectors.ptr,ray);
    }

    /*! Intersects a packet of 4 rays with the scene. */
    __forceinline void intersect4 (const void* valid, RTCRay4& ray) {
      assert(intersectors.intersector4.intersect);
      intersectors.intersector4.intersect(valid,intersectors.ptr,ray);
    }

    /*! Intersects a packet of 8 rays with the scene. */
    __forceinline void intersect8 (const void* valid, RTCRay8& ray) {
      assert(intersectors.intersector8.intersect);
      intersectors.intersector8.intersect(valid,intersectors.ptr,ray);
    }

    /*! Intersects a packet of 16 rays with the scene. */
    __forceinline void intersect16 (const void* valid, RTCRay16& ray) {
      assert(intersectors.intersector16.intersect);
      intersectors.intersector16.intersect(valid,intersectors.ptr,ray);
    }

    /*! Intersects a packet of N rays in SOA layout with the scene. */
    __forceinline void intersectN (const size_t N, void* rayN) {
      assert(intersectors.intersectorN.intersect);
      intersectors.intersectorN.intersect(N,intersectors.ptr,rayN);
    }

    /*! Tests if single ray is occluded by the scene. */
    __forceinline void occluded (RTCRay& ray) {
      assert(intersectors.intersector1.occluded);
      intersectors.intersector1.occluded(intersectors.ptr,ray);
    }
    
    /*! Tests if a packet of 4 rays is occluded by the scene. */
    __forceinline void occluded4 (const void* valid, RTCRay4& ray) {
      assert(intersectors.intersector4.occluded);
      intersectors.intersector4.occluded(valid,intersectors.ptr,ray);
    }

    /*! Tests if a packet of 8 rays is occluded by the scene. */
    __forceinline void occluded8 (const void* valid, RTCRay8& ray) {
      assert(intersectors.intersector8.occluded);
      intersectors.intersector8.occluded(valid,intersectors.ptr,ray);
    }

    /*! Tests if a packet of 16 rays is occluded by the scene. */
    __forceinline void occluded16 (const void* valid, RTCRay16& ray) {
      assert(intersectors.intersector16.occluded);
      intersectors.intersector16.occluded(valid,intersectors.ptr,ray);
    }

    /*! Tests if a packet of N rays in SOA layout is occluded by the scene. */
    __forceinline void occludedN (const size_t N, void* rayN) {
      assert(intersectors.intersectorN.occluded);
      intersectors.intersectorN.occluded(N,intersectors.ptr,rayN);
    }

  public:
    Intersectors intersectors;
  };

#define DEFINE_INTERSECTOR1(symbol,intersector)                         \
  Accel::Intersector1 symbol((Accel::IntersectFunc)intersector::intersect, \
                             (Accel::OccludedFunc )intersector::occluded, \
                             TOSTRING(isa) "::" TOSTRING(symbol));
  
#define DEFINE_INTERSECTOR4(symbol,intersector)                         \
  Accel::Intersector4 symbol((Accel::IntersectFunc4)intersector::intersect, \
                             (Accel::OccludedFunc4)intersector::occluded, \
                             TOSTRING(isa) "::" TOSTRING(symbol));
  
#define DEFINE_INTERSECTOR8(symbol,intersector)                         \
  Accel::Intersector8 symbol((Accel::IntersectFunc8)intersector::intersect, \
                             (Accel::OccludedFunc8)intersector::occluded, \
                             TOSTRING(isa) "::" TOSTRING(symbol));

#define DEFINE_INTERSECTOR16(symbol,intersector)                         \
  Accel::Intersector16 symbol((Accel::IntersectFunc16)intersector::intersect, \
                              (Accel::OccludedFunc16)intersector::occluded,\
                              TOSTRING(isa) "::" TOSTRING(symbol));

#define DEFINE_INTERSECTORN(symbol,intersector)                         \
  Accel::IntersectorN symbol((Accel::IntersectFuncN)intersector::intersect, \
                              (Accel::OccludedFuncN)intersector::occluded,\
                              TOSTRING(isa) "::" TOSTRING(symbol));
 
}
