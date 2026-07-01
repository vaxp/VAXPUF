import os

file_path = "/home/x/Desktop/VAXPUF/src/graphics/vaxp_canvas_opengl.c"
with open(file_path, "r") as f:
    content = f.read()

# 1. Includes and glBufferSubData
content = content.replace(
    "#include <stdlib.h>",
    "#include <stdlib.h>\n#include <stddef.h>\n#include <cairo/cairo.h>\n#include <pango/pangocairo.h>"
)

content = content.replace(
    "static PFNGLBUFFERDATAPROC glBufferData = NULL;",
    "static PFNGLBUFFERDATAPROC glBufferData = NULL;\nstatic PFNGLBUFFERSUBDATAPROC glBufferSubData = NULL;"
)

content = content.replace(
    "LOAD_GL(glBufferData);",
    "LOAD_GL(glBufferData);\n    LOAD_GL(glBufferSubData);"
)

# 2. Add Uber Shaders
UBER_SHADER_V = """/* Uber Vertex Shader for Batching */
static const char* VERTEX_SHADER_UBER = 
    "#version 330 core\\n"
    "layout(location = 0) in vec2 aPos;\\n"
    "layout(location = 1) in vec2 aTexCoord;\\n"
    "layout(location = 2) in vec4 aColor;\\n"
    "layout(location = 3) in vec4 aParams;\\n"
    "uniform mat4 uProjection;\\n"
    "out vec2 vTexCoord;\\n"
    "out vec4 vColor;\\n"
    "out vec4 vParams;\\n"
    "void main() {\\n"
    "    gl_Position = uProjection * vec4(aPos, 0.0, 1.0);\\n"
    "    vTexCoord = aTexCoord;\\n"
    "    vColor = aColor;\\n"
    "    vParams = aParams;\\n"
    "}\\n";

/* Uber Fragment Shader for Batching */
static const char* FRAGMENT_SHADER_UBER = 
    "#version 330 core\\n"
    "in vec2 vTexCoord;\\n"
    "in vec4 vColor;\\n"
    "in vec4 vParams;\\n"
    "uniform sampler2D uTex;\\n"
    "out vec4 FragColor;\\n"
    "float roundedBoxSDF(vec2 p, vec2 b, float r) {\\n"
    "    vec2 q = abs(p) - b + r;\\n"
    "    return length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - r;\\n"
    "}\\n"
    "void main() {\\n"
    "    int type = int(vParams.x + 0.5);\\n"
    "    if (type == 0) {\\n"
    "        FragColor = vColor;\\n"
    "    } else if (type == 1) {\\n"
    "        FragColor = texture(uTex, vTexCoord) * vColor;\\n"
    "    } else if (type == 2) {\\n"
    "        vec2 size = vParams.zw;\\n"
    "        float radius = vParams.y;\\n"
    "        vec2 halfSize = size * 0.5;\\n"
    "        vec2 p = vTexCoord - halfSize;\\n"
    "        float d = roundedBoxSDF(p, halfSize, radius);\\n"
    "        float alpha = 1.0 - smoothstep(-1.0, 1.0, d);\\n"
    "        FragColor = vec4(vColor.rgb, vColor.a * alpha);\\n"
    "    }\\n"
    "}\\n";
"""
content = content.replace("/* Basic 2D vertex shader */", UBER_SHADER_V + "\n/* Basic 2D vertex shader */")

# 3. Add Structs
STRUCTS = """
#define MAX_BATCH_VERTICES 16384
#define MAX_TEXT_CACHE 256

typedef struct {
    VaxpF32 x, y;
    VaxpF32 u, v;
    VaxpF32 r, g, b, a;
    VaxpF32 type, radius, width, height;
} VaxpGLVertex;

typedef struct {
    char* text;
    VaxpColor color;
    GLuint texture;
    int width;
    int height;
    float font_size;
} TextCacheEntry;
"""
content = content.replace("typedef struct VaxpGLCanvas {", STRUCTS + "\ntypedef struct VaxpGLCanvas {")

CANVAS_ADD = """
    /* Batching state */
    GLuint prog_uber;
    GLuint uber_vao;
    GLuint uber_vbo;
    VaxpGLVertex batch_vertices[MAX_BATCH_VERTICES];
    int batch_count;
    GLuint current_texture;
    
    /* Text cache */
    TextCacheEntry text_cache[MAX_TEXT_CACHE];
    int text_cache_count;
    int text_cache_head;
"""
content = content.replace("/* Current transform */", CANVAS_ADD + "\n    /* Current transform */")

# 4. Batch Flush Function
BATCH_FLUSH = """
static void gl_batch_flush(VaxpGLCanvas* c) {
    if (c->batch_count == 0) return;
    
    glUseProgram(c->prog_uber);
    glUniformMatrix4fv(glGetUniformLocation(c->prog_uber, "uProjection"), 1, GL_FALSE, c->projection_matrix);
    
    if (c->current_texture != 0) {
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, c->current_texture);
        glUniform1i(glGetUniformLocation(c->prog_uber, "uTex"), 0);
    }
    
    glBindVertexArray(c->uber_vao);
    glBindBuffer(GL_ARRAY_BUFFER, c->uber_vbo);
    glBufferSubData(GL_ARRAY_BUFFER, 0, c->batch_count * sizeof(VaxpGLVertex), c->batch_vertices);
    
    glDrawArrays(GL_TRIANGLES, 0, c->batch_count);
    c->batch_count = 0;
}

static void gl_batch_push_quad(VaxpGLCanvas* c, VaxpF32 type, VaxpF32 rad, VaxpF32 w, VaxpF32 h,
                               VaxpF32* pts, VaxpF32* uvs, VaxpColor color, GLuint tex) {
    if (c->current_texture != tex || c->batch_count + 6 > MAX_BATCH_VERTICES) {
        gl_batch_flush(c);
        c->current_texture = tex;
    }
    
    VaxpF32 r = color.r/255.0f, g = color.g/255.0f, b = color.b/255.0f, a = color.a/255.0f;
    
    /* Apply model matrix to vertices */
    VaxpF32 tx[4], ty[4];
    for (int i=0; i<4; i++) {
        tx[i] = pts[i*2] * c->model_matrix[0] + pts[i*2+1] * c->model_matrix[4] + c->model_matrix[12];
        ty[i] = pts[i*2] * c->model_matrix[1] + pts[i*2+1] * c->model_matrix[5] + c->model_matrix[13];
    }
    
    int indices[] = {0, 1, 2, 0, 2, 3};
    for (int i=0; i<6; i++) {
        int idx = indices[i];
        VaxpGLVertex* v = &c->batch_vertices[c->batch_count++];
        v->x = tx[idx]; v->y = ty[idx];
        v->u = uvs[idx*2]; v->v = uvs[idx*2+1];
        v->r = r; v->g = g; v->b = b; v->a = a;
        v->type = type; v->radius = rad; v->width = w; v->height = h;
    }
}
"""
content = content.replace("static void gl_canvas_clear", BATCH_FLUSH + "\nstatic void gl_canvas_clear")


# 5. Initialization
INIT = """
    /* Create Uber Shader for Batching */
    c->prog_uber = create_program(VERTEX_SHADER_UBER, FRAGMENT_SHADER_UBER);
    glGenVertexArrays(1, &c->uber_vao);
    glGenBuffers(1, &c->uber_vbo);
    glBindVertexArray(c->uber_vao);
    glBindBuffer(GL_ARRAY_BUFFER, c->uber_vbo);
    glBufferData(GL_ARRAY_BUFFER, MAX_BATCH_VERTICES * sizeof(VaxpGLVertex), NULL, GL_DYNAMIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VaxpGLVertex), (void*)offsetof(VaxpGLVertex, x));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VaxpGLVertex), (void*)offsetof(VaxpGLVertex, u));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(VaxpGLVertex), (void*)offsetof(VaxpGLVertex, r));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(VaxpGLVertex), (void*)offsetof(VaxpGLVertex, type));
"""
content = content.replace("c->prog_text = create_program(VERTEX_SHADER_TEXT, FRAGMENT_SHADER_TEXT);", 
                          "c->prog_text = create_program(VERTEX_SHADER_TEXT, FRAGMENT_SHADER_TEXT);\n" + INIT)


# 6. Clip changes flush
content = content.replace(
    "c->current_clip = new_clip;\n    c->clip_enabled = VAXP_TRUE;",
    "gl_batch_flush(c);\n    c->current_clip = new_clip;\n    c->clip_enabled = VAXP_TRUE;"
)

content = content.replace(
    "c->current_clip = c->clip_stack[c->stack_depth];\n        c->clip_enabled = c->clip_enabled_stack[c->stack_depth];",
    "gl_batch_flush(c);\n        c->current_clip = c->clip_stack[c->stack_depth];\n        c->clip_enabled = c->clip_enabled_stack[c->stack_depth];"
)

# 7. Draw functions rewrites via string manipulation (safe)
import re

def replace_func(name, new_body):
    global content
    idx = content.find("static void " + name)
    if idx == -1: return
    end_idx = content.find("}\n", idx)
    content = content[:idx] + new_body + "\n" + content[end_idx+2:]

draw_rect = """static void gl_canvas_draw_rect(VaxpCanvas* canvas, VaxpRectF rect, const VaxpPaint* paint) {
    VaxpGLCanvas* c = (VaxpGLCanvas*)canvas;
    VaxpF32 pts[8] = {rect.x, rect.y, rect.x+rect.width, rect.y, rect.x+rect.width, rect.y+rect.height, rect.x, rect.y+rect.height};
    VaxpF32 uvs[8] = {0,0, 0,0, 0,0, 0,0};
    gl_batch_push_quad(c, 0.0f, 0.0f, 0.0f, 0.0f, pts, uvs, paint->color, 0);
}"""
replace_func("gl_canvas_draw_rect", draw_rect)

draw_rounded_rect = """static void gl_canvas_draw_rounded_rect(VaxpCanvas* canvas, VaxpRectF rect, VaxpF32 radius, const VaxpPaint* paint) {
    VaxpGLCanvas* c = (VaxpGLCanvas*)canvas;
    VaxpF32 pts[8] = {rect.x, rect.y, rect.x+rect.width, rect.y, rect.x+rect.width, rect.y+rect.height, rect.x, rect.y+rect.height};
    VaxpF32 uvs[8] = {0,0, rect.width,0, rect.width,rect.height, 0,rect.height};
    gl_batch_push_quad(c, 2.0f, radius, rect.width, rect.height, pts, uvs, paint->color, 0);
}"""
replace_func("gl_canvas_draw_rounded_rect", draw_rounded_rect)

draw_circle = """static void gl_canvas_draw_circle(VaxpCanvas* canvas, VaxpF32 cx, VaxpF32 cy, VaxpF32 radius, const VaxpPaint* paint) {
    VaxpGLCanvas* c = (VaxpGLCanvas*)canvas;
    VaxpF32 pts[8] = {cx-radius, cy-radius, cx+radius, cy-radius, cx+radius, cy+radius, cx-radius, cy+radius};
    VaxpF32 uvs[8] = {-radius,-radius, radius,-radius, radius,radius, -radius,radius};
    gl_batch_push_quad(c, 2.0f, radius, radius*2, radius*2, pts, uvs, paint->color, 0);
}"""
replace_func("gl_canvas_draw_circle", draw_circle)

draw_image = """static void gl_canvas_draw_image(VaxpCanvas* canvas, const VaxpImage* image, VaxpF32 x, VaxpF32 y) {
    VaxpGLCanvas* c = (VaxpGLCanvas*)canvas;
    if (!image || !image->platform_handle) return;
    GLuint tex = (GLuint)(uintptr_t)image->platform_handle;
    VaxpF32 w = image->width, h = image->height;
    VaxpF32 pts[8] = {x, y, x+w, y, x+w, y+h, x, y+h};
    VaxpF32 uvs[8] = {0,0, 1,0, 1,1, 0,1};
    gl_batch_push_quad(c, 1.0f, 0.0f, 0.0f, 0.0f, pts, uvs, vaxp_color_rgba(255,255,255,255), tex);
}"""
replace_func("gl_canvas_draw_image", draw_image)

DRAW_TEXT = """static void gl_canvas_draw_text(VaxpCanvas* canvas, const char* text, VaxpF32 x, VaxpF32 y,
                                 const VaxpFont* font, const VaxpPaint* paint) {
    VaxpGLCanvas* c = (VaxpGLCanvas*)canvas;
    if (!text || !*text) return;
    
    GLuint tex = 0;
    int tw = 0, th = 0;
    
    /* Search cache */
    for (int i=0; i<c->text_cache_count; i++) {
        TextCacheEntry* e = &c->text_cache[i];
        if (e->color.r == paint->color.r && e->color.g == paint->color.g && 
            e->color.b == paint->color.b && e->color.a == paint->color.a &&
            e->font_size == c->font_size && strcmp(e->text, text) == 0) {
            tex = e->texture;
            tw = e->width;
            th = e->height;
            break;
        }
    }
    
    if (tex == 0) {
        cairo_surface_t* temp_surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, 1, 1);
        cairo_t* cr = cairo_create(temp_surface);
        PangoLayout* layout = pango_cairo_create_layout(cr);
        
        char font_desc_str[64];
        snprintf(font_desc_str, sizeof(font_desc_str), "Noto Sans %f", c->font_size > 0 ? c->font_size : 14.0f);
        PangoFontDescription* desc = pango_font_description_from_string(font_desc_str);
        pango_layout_set_font_description(layout, desc);
        pango_font_description_free(desc);
        
        pango_layout_set_text(layout, text, -1);
        pango_layout_get_pixel_size(layout, &tw, &th);
        
        cairo_destroy(cr);
        cairo_surface_destroy(temp_surface);
        
        if (tw == 0 || th == 0) {
            g_object_unref(layout);
            return;
        }
        
        cairo_surface_t* surface = cairo_image_surface_create(CAIRO_FORMAT_ARGB32, tw, th);
        cr = cairo_create(surface);
        
        cairo_set_source_rgba(cr, 0, 0, 0, 0);
        cairo_set_operator(cr, CAIRO_OPERATOR_SOURCE);
        cairo_paint(cr);
        
        cairo_set_operator(cr, CAIRO_OPERATOR_OVER);
        cairo_set_source_rgba(cr, paint->color.r/255.0f, paint->color.g/255.0f, paint->color.b/255.0f, paint->color.a/255.0f);
        
        pango_cairo_update_layout(cr, layout);
        
        /* Adjust for baseline like cairo backend */
        VaxpF32 adjusted_y = th * 0.15f; 
        cairo_move_to(cr, 0, 0);
        
        pango_cairo_show_layout(cr, layout);
        
        cairo_surface_flush(surface);
        unsigned char* data = cairo_image_surface_get_data(surface);
        
        /* Generate texture */
        /* Must flush batch before generating texture because active GL context operations? No, it's fine. */
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, tw, th, 0, GL_BGRA, GL_UNSIGNED_BYTE, data);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        
        cairo_destroy(cr);
        cairo_surface_destroy(surface);
        g_object_unref(layout);
        
        /* Add to LRU/Cache */
        int idx = c->text_cache_count < MAX_TEXT_CACHE ? c->text_cache_count++ : c->text_cache_head;
        c->text_cache_head = (idx + 1) % MAX_TEXT_CACHE;
        
        TextCacheEntry* e = &c->text_cache[idx];
        if (e->text) free(e->text);
        e->text = strdup(text);
        if (e->texture) glDeleteTextures(1, &e->texture);
        e->texture = tex;
        e->width = tw;
        e->height = th;
        e->color = paint->color;
        e->font_size = c->font_size;
    }
    
    /* Adjust drawing Y so it matches Cairo's baseline behavior */
    VaxpF32 draw_y = y - th * 0.7f;
    VaxpF32 pts[8] = {x, draw_y, x+tw, draw_y, x+tw, draw_y+th, x, draw_y+th};
    VaxpF32 uvs[8] = {0,0, 1,0, 1,1, 0,1};
    gl_batch_push_quad(c, 1.0f, 0.0f, 0.0f, 0.0f, pts, uvs, vaxp_color_rgba(255,255,255,255), tex);
}"""
idx = content.find("static void gl_canvas_draw_text")
if idx != -1:
    end_idx = content.find("static void gl_canvas_draw_image", idx)
    if end_idx != -1:
        content = content[:idx] + DRAW_TEXT + "\n\n" + content[end_idx:]


# Finally, flush at the end of the frame
content = content.replace("static void gl_canvas_flush(VaxpCanvas* canvas) {",
                          "static void gl_canvas_flush(VaxpCanvas* canvas) {\n    gl_batch_flush((VaxpGLCanvas*)canvas);")

with open(file_path, "w") as f:
    f.write(content)
print("Patch script completed.")
