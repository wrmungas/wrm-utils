#include "render.h"

wrm_Camera wrm_camera;

void wrm_render_updateCamera(float *fov, float *offset, const vec3 pos, const vec3 rot)
{
    if(fov) wrm_camera.fov = *fov;
    if(offset) wrm_camera.offset = *offset;
    if(pos) wrm_vec3_copy(pos, wrm_camera.pos);
    if(rot) wrm_vec3_copy(rot, wrm_camera.rot);
}

void wrm_render_getCameraData(float *fov, float *offset, vec3 pos, vec3 rot)
{
    if(fov) *fov = wrm_camera.fov;
    if(offset) *offset = wrm_camera.offset;
    if(pos) wrm_vec3_copy(wrm_camera.pos, pos);
    if(rot) wrm_vec3_copy(wrm_camera.rot, rot);
}

void wrm_render_getViewMatrix(mat4 view)
{
    // update camera facing direction
    vec3 facing;
    vec3 up;
    wrm_render_getOrientation(wrm_camera.rot, facing, up, NULL);

    vec3 pos;
    wrm_vec3_copy(wrm_camera.pos, pos);
    
    // include offset in camera eye position
    vec3 eye;
    vec3 offset_vec;
    // If offset > 0 (backward): Subtract direction vector * offset from player position
    glm_vec3_scale(facing, wrm_camera.offset, offset_vec);
    glm_vec3_sub(pos, offset_vec, eye); 

    // get target point (can't just pass facing to lookAt())
    vec3 target;
    glm_vec3_add(eye, facing, target); 

    // get the view projection matrix
    glm_lookat(eye, target, up, view);
}

void wrm_render_printCameraData(void)
{
    vec3 facing;
    vec3 up;
    vec3 right;

    wrm_render_getOrientation(wrm_camera.rot, facing, up, right);
    printf(
        "Camera: {\n"
        "   pos: < %f %f %f >, rot = < %f %f %f >, fov: %f, offset: %f\n"
        "   facing: < %f %f %f >, up: < %f %f %f >, right: < %f %f %f >"
        "}\n",
        wrm_camera.pos[WRM_X], wrm_camera.pos[WRM_Y], wrm_camera.pos[WRM_Z],
        wrm_camera.rot[WRM_ROLL], wrm_camera.rot[WRM_YAW], wrm_camera.rot[WRM_PITCH],
        wrm_camera.fov, wrm_camera.offset,
        facing[WRM_X], facing[WRM_Y], facing[WRM_Z],
        up[WRM_X], up[WRM_Y], up[WRM_Z],
        right[WRM_X], right[WRM_Y], right[WRM_Z]
    );
}
