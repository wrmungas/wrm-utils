#include "render.h"


// these are all defined as inline in the header: this ensures a compiler symbol is actually emitted for them

wrm_RGBAf wrm_RGBAf_fromRGBAi(wrm_RGBAi rgbai);

wrm_RGBAf wrm_RGBAf_fromRGBA(wrm_RGBA rgba);

wrm_RGBAi wrm_RGBAi_fromRGBAf(wrm_RGBAf rgbaf);

wrm_RGBAi wrm_RGBAi_fromRGBA(wrm_RGBA rgba);

wrm_RGBA wrm_RGBA_fromRGBAi(wrm_RGBAi rgbai);

wrm_RGBA wrm_RGBA_fromRGBAf(wrm_RGBAf rgbaf);

// user-visible

wrm_Option_Handle wrm_render_createTexture(const wrm_Texture_Data *data)
{
    if(!data) return OPTION_NONE(Handle);
    wrm_Option_Handle result = wrm_Pool_getSlot(&wrm_textures);
    if(!result.exists) return result;

    GLuint texture;
    glActiveTexture(GL_TEXTURE0);
    glGenTextures(1, &texture);
    glBindTexture(GL_TEXTURE_2D, texture);

    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);	
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST_MIPMAP_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

    wrm_Texture *t = wrm_Pool_AT(wrm_textures, wrm_Texture, result.val);
    *t = (wrm_Texture){
        .gl_tex = texture,
        .w = data->width,
        .h = data->height,
    };

    if(data->channels == 1) {
        glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
        return result;
    }

    glTexImage2D(
        GL_TEXTURE_2D,  // texture target type
        0,              // detail level (for manually adding mipmaps; don't do this, generate them with glGenerateMipmap)
        GL_RGBA,      // format OpenGL should store the image with
        data->width,    // width in pixels
        data->height,   // height in pixels
        0,              // border (weird legacy argument - borders should be set explicitly with glTexParameterxx)
        GL_RGBA,      // format of the incoming image data
        GL_UNSIGNED_BYTE,
        data->pixels
    );
    
    glGenerateMipmap(GL_TEXTURE_2D);
    return result;
}

bool wrm_render_updateTexture(wrm_Handle texture, wrm_Texture_Data *data, u32 x, u32 y)
{
    if(!wrm_render_exists(texture, WRM_RENDER_RESOURCE_TEXTURE, "updateTexture()", "")) {
        return false;
    }

    wrm_Texture *t = wrm_Pool_AT(wrm_textures, wrm_Texture, texture);

    GLuint gl_format = (data->channels == 1) ? GL_ALPHA : GL_RGBA;
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, t->gl_tex);
    glTexSubImage2D(GL_TEXTURE, 0, x, y, data->width, data->height, gl_format, GL_UNSIGNED_BYTE, data->pixels);
    t->w = data->width;
    t->h = data->height;
    return true;
}

void wrm_render_printTextureData(wrm_Handle texture)
{
    if(!wrm_render_exists(texture, WRM_RENDER_RESOURCE_TEXTURE, "printTextureData()", "")) return;
    wrm_Texture *t = wrm_Pool_AT(wrm_textures, wrm_Texture, texture);
    printf(
        "[%u]: { gl_tex: %u, h: %u, w: %u }\n", 
        texture,
        t->gl_tex,
        t->h,
        t->w
    );
}

void wrm_render_deleteTexture(wrm_Handle texture)
{
    if(!wrm_render_exists(texture, WRM_RENDER_RESOURCE_TEXTURE, "deleteTexture()", "")) return;

    wrm_Texture *t = wrm_Pool_AT(wrm_textures, wrm_Texture, texture);

    glDeleteTextures(1, &(t->gl_tex));
    wrm_Pool_freeSlot(&wrm_textures, texture);
}

// module internal

bool wrm_render_createErrorTexture(void)
{
    wrm_RGBAi p = wrm_RGBAi_fromRGBA(WRM_RGBA_PURPLE);
    wrm_RGBAi b = wrm_RGBAi_fromRGBA(WRM_RGBA_BLACK);

    u8 pixels[] = {
        p.r, p.g, p.b, p.a,
        b.r, b.g, b.b, b.a,
        b.r, b.g, b.b, b.a,
        p.r, p.g, p.b, p.a
    };

    wrm_Texture_Data t = (wrm_Texture_Data) {
        .pixels = pixels,
        .height = 2,
        .width = 2
    };

    wrm_Option_Handle texture = wrm_render_createTexture(&t);
    if(!texture.exists) {
        return false;
    }

    if(wrm_render_settings.verbose) printf("Render: created error texture\n");
    return true;
}
