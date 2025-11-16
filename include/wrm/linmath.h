#ifndef WRM_LINMATH_H
#define WRM_LINMATH_H

/*
File linmath.h

Created  Nov 14, 2025 
by William R Mungas (wrm)

Version: 0.1.0 
(Last modified Nov 14, 2025)

DESCRIPTION:
Basic 3d linear math using cglm, with some additional functions to add const-correctness

PROVIDES:
- cglm types: vec<n>, mat<n>, versor (quaternion) - typedefs of arrays, used in SIMD-efficient
    functions
- standard X,Y,Z and ROLL,YAW,PITCH values for designated initializers to prevent bugs

REQUIREMENTS:
Must link with c standard math lib: -lm
Must have cglm/cglm.h in your 'include/', used as header-only

*/

#include "wrm/common.h"
#include "cglm/cglm.h"


#define WRM_X 0
#define WRM_Y 1
#define WRM_Z 2

#define WRM_PITCH WRM_Z
#define WRM_YAW WRM_Y
#define WRM_ROLL WRM_X

/* const-correct vector copy - use instead of `glm_vec3_copy()` if you have a const `src` vector */
inline void wrm_vec3_copy(const vec3 src, vec3 dest)
{
    if(src && dest && src != dest) {
        dest[WRM_X] = src[WRM_X];
        dest[WRM_Y] = src[WRM_Y];
        dest[WRM_Z] = src[WRM_Z];
    }
}

/* const-correct vector add - use instead of `glm_vec3_add()` if you have a const `src` vector */
inline void wrm_vec3_add(const vec3 src, vec3 dest)
{
    if(src && dest && src != dest) {
        dest[WRM_X] += src[WRM_X];
        dest[WRM_Y] += src[WRM_Y];
        dest[WRM_Z] += src[WRM_Z];
    }
}

#endif