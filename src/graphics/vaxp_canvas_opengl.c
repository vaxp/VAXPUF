/*
 * VAXPUI - High Performance GUI Framework
 * 
 * vaxp_canvas_opengl.c - OpenGL 3.3 Core Profile implementation of canvas
 * 
 * Hardware-accelerated 2D/3D rendering with GLSL shaders.
 * Text rendering: FreeType + HarfBuzz + FriBidi (NO Pango, NO Cairo)
 */

#define _USE_MATH_DEFINES
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "vaxp/graphics/vaxp_canvas.h"
#include "vaxp/widgets/vaxp_image.h"
#include "vaxp/core/vaxp_memory.h"
#include "vaxp_text_engine.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glext.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stddef.h>
#include <sys/time.h>

/* stb_truetype kept for non-text font bitmap (not used for text) */
#define STB_TRUETYPE_IMPLEMENTATION
#include "vaxp/third_party/stb_truetype.h"

/* ============================================================================
 * OPENGL FUNCTION POINTERS (GL 3.3 Core)
 * ============================================================================ */

/* Shader functions */
static PFNGLCREATESHADERPROC glCreateShader = NULL;
static PFNGLSHADERSOURCEPROC glShaderSource = NULL;
static PFNGLCOMPILESHADERPROC glCompileShader = NULL;
static PFNGLGETSHADERIVPROC glGetShaderiv = NULL;
static PFNGLGETSHADERINFOLOGPROC glGetShaderInfoLog = NULL;
static PFNGLDELETESHADERPROC glDeleteShader = NULL;
static PFNGLCREATEPROGRAMPROC glCreateProgram = NULL;
static PFNGLATTACHSHADERPROC glAttachShader = NULL;
static PFNGLLINKPROGRAMPROC glLinkProgram = NULL;
static PFNGLGETPROGRAMIVPROC glGetProgramiv = NULL;
static PFNGLGETPROGRAMINFOLOGPROC glGetProgramInfoLog = NULL;
static PFNGLDELETEPROGRAMPROC glDeleteProgram = NULL;
static PFNGLUSEPROGRAMPROC glUseProgram = NULL;
static PFNGLGETUNIFORMLOCATIONPROC glGetUniformLocation = NULL;
static PFNGLUNIFORM1FPROC glUniform1f = NULL;
static PFNGLUNIFORM1IPROC glUniform1i = NULL;
static PFNGLUNIFORM2FPROC glUniform2f = NULL;
static PFNGLUNIFORM3FPROC glUniform3f = NULL;
static PFNGLUNIFORM4FPROC glUniform4f = NULL;
static PFNGLUNIFORMMATRIX4FVPROC glUniformMatrix4fv = NULL;

/* Buffer functions */
static PFNGLGENVERTEXARRAYSPROC glGenVertexArrays = NULL;
static PFNGLBINDVERTEXARRAYPROC glBindVertexArray = NULL;
static PFNGLDELETEVERTEXARRAYSPROC glDeleteVertexArrays = NULL;
static PFNGLGENBUFFERSPROC glGenBuffers = NULL;
static PFNGLBINDBUFFERPROC glBindBuffer = NULL;
static PFNGLBUFFERDATAPROC glBufferData = NULL;
static PFNGLBUFFERSUBDATAPROC glBufferSubData = NULL;
static PFNGLDELETEBUFFERSPROC glDeleteBuffers = NULL;
static PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = NULL;
static PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = NULL;

/* Framebuffer functions */
static PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers = NULL;
static PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer = NULL;
static PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers = NULL;
static PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D = NULL;
static PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus = NULL;

static VaxpBool gl_functions_loaded = VAXP_FALSE;

static VaxpBool load_gl_functions(void) {
    if (gl_functions_loaded) return VAXP_TRUE;
    
    #define LOAD_GL(name) do { \
        name = (void*)glXGetProcAddress((const GLubyte*)#name); \
        if (!name) { fprintf(stderr, "Failed to load " #name "\n"); return VAXP_FALSE; } \
    } while(0)
    
    LOAD_GL(glCreateShader);
    LOAD_GL(glShaderSource);
    LOAD_GL(glCompileShader);
    LOAD_GL(glGetShaderiv);
    LOAD_GL(glGetShaderInfoLog);
    LOAD_GL(glDeleteShader);
    LOAD_GL(glCreateProgram);
    LOAD_GL(glAttachShader);
    LOAD_GL(glLinkProgram);
    LOAD_GL(glGetProgramiv);
    LOAD_GL(glGetProgramInfoLog);
    LOAD_GL(glDeleteProgram);
    LOAD_GL(glUseProgram);
    LOAD_GL(glGetUniformLocation);
    LOAD_GL(glUniform1f);
    LOAD_GL(glUniform1i);
    LOAD_GL(glUniform2f);
    LOAD_GL(glUniform3f);
    LOAD_GL(glUniform4f);
    LOAD_GL(glUniformMatrix4fv);
    LOAD_GL(glGenVertexArrays);
    LOAD_GL(glBindVertexArray);
    LOAD_GL(glDeleteVertexArrays);
    LOAD_GL(glGenBuffers);
    LOAD_GL(glBindBuffer);
    LOAD_GL(glBufferData);
    LOAD_GL(glBufferSubData);
    LOAD_GL(glDeleteBuffers);
    LOAD_GL(glVertexAttribPointer);
    LOAD_GL(glEnableVertexAttribArray);
    LOAD_GL(glGenFramebuffers);
    LOAD_GL(glBindFramebuffer);
    LOAD_GL(glDeleteFramebuffers);
    LOAD_GL(glFramebufferTexture2D);
    LOAD_GL(glCheckFramebufferStatus);
    
    #undef LOAD_GL
    
    gl_functions_loaded = VAXP_TRUE;
    return VAXP_TRUE;
}

/* ============================================================================
 * SHADER SOURCES
 * ============================================================================ */

/* Uber Vertex Shader for Batching */
static const char* VERTEX_SHADER_UBER = 
    "#version 330 core\n"
    "layout(location = 0) in vec2 aPos;\n"
    "layout(location = 1) in vec2 aTexCoord;\n"
    "layout(location = 2) in vec4 aColor;\n"
    "layout(location = 3) in vec4 aParams;\n"
    "uniform mat4 uProjection;\n"
    "out vec2 vTexCoord;\n"
    "out vec4 vColor;\n"
    "out vec4 vParams;\n"
    "void main() {\n"
    "    gl_Position = uProjection * vec4(aPos, 0.0, 1.0);\n"
    "    vTexCoord = aTexCoord;\n"
    "    vColor = aColor;\n"
    "    vParams = aParams;\n"
    "}\n";

/* Uber Fragment Shader for Batching */
static const char* FRAGMENT_SHADER_UBER = 
    "#version 330 core\n"
    "in vec2 vTexCoord;\n"
    "in vec4 vColor;\n"
    "in vec4 vParams;\n"
    "uniform sampler2D uTex;\n"
    "out vec4 FragColor;\n"
    "float roundedBoxSDF(vec2 p, vec2 b, float r) {\n"
    "    vec2 q = abs(p) - b + r;\n"
    "    return length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - r;\n"
    "}\n"
    "void main() {\n"
    "    int type = int(vParams.x + 0.5);\n"
    "    if (type == 0) {\n"
    "        FragColor = vColor;\n"
    "    } else if (type == 1) {\n"
    "        FragColor = texture(uTex, vTexCoord) * vColor;\n"
    "    } else if (type == 2) {\n"
    "        vec2 size = vParams.zw;\n"
    "        float radius = vParams.y;\n"
    "        vec2 halfSize = size * 0.5;\n"
    "        vec2 p = vTexCoord - halfSize;\n"
    "        float d = roundedBoxSDF(p, halfSize, radius);\n"
    "        float alpha = 1.0 - smoothstep(-1.0, 1.0, d);\n"
    "        FragColor = vec4(vColor.rgb, vColor.a * alpha);\n"
    "    }\n"
    "}\n";

/* Basic 2D vertex shader */
static const char* VERTEX_SHADER_2D = 
    "#version 330 core\n"
    "layout(location = 0) in vec2 aPos;\n"
    "layout(location = 1) in vec2 aTexCoord;\n"
    "uniform mat4 uProjection;\n"
    "uniform mat4 uModel;\n"
    "out vec2 vTexCoord;\n"
    "out vec2 vLocalPos;\n"
    "void main() {\n"
    "    gl_Position = uProjection * uModel * vec4(aPos, 0.0, 1.0);\n"
    "    vTexCoord = aTexCoord;\n"
    "    vLocalPos = aPos;\n"
    "}\n";

/* Solid color fragment shader */
static const char* FRAGMENT_SHADER_SOLID = 
    "#version 330 core\n"
    "uniform vec4 uColor;\n"
    "out vec4 FragColor;\n"
    "void main() {\n"
    "    FragColor = uColor;\n"
    "}\n";

/* Rounded rectangle fragment shader (SDF-based) */
static const char* FRAGMENT_SHADER_ROUNDED_RECT = 
    "#version 330 core\n"
    "uniform vec4 uColor;\n"
    "uniform vec2 uSize;\n"
    "uniform float uRadius;\n"
    "in vec2 vLocalPos;\n"
    "out vec4 FragColor;\n"
    "float roundedBoxSDF(vec2 p, vec2 b, float r) {\n"
    "    vec2 q = abs(p) - b + r;\n"
    "    return length(max(q, 0.0)) + min(max(q.x, q.y), 0.0) - r;\n"
    "}\n"
    "void main() {\n"
    "    vec2 halfSize = uSize * 0.5;\n"
    "    vec2 p = vLocalPos - halfSize;\n"
    "    float d = roundedBoxSDF(p, halfSize, uRadius);\n"
    "    float alpha = 1.0 - smoothstep(-1.0, 1.0, d);\n"
    "    FragColor = vec4(uColor.rgb, uColor.a * alpha);\n"
    "}\n";

/* Circle fragment shader */
static const char* FRAGMENT_SHADER_CIRCLE = 
    "#version 330 core\n"
    "uniform vec4 uColor;\n"
    "uniform float uRadius;\n"
    "in vec2 vLocalPos;\n"
    "out vec4 FragColor;\n"
    "void main() {\n"
    "    float d = length(vLocalPos) - uRadius;\n"
    "    float alpha = 1.0 - smoothstep(-1.0, 1.0, d);\n"
    "    FragColor = vec4(uColor.rgb, uColor.a * alpha);\n"
    "}\n";

/* 3D vertex shader with lighting */
static const char* VERTEX_SHADER_3D = 
    "#version 330 core\n"
    "layout(location = 0) in vec3 aPos;\n"
    "layout(location = 1) in vec3 aNormal;\n"
    "layout(location = 2) in vec3 aColor;\n"
    "uniform mat4 uModel;\n"
    "uniform mat4 uView;\n"
    "uniform mat4 uProjection;\n"
    "out vec3 vNormal;\n"
    "out vec3 vFragPos;\n"
    "out vec3 vColor;\n"
    "void main() {\n"
    "    gl_Position = uProjection * uView * uModel * vec4(aPos, 1.0);\n"
    "    vFragPos = vec3(uModel * vec4(aPos, 1.0));\n"
    "    vNormal = mat3(transpose(inverse(uModel))) * aNormal;\n"
    "    vColor = aColor;\n"
    "}\n";

/* 3D fragment shader with Phong lighting */
static const char* FRAGMENT_SHADER_3D = 
    "#version 330 core\n"
    "uniform vec3 uLightPos;\n"
    "uniform vec3 uLightColor;\n"
    "uniform vec3 uViewPos;\n"
    "in vec3 vNormal;\n"
    "in vec3 vFragPos;\n"
    "in vec3 vColor;\n"
    "out vec4 FragColor;\n"
    "void main() {\n"
    "    float ambientStrength = 0.2;\n"
    "    vec3 ambient = ambientStrength * uLightColor;\n"
    "    vec3 norm = normalize(vNormal);\n"
    "    vec3 lightDir = normalize(uLightPos - vFragPos);\n"
    "    float diff = max(dot(norm, lightDir), 0.0);\n"
    "    vec3 diffuse = diff * uLightColor;\n"
    "    float specularStrength = 0.5;\n"
    "    vec3 viewDir = normalize(uViewPos - vFragPos);\n"
    "    vec3 reflectDir = reflect(-lightDir, norm);\n"
    "    float spec = pow(max(dot(viewDir, reflectDir), 0.0), 32.0);\n"
    "    vec3 specular = specularStrength * spec * uLightColor;\n"
    "    vec3 result = (ambient + diffuse + specular) * vColor;\n"
    "    FragColor = vec4(result, 1.0);\n"
    "}\n";

/* Text vertex shader */
static const char* VERTEX_SHADER_TEXT = 
    "#version 330 core\n"
    "layout(location = 0) in vec2 aPos;\n"
    "layout(location = 1) in vec2 aTexCoord;\n"
    "uniform mat4 uProjection;\n"
    "uniform mat4 uModel;\n"
    "out vec2 vTexCoord;\n"
    "void main() {\n"
    "    gl_Position = uProjection * uModel * vec4(aPos, 0.0, 1.0);\n"
    "    vTexCoord = aTexCoord;\n"
    "}\n";

/* Text fragment shader */
static const char* FRAGMENT_SHADER_TEXT = 
    "#version 330 core\n"
    "in vec2 vTexCoord;\n"
    "out vec4 FragColor;\n"
    "uniform sampler2D uTexture;\n"
    "uniform vec4 uColor;\n"
    "void main() {\n"
    "    float alpha = texture(uTexture, vTexCoord).r;\n"
    "    FragColor = vec4(uColor.rgb, uColor.a * alpha);\n"
    "}\n";

/* ============================================================================
 * OPENGL CANVAS STRUCTURE
 * ============================================================================ */


#define MAX_BATCH_VERTICES 16384

typedef struct {
    VaxpF32 x, y;
    VaxpF32 u, v;
    VaxpF32 r, g, b, a;
    VaxpF32 type, radius, width, height;
} VaxpGLVertex;

typedef struct VaxpGLCanvas {
    VaxpCanvas base;
    
    /* X11/GLX context */
    Display* display;
    Window window;
    GLXContext glx_context;
    Colormap colormap;
    
    /* Shader programs */
    GLuint prog_solid;
    GLuint prog_rounded;
    GLuint prog_circle;
    GLuint prog_3d;
    GLuint prog_text;  /* Text/texture shader */
    
    /* Quad mesh for 2D rendering */
    GLuint quad_vao;
    GLuint quad_vbo;
    
    /* 3D cube mesh */
    GLuint cube_vao;
    GLuint cube_vbo;
    
    /* Font rendering (stb - legacy, for compatibility) */
    GLuint font_texture;
    stbtt_bakedchar font_chars[96];
    int font_loaded;
    float font_size;
    
    /* State stack */
    float transform_stack[32][16];
    VaxpRectF clip_stack[32];
    VaxpRectF current_clip;
    VaxpBool clip_enabled_stack[32];
    VaxpBool clip_enabled;
    int stack_depth;
    
    /* Batching state (for non-text geometry) */
    GLuint prog_uber;
    GLuint uber_vao;
    GLuint uber_vbo;
    VaxpGLVertex batch_vertices[MAX_BATCH_VERTICES];
    int batch_count;
    GLuint current_texture;
    
    /* Current transform */
    float model_matrix[16];
    float projection_matrix[16];
    
    /* NEW: FreeType + HarfBuzz text engine */
    VaxpTextEngine* text_engine;
    VaxpFontId      font_regular;    /* Regular monospace */
    VaxpFontId      font_bold;       /* Bold monospace */
    float           term_font_size;  /* Font size for terminal */
    
} VaxpGLCanvas;

/* ============================================================================
 * MATRIX UTILITIES
 * ============================================================================ */

static void mat4_identity(float* m) {
    memset(m, 0, 16 * sizeof(float));
    m[0] = m[5] = m[10] = m[15] = 1.0f;
}

static void mat4_ortho(float* m, float left, float right, float bottom, float top, float near, float far) {
    memset(m, 0, 16 * sizeof(float));
    m[0] = 2.0f / (right - left);
    m[5] = 2.0f / (top - bottom);
    m[10] = -2.0f / (far - near);
    m[12] = -(right + left) / (right - left);
    m[13] = -(top + bottom) / (top - bottom);
    m[14] = -(far + near) / (far - near);
    m[15] = 1.0f;
}

static void mat4_perspective(float* m, float fov, float aspect, float near, float far) {
    memset(m, 0, 16 * sizeof(float));
    float f = 1.0f / tanf(fov * 0.5f);
    m[0] = f / aspect;
    m[5] = f;
    m[10] = (far + near) / (near - far);
    m[11] = -1.0f;
    m[14] = (2.0f * far * near) / (near - far);
}

static void mat4_translate(float* m, float x, float y, float z) {
    m[12] += m[0]*x + m[4]*y + m[8]*z;
    m[13] += m[1]*x + m[5]*y + m[9]*z;
    m[14] += m[2]*x + m[6]*y + m[10]*z;
}

static void mat4_scale(float* m, float sx, float sy, float sz) {
    m[0] *= sx; m[1] *= sx; m[2] *= sx; m[3] *= sx;
    m[4] *= sy; m[5] *= sy; m[6] *= sy; m[7] *= sy;
    m[8] *= sz; m[9] *= sz; m[10] *= sz; m[11] *= sz;
}

static void mat4_rotate_z(float* m, float angle) {
    float c = cosf(angle);
    float s = sinf(angle);
    float m0 = m[0], m1 = m[1], m2 = m[2], m3 = m[3];
    float m4 = m[4], m5 = m[5], m6 = m[6], m7 = m[7];
    
    m[0] = m0*c + m4*s;
    m[1] = m1*c + m5*s;
    m[2] = m2*c + m6*s;
    m[3] = m3*c + m7*s;
    m[4] = m4*c - m0*s;
    m[5] = m5*c - m1*s;
    m[6] = m6*c - m2*s;
    m[7] = m7*c - m3*s;
}

static void mat4_rotate_x(float* m, float angle) {
    float c = cosf(angle);
    float s = sinf(angle);
    float m4 = m[4], m5 = m[5], m6 = m[6], m7 = m[7];
    float m8 = m[8], m9 = m[9], m10 = m[10], m11 = m[11];
    
    m[4] = m4*c + m8*s;
    m[5] = m5*c + m9*s;
    m[6] = m6*c + m10*s;
    m[7] = m7*c + m11*s;
    m[8] = m8*c - m4*s;
    m[9] = m9*c - m5*s;
    m[10] = m10*c - m6*s;
    m[11] = m11*c - m7*s;
}

static void mat4_rotate_y(float* m, float angle) {
    float c = cosf(angle);
    float s = sinf(angle);
    float m0 = m[0], m1 = m[1], m2 = m[2], m3 = m[3];
    float m8 = m[8], m9 = m[9], m10 = m[10], m11 = m[11];
    
    m[0] = m0*c - m8*s;
    m[1] = m1*c - m9*s;
    m[2] = m2*c - m10*s;
    m[3] = m3*c - m11*s;
    m[8] = m0*s + m8*c;
    m[9] = m1*s + m9*c;
    m[10] = m2*s + m10*c;
    m[11] = m3*s + m11*c;
}

static void mat4_look_at(float* m, float eyeX, float eyeY, float eyeZ,
                          float centerX, float centerY, float centerZ,
                          float upX, float upY, float upZ) {
    float fx = centerX - eyeX, fy = centerY - eyeY, fz = centerZ - eyeZ;
    float len = sqrtf(fx*fx + fy*fy + fz*fz);
    fx /= len; fy /= len; fz /= len;
    
    float sx = fy*upZ - fz*upY, sy = fz*upX - fx*upZ, sz = fx*upY - fy*upX;
    len = sqrtf(sx*sx + sy*sy + sz*sz);
    sx /= len; sy /= len; sz /= len;
    
    float ux = sy*fz - sz*fy, uy = sz*fx - sx*fz, uz = sx*fy - sy*fx;
    
    m[0] = sx;  m[4] = sy;  m[8] = sz;   m[12] = 0;
    m[1] = ux;  m[5] = uy;  m[9] = uz;   m[13] = 0;
    m[2] = -fx; m[6] = -fy; m[10] = -fz; m[14] = 0;
    m[3] = 0;   m[7] = 0;   m[11] = 0;   m[15] = 1;
    
    mat4_translate(m, -eyeX, -eyeY, -eyeZ);
}

static void mat4_copy(float* dst, const float* src) {
    memcpy(dst, src, 16 * sizeof(float));
}

/* ============================================================================
 * SHADER UTILITIES
 * ============================================================================ */

static GLuint compile_shader(GLenum type, const char* source) {
    GLuint shader = glCreateShader(type);
    glShaderSource(shader, 1, &source, NULL);
    glCompileShader(shader);
    
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char log[512];
        glGetShaderInfoLog(shader, sizeof(log), NULL, log);
        fprintf(stderr, "Shader compile error: %s\n", log);
        glDeleteShader(shader);
        return 0;
    }
    return shader;
}

static GLuint create_program(const char* vs_source, const char* fs_source) {
    GLuint vs = compile_shader(GL_VERTEX_SHADER, vs_source);
    GLuint fs = compile_shader(GL_FRAGMENT_SHADER, fs_source);
    if (!vs || !fs) {
        if (vs) glDeleteShader(vs);
        if (fs) glDeleteShader(fs);
        return 0;
    }
    
    GLuint program = glCreateProgram();
    glAttachShader(program, vs);
    glAttachShader(program, fs);
    glLinkProgram(program);
    
    GLint success;
    glGetProgramiv(program, GL_LINK_STATUS, &success);
    if (!success) {
        char log[512];
        glGetProgramInfoLog(program, sizeof(log), NULL, log);
        fprintf(stderr, "Program link error: %s\n", log);
        glDeleteProgram(program);
        program = 0;
    }
    
    glDeleteShader(vs);
    glDeleteShader(fs);
    return program;
}

/* ============================================================================
 * 3D CUBE DATA
 * ============================================================================ */

/* Cube vertices: position (3) + normal (3) + color (3) */
static const float CUBE_VERTICES[] = {
    /* Front face (red) */
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.3f, 0.3f,
     0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.3f, 0.3f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.3f, 0.3f,
     0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.3f, 0.3f,
    -0.5f,  0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.3f, 0.3f,
    -0.5f, -0.5f,  0.5f,  0.0f,  0.0f,  1.0f,  1.0f, 0.3f, 0.3f,
    
    /* Back face (green) */
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.3f, 1.0f, 0.3f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.3f, 1.0f, 0.3f,
     0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.3f, 1.0f, 0.3f,
     0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.3f, 1.0f, 0.3f,
    -0.5f, -0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.3f, 1.0f, 0.3f,
    -0.5f,  0.5f, -0.5f,  0.0f,  0.0f, -1.0f,  0.3f, 1.0f, 0.3f,
    
    /* Left face (blue) */
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.3f, 0.3f, 1.0f,
    -0.5f,  0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.3f, 0.3f, 1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.3f, 0.3f, 1.0f,
    -0.5f, -0.5f, -0.5f, -1.0f,  0.0f,  0.0f,  0.3f, 0.3f, 1.0f,
    -0.5f, -0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.3f, 0.3f, 1.0f,
    -0.5f,  0.5f,  0.5f, -1.0f,  0.0f,  0.0f,  0.3f, 0.3f, 1.0f,
    
    /* Right face (yellow) */
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.3f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.3f,
     0.5f,  0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.3f,
     0.5f, -0.5f, -0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.3f,
     0.5f,  0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.3f,
     0.5f, -0.5f,  0.5f,  1.0f,  0.0f,  0.0f,  1.0f, 1.0f, 0.3f,
    
    /* Bottom face (cyan) */
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.3f, 1.0f, 1.0f,
     0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.3f, 1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.3f, 1.0f, 1.0f,
     0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.3f, 1.0f, 1.0f,
    -0.5f, -0.5f,  0.5f,  0.0f, -1.0f,  0.0f,  0.3f, 1.0f, 1.0f,
    -0.5f, -0.5f, -0.5f,  0.0f, -1.0f,  0.0f,  0.3f, 1.0f, 1.0f,
    
    /* Top face (magenta) */
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.3f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.3f, 1.0f,
     0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.3f, 1.0f,
     0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.3f, 1.0f,
    -0.5f,  0.5f, -0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.3f, 1.0f,
    -0.5f,  0.5f,  0.5f,  0.0f,  1.0f,  0.0f,  1.0f, 0.3f, 1.0f,
};

/* ============================================================================
 * CANVAS OPERATIONS
 * ============================================================================ */

/* Make this canvas's context current before any GL operations */
static void gl_batch_flush(VaxpGLCanvas* c);
static void gl_make_current(VaxpGLCanvas* c) {
    glXMakeCurrent(c->display, c->window, c->glx_context);
}

/* Load font and create texture atlas */


static void gl_canvas_destroy(VaxpCanvas* canvas) {
    VaxpGLCanvas* c = (VaxpGLCanvas*)canvas;
    gl_make_current(c);
    
    if (c->prog_solid) glDeleteProgram(c->prog_solid);
    if (c->prog_rounded) glDeleteProgram(c->prog_rounded);
    if (c->prog_circle) glDeleteProgram(c->prog_circle);
    if (c->prog_3d) glDeleteProgram(c->prog_3d);
    if (c->quad_vao) glDeleteVertexArrays(1, &c->quad_vao);
    if (c->quad_vbo) glDeleteBuffers(1, &c->quad_vbo);
    if (c->cube_vao) glDeleteVertexArrays(1, &c->cube_vao);
    if (c->cube_vbo) glDeleteBuffers(1, &c->cube_vbo);
    
    /* Destroy text engine (frees FreeType, HarfBuzz, atlas texture - zero leaks) */
    vaxp_text_engine_destroy(c->text_engine);
    c->text_engine = NULL;
    
    if (c->glx_context) {
        glXMakeCurrent(c->display, None, NULL);
        glXDestroyContext(c->display, c->glx_context);
    }
}


static void gl_canvas_save(VaxpCanvas* canvas) {
    VaxpGLCanvas* c = (VaxpGLCanvas*)canvas;
    if (c->stack_depth < 31) {
        mat4_copy(c->transform_stack[c->stack_depth], c->model_matrix);
        c->clip_stack[c->stack_depth] = c->current_clip;
        c->clip_enabled_stack[c->stack_depth] = c->clip_enabled;
        c->stack_depth++;
    }
}

static void gl_canvas_restore(VaxpCanvas* canvas) {
    VaxpGLCanvas* c = (VaxpGLCanvas*)canvas;
    if (c->stack_depth > 0) {
        c->stack_depth--;
        mat4_copy(c->model_matrix, c->transform_stack[c->stack_depth]);
        gl_batch_flush(c);
        c->current_clip = c->clip_stack[c->stack_depth];
        c->clip_enabled = c->clip_enabled_stack[c->stack_depth];
        
        if (c->clip_enabled) {
            glScissor((GLint)c->current_clip.x, (GLint)c->current_clip.y, 
                      (GLsizei)c->current_clip.width, (GLsizei)c->current_clip.height);
            glEnable(GL_SCISSOR_TEST);
        } else {
            glDisable(GL_SCISSOR_TEST);
        }
    }
}

static void gl_canvas_translate(VaxpCanvas* canvas, VaxpF32 dx, VaxpF32 dy) {
    VaxpGLCanvas* c = (VaxpGLCanvas*)canvas;
    mat4_translate(c->model_matrix, dx, dy, 0.0f);
}

static void gl_canvas_scale(VaxpCanvas* canvas, VaxpF32 sx, VaxpF32 sy) {
    VaxpGLCanvas* c = (VaxpGLCanvas*)canvas;
    mat4_scale(c->model_matrix, sx, sy, 1.0f);
}

static void gl_canvas_rotate(VaxpCanvas* canvas, VaxpF32 degrees) {
    VaxpGLCanvas* c = (VaxpGLCanvas*)canvas;
    mat4_rotate_z(c->model_matrix, degrees * (float)M_PI / 180.0f);
}

static void gl_canvas_clip_rect(VaxpCanvas* canvas, VaxpRectF rect) {
    VaxpGLCanvas* c = (VaxpGLCanvas*)canvas;
    
    /* Calculate absolute window coordinates */
    /* rect is in local coordinates. c->model_matrix has translation in [12] and [13]. */
    VaxpF32 abs_x = rect.x + c->model_matrix[12];
    VaxpF32 abs_y = rect.y + c->model_matrix[13];
    
    /* glScissor expects origin at bottom-left */
    VaxpF32 scissor_y = canvas->height - (abs_y + rect.height);
    
    VaxpRectF new_clip = { abs_x, scissor_y, rect.width, rect.height };
    
    /* Intersect with current clip if there is one */
    if (c->clip_enabled) {
        VaxpF32 x1 = VAXP_MAX(c->current_clip.x, new_clip.x);
        VaxpF32 y1 = VAXP_MAX(c->current_clip.y, new_clip.y);
        VaxpF32 x2 = VAXP_MIN(c->current_clip.x + c->current_clip.width, new_clip.x + new_clip.width);
        VaxpF32 y2 = VAXP_MIN(c->current_clip.y + c->current_clip.height, new_clip.y + new_clip.height);
        
        new_clip.x = x1;
        new_clip.y = y1;
        new_clip.width = x2 > x1 ? x2 - x1 : 0;
        new_clip.height = y2 > y1 ? y2 - y1 : 0;
    }
    
    gl_batch_flush(c);
    c->current_clip = new_clip;
    c->clip_enabled = VAXP_TRUE;
    
    glScissor((GLint)c->current_clip.x, (GLint)c->current_clip.y, 
              (GLsizei)c->current_clip.width, (GLsizei)c->current_clip.height);
    glEnable(GL_SCISSOR_TEST);
}

static void gl_canvas_clip_rounded_rect(VaxpCanvas* canvas, VaxpRectF rect, VaxpF32 radius) {
    /* For simplicity, use regular clipping. Full implementation would use stencil. */
    gl_canvas_clip_rect(canvas, rect);
    (void)radius;
}


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

static void gl_canvas_clear(VaxpCanvas* canvas, VaxpColor color) {
    VaxpGLCanvas* c = (VaxpGLCanvas*)canvas;
    gl_make_current(c);
    
    glDisable(GL_SCISSOR_TEST);
    c->clip_enabled = VAXP_FALSE;
    c->stack_depth = 0;
    mat4_identity(c->model_matrix);
    mat4_identity(c->projection_matrix);
    mat4_ortho(c->projection_matrix, 0.0f, canvas->width, canvas->height, 0.0f, -1.0f, 1.0f);
    glViewport(0, 0, canvas->width, canvas->height);
    
    glClearColor(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}



static void gl_canvas_draw_rect(VaxpCanvas* canvas, VaxpRectF rect, const VaxpPaint* paint) {
    VaxpGLCanvas* c = (VaxpGLCanvas*)canvas;
    VaxpF32 pts[8] = {rect.x, rect.y, rect.x+rect.width, rect.y, rect.x+rect.width, rect.y+rect.height, rect.x, rect.y+rect.height};
    VaxpF32 uvs[8] = {0,0, 0,0, 0,0, 0,0};
    gl_batch_push_quad(c, 0.0f, 0.0f, 0.0f, 0.0f, pts, uvs, paint->color, 0);
}

static void gl_canvas_draw_rounded_rect(VaxpCanvas* canvas, VaxpRectF rect, VaxpF32 radius, const VaxpPaint* paint) {
    VaxpGLCanvas* c = (VaxpGLCanvas*)canvas;
    VaxpF32 pts[8] = {rect.x, rect.y, rect.x+rect.width, rect.y, rect.x+rect.width, rect.y+rect.height, rect.x, rect.y+rect.height};
    VaxpF32 uvs[8] = {0,0, rect.width,0, rect.width,rect.height, 0,rect.height};
    gl_batch_push_quad(c, 2.0f, radius, rect.width, rect.height, pts, uvs, paint->color, 0);
}

static void gl_canvas_draw_circle(VaxpCanvas* canvas, VaxpF32 cx, VaxpF32 cy, VaxpF32 radius, const VaxpPaint* paint) {
    VaxpGLCanvas* c = (VaxpGLCanvas*)canvas;
    VaxpF32 pts[8] = {cx-radius, cy-radius, cx+radius, cy-radius, cx+radius, cy+radius, cx-radius, cy+radius};
    VaxpF32 uvs[8] = {0,0, radius*2,0, radius*2,radius*2, 0,radius*2};
    gl_batch_push_quad(c, 2.0f, radius, radius*2, radius*2, pts, uvs, paint->color, 0);
}

static void gl_canvas_draw_oval(VaxpCanvas* canvas, VaxpRectF rect, const VaxpPaint* paint) {
    /* Draw as stretched circle */
    VaxpF32 cx = rect.x + rect.width / 2.0f;
    VaxpF32 cy = rect.y + rect.height / 2.0f;
    VaxpF32 r = VAXP_MIN(rect.width, rect.height) / 2.0f;
    gl_canvas_draw_circle(canvas, cx, cy, r, paint);
}

static void gl_canvas_draw_line(VaxpCanvas* canvas, VaxpF32 x1, VaxpF32 y1, 
                                 VaxpF32 x2, VaxpF32 y2, const VaxpPaint* paint) {
    VaxpGLCanvas* c = (VaxpGLCanvas*)canvas;
    
    /* Create line as thin rectangle */
    float dx = x2 - x1, dy = y2 - y1;
    float len = sqrtf(dx*dx + dy*dy);
    float thick = paint->stroke_width * 0.5f;
    float nx = -dy / len * thick, ny = dx / len * thick;
    
    float vertices[] = {
        x1 + nx, y1 + ny, 0.0f, 0.0f,
        x1 - nx, y1 - ny, 1.0f, 0.0f,
        x2 - nx, y2 - ny, 1.0f, 1.0f,
        x1 + nx, y1 + ny, 0.0f, 0.0f,
        x2 - nx, y2 - ny, 1.0f, 1.0f,
        x2 + nx, y2 + ny, 0.0f, 1.0f,
    };
    
    glUseProgram(c->prog_solid);
    glUniformMatrix4fv(glGetUniformLocation(c->prog_solid, "uProjection"), 
                       1, GL_FALSE, c->projection_matrix);
    glUniformMatrix4fv(glGetUniformLocation(c->prog_solid, "uModel"), 
                       1, GL_FALSE, c->model_matrix);
    glUniform4f(glGetUniformLocation(c->prog_solid, "uColor"),
                paint->color.r / 255.0f, paint->color.g / 255.0f,
                paint->color.b / 255.0f, paint->color.a / 255.0f);
    
    glBindVertexArray(c->quad_vao);
    glBindBuffer(GL_ARRAY_BUFFER, c->quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

static void gl_canvas_draw_path(VaxpCanvas* canvas, const VaxpPath* path, const VaxpPaint* paint) {
    /* Path rendering requires tessellation - not implemented for simplicity */
    (void)canvas; (void)path; (void)paint;
}

static void gl_canvas_draw_text(VaxpCanvas* canvas, const char* text, VaxpF32 x, VaxpF32 y,
                                 const VaxpFont* font, const VaxpPaint* paint) {
    VaxpGLCanvas* c = (VaxpGLCanvas*)canvas;
    if (!text || !*text || !c->text_engine) return;
    
    float size_px = (font && font->size > 0) ? font->size : 14.0f;
    bool bold = (font && font->bold);
    const char* family = (font && font->family && font->family[0]) ? font->family : "monospace";
    
    /* Ensure font is loaded for this size */
    VaxpFontId fid = vaxp_text_engine_load_font_by_name(c->text_engine, family, size_px, bold, false);
    if (fid == VAXP_FONT_INVALID) return;
    
    /* Flush geometry batch before text engine draws */
    gl_batch_flush(c);
    
    /* Set text engine transform to match canvas */
    vaxp_text_engine_set_transform(c->text_engine, c->model_matrix);
    
    VaxpTextColor color = {paint->color.r, paint->color.g, paint->color.b, paint->color.a};
    vaxp_text_engine_draw_string(c->text_engine, fid, text, x, y, color);
    vaxp_text_engine_flush(c->text_engine);
}

/* 
 * Terminal GPU Batching via FreeType+HarfBuzz text engine
 */
static void gl_canvas_draw_terminal_cells(VaxpCanvas* canvas, const VaxpCanvasTextCell* cells, int count, 
                                          VaxpF32 start_x, VaxpF32 y, VaxpF32 cell_w, VaxpF32 cell_h, 
                                          const VaxpFont* font, VaxpColor default_bg, const char* match_mask) {
    VaxpGLCanvas* c = (VaxpGLCanvas*)canvas;
    if (!c->text_engine || !cells || count <= 0) return;
    
    float size_px = (font && font->size > 0) ? font->size : 14.0f;
    
    /* Load fonts if size changed */
    if (c->font_regular == VAXP_FONT_INVALID || fabsf(c->term_font_size - size_px) > 0.5f) {
        c->font_regular = vaxp_text_engine_load_font_by_name(c->text_engine, "monospace", size_px, false, false);
        c->font_bold    = vaxp_text_engine_load_font_by_name(c->text_engine, "monospace", size_px, true,  false);
        c->term_font_size = size_px;
    }
    
    /* Convert VaxpCanvasTextCell → VaxpTextCell */
    VaxpTextCell* te_cells = (VaxpTextCell*)malloc(count * sizeof(VaxpTextCell));
    for (int i = 0; i < count; i++) {
        memcpy(te_cells[i].ch, cells[i].ch, 5);
        te_cells[i].fg.r = cells[i].fg.r;
        te_cells[i].fg.g = cells[i].fg.g;
        te_cells[i].fg.b = cells[i].fg.b;
        te_cells[i].fg.a = cells[i].fg.a;
        te_cells[i].flags = cells[i].flags;
        /* Apply search match: invert fg to black on yellow */
        if (match_mask && match_mask[i]) {
            te_cells[i].fg.r = 0;
            te_cells[i].fg.g = 0;
            te_cells[i].fg.b = 0;
            te_cells[i].fg.a = 255;
        }
    }
    
    /* Flush geometry batch before text engine */
    gl_batch_flush(c);
    
    /* Set text engine transform to match canvas */
    vaxp_text_engine_set_transform(c->text_engine, c->model_matrix);
    
    vaxp_text_engine_draw_cells(c->text_engine,
                                 c->font_regular, c->font_bold,
                                 te_cells, count,
                                 start_x, y, cell_w, cell_h);
    vaxp_text_engine_flush(c->text_engine);
    
    free(te_cells);
}

static void gl_canvas_draw_image(VaxpCanvas* canvas, const VaxpImage* image, VaxpF32 x, VaxpF32 y) {
    if (!image) return;
    VaxpGLCanvas* c = (VaxpGLCanvas*)canvas;
    VaxpImageData* img = (VaxpImageData*)image;
    
    if (img->gl_texture == 0) {
        GLuint tex;
        glGenTextures(1, &tex);
        glBindTexture(GL_TEXTURE_2D, tex);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
        
        /* Premultiplied BGRA data from Cairo to OpenGL */
        /* Assuming Cairo uses BGRA format internally */
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, img->width, img->height, 0, GL_BGRA, GL_UNSIGNED_BYTE, img->pixels);
        img->gl_texture = tex;
    }
    
    VaxpF32 w = img->width;
    VaxpF32 h = img->height;
    
    VaxpF32 pts[8] = {x, y, x+w, y, x+w, y+h, x, y+h};
    VaxpF32 uvs[8] = {0,0, 1,0, 1,1, 0,1};
    
    gl_batch_push_quad(c, 1.0f, 0.0f, 0.0f, 0.0f, pts, uvs, vaxp_color_rgba(255,255,255,255), img->gl_texture);
}

static void gl_canvas_draw_image_rect(VaxpCanvas* canvas, const VaxpImage* image,
                                       VaxpRectF src, VaxpRectF dst, const VaxpPaint* paint) {
    (void)canvas; (void)image; (void)src; (void)dst; (void)paint;
    /* TODO: texture rendering */
}

static void gl_canvas_flush(VaxpCanvas* canvas) {
    gl_batch_flush((VaxpGLCanvas*)canvas);
    VaxpGLCanvas* c = (VaxpGLCanvas*)canvas;
    gl_make_current(c);
    glFlush();
    glXSwapBuffers(c->display, c->window);
}

static VaxpSize2D gl_canvas_get_size(VaxpCanvas* canvas) {
    return (VaxpSize2D){ .width = canvas->width, .height = canvas->height };
}

/* ============================================================================
 * CANVAS VTABLE
 * ============================================================================ */

static const VaxpCanvasOps gl_canvas_ops = {
    .destroy = gl_canvas_destroy,
    .save = gl_canvas_save,
    .restore = gl_canvas_restore,
    .translate = gl_canvas_translate,
    .scale = gl_canvas_scale,
    .rotate = gl_canvas_rotate,
    .clip_rect = gl_canvas_clip_rect,
    .clip_rounded_rect = gl_canvas_clip_rounded_rect,
    .clear = gl_canvas_clear,
    .draw_rect = gl_canvas_draw_rect,
    .draw_rounded_rect = gl_canvas_draw_rounded_rect,
    .draw_circle = gl_canvas_draw_circle,
    .draw_oval = gl_canvas_draw_oval,
    .draw_line = gl_canvas_draw_line,
    .draw_path = gl_canvas_draw_path,
    .draw_text = gl_canvas_draw_text,
    .draw_terminal_cells = gl_canvas_draw_terminal_cells,
    .draw_image = gl_canvas_draw_image,
    .draw_image_rect = gl_canvas_draw_image_rect,
    .flush = gl_canvas_flush,
    .get_size = gl_canvas_get_size,
};

/* ============================================================================
 * CANVAS CREATION - PUBLIC API
 * ============================================================================ */

static void canvas_destructor(void* self) {
    gl_canvas_destroy((VaxpCanvas*)self);
}

VaxpResultPtr vaxp_canvas_create_opengl(Display* display, Window window, 
                                           VaxpU32 width, VaxpU32 height) {
    /* Disable V-Sync for unlocked FPS */
    setenv("vblank_mode", "0", 1);
    
    /* Find a suitable visual */
    static int visual_attribs[] = {
        GLX_X_RENDERABLE, True,
        GLX_DRAWABLE_TYPE, GLX_WINDOW_BIT,
        GLX_RENDER_TYPE, GLX_RGBA_BIT,
        GLX_X_VISUAL_TYPE, GLX_TRUE_COLOR,
        GLX_RED_SIZE, 8,
        GLX_GREEN_SIZE, 8,
        GLX_BLUE_SIZE, 8,
        GLX_ALPHA_SIZE, 8,
        GLX_DEPTH_SIZE, 24,
        GLX_STENCIL_SIZE, 8,
        GLX_DOUBLEBUFFER, True,
        None
    };
    
    int fbcount;
    GLXFBConfig* fbc = glXChooseFBConfig(display, DefaultScreen(display), visual_attribs, &fbcount);
    if (!fbc || fbcount == 0) {
        fprintf(stderr, "Failed to find suitable framebuffer config\n");
        return VAXP_ERR_PTR(VAXP_ERROR_SURFACE_CREATE);
    }
    
    /* Get window's visual ID */
    XWindowAttributes window_attrs;
    XGetWindowAttributes(display, window, &window_attrs);
    VisualID window_visual_id = XVisualIDFromVisual(window_attrs.visual);
    
    GLXFBConfig best_fbc = NULL;
    for (int i = 0; i < fbcount; i++) {
        XVisualInfo* vi = glXGetVisualFromFBConfig(display, fbc[i]);
        if (vi) {
            if (vi->visualid == window_visual_id) {
                best_fbc = fbc[i];
                XFree(vi);
                break;
            }
            XFree(vi);
        }
    }
    
    if (!best_fbc) {
        best_fbc = fbc[0]; /* Fallback */
        XVisualInfo* fb_vi = glXGetVisualFromFBConfig(display, best_fbc);
        fprintf(stderr, "VAXPUI GL: No exact FBConfig match! Window Visual: %lu, Fallback FBConfig Visual: %lu\n", 
                (unsigned long)window_visual_id, fb_vi ? (unsigned long)fb_vi->visualid : 0);
        if (fb_vi) XFree(fb_vi);
    } else {
        fprintf(stderr, "VAXPUI GL: Matched FBConfig perfectly with Window Visual ID: %lu\n", (unsigned long)window_visual_id);
    }
    XFree(fbc);
    
    /* Create OpenGL 3.3 Core context */
    typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
    glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 
        (glXCreateContextAttribsARBProc)glXGetProcAddress((const GLubyte*)"glXCreateContextAttribsARB");
    
    if (!glXCreateContextAttribsARB) {
        fprintf(stderr, "glXCreateContextAttribsARB not available\n");
        return VAXP_ERR_PTR(VAXP_ERROR_CANVAS_CREATE);
    }
    
    int context_attribs[] = {
        GLX_CONTEXT_MAJOR_VERSION_ARB, 3,
        GLX_CONTEXT_MINOR_VERSION_ARB, 3,
        GLX_CONTEXT_PROFILE_MASK_ARB, GLX_CONTEXT_CORE_PROFILE_BIT_ARB,
        None
    };
    
    GLXContext glx_context = glXCreateContextAttribsARB(display, best_fbc, NULL, True, context_attribs);
    if (!glx_context) {
        fprintf(stderr, "Failed to create OpenGL 3.3 context\n");
        return VAXP_ERR_PTR(VAXP_ERROR_CANVAS_CREATE);
    }
    
    /* Make context current */
    if (!glXMakeCurrent(display, window, glx_context)) {
        glXDestroyContext(display, glx_context);
        return VAXP_ERR_PTR(VAXP_ERROR_CANVAS_CREATE);
    }
    
    /* Load GL functions */
    if (!load_gl_functions()) {
        glXDestroyContext(display, glx_context);
        return VAXP_ERR_PTR(VAXP_ERROR_CANVAS_CREATE);
    }
    
    printf("OpenGL Version: %s\n", glGetString(GL_VERSION));
    printf("OpenGL Renderer: %s\n", glGetString(GL_RENDERER));
    
    /* Create canvas */
    VaxpGLCanvas* canvas = (VaxpGLCanvas*)VAXP_REF_NEW(VaxpGLCanvas, canvas_destructor);
    if (!canvas) {
        glXDestroyContext(display, glx_context);
        return VAXP_ERR_PTR(VAXP_ERROR_OUT_OF_MEMORY);
    }
    
    canvas->base.ops = &gl_canvas_ops;
    canvas->base.width = width;
    canvas->base.height = height;
    canvas->display = display;
    canvas->window = window;
    canvas->glx_context = glx_context;
    canvas->stack_depth = 0;
    
    /* Initialize matrices */
    mat4_identity(canvas->model_matrix);
    mat4_ortho(canvas->projection_matrix, 0.0f, (float)width, (float)height, 0.0f, -1.0f, 1.0f);
    
    /* Create shaders */
    canvas->prog_solid = create_program(VERTEX_SHADER_2D, FRAGMENT_SHADER_SOLID);
    canvas->prog_rounded = create_program(VERTEX_SHADER_2D, FRAGMENT_SHADER_ROUNDED_RECT);
    canvas->prog_circle = create_program(VERTEX_SHADER_2D, FRAGMENT_SHADER_CIRCLE);
    canvas->prog_3d = create_program(VERTEX_SHADER_3D, FRAGMENT_SHADER_3D);
    canvas->prog_text = create_program(VERTEX_SHADER_TEXT, FRAGMENT_SHADER_TEXT);

    /* Create Uber Shader for Batching */
    canvas->prog_uber = create_program(VERTEX_SHADER_UBER, FRAGMENT_SHADER_UBER);
    glGenVertexArrays(1, &canvas->uber_vao);
    glGenBuffers(1, &canvas->uber_vbo);
    glBindVertexArray(canvas->uber_vao);
    glBindBuffer(GL_ARRAY_BUFFER, canvas->uber_vbo);
    glBufferData(GL_ARRAY_BUFFER, MAX_BATCH_VERTICES * sizeof(VaxpGLVertex), NULL, GL_DYNAMIC_DRAW);
    
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, sizeof(VaxpGLVertex), (void*)offsetof(VaxpGLVertex, x));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(VaxpGLVertex), (void*)offsetof(VaxpGLVertex, u));
    glEnableVertexAttribArray(2);
    glVertexAttribPointer(2, 4, GL_FLOAT, GL_FALSE, sizeof(VaxpGLVertex), (void*)offsetof(VaxpGLVertex, r));
    glEnableVertexAttribArray(3);
    glVertexAttribPointer(3, 4, GL_FLOAT, GL_FALSE, sizeof(VaxpGLVertex), (void*)offsetof(VaxpGLVertex, type));

    
    /* Initialize font state */
    canvas->font_loaded = 0;
    canvas->font_texture = 0;
    
    if (!canvas->prog_solid || !canvas->prog_rounded || !canvas->prog_circle || !canvas->prog_3d) {
        fprintf(stderr, "Failed to create shader programs\n");
        vaxp_unref(canvas);
        return VAXP_ERR_PTR(VAXP_ERROR_CANVAS_CREATE);
    }
    
    /* Create quad VAO/VBO */
    glGenVertexArrays(1, &canvas->quad_vao);
    glGenBuffers(1, &canvas->quad_vbo);
    
    glBindVertexArray(canvas->quad_vao);
    glBindBuffer(GL_ARRAY_BUFFER, canvas->quad_vbo);
    
    glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
    glEnableVertexAttribArray(1);
    
    /* Create cube VAO/VBO */
    glGenVertexArrays(1, &canvas->cube_vao);
    glGenBuffers(1, &canvas->cube_vbo);
    
    glBindVertexArray(canvas->cube_vao);
    glBindBuffer(GL_ARRAY_BUFFER, canvas->cube_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(CUBE_VERTICES), CUBE_VERTICES, GL_STATIC_DRAW);
    
    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);
    glVertexAttribPointer(2, 3, GL_FLOAT, GL_FALSE, 9 * sizeof(float), (void*)(6 * sizeof(float)));
    glEnableVertexAttribArray(2);
    
    /* Setup OpenGL state */
    glEnable(GL_BLEND);
    glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
    glViewport(0, 0, width, height);
    
    /* Create FreeType + HarfBuzz text engine (replaces Pango/Cairo completely) */
    canvas->text_engine   = vaxp_text_engine_create();
    canvas->font_regular  = VAXP_FONT_INVALID;
    canvas->font_bold     = VAXP_FONT_INVALID;
    canvas->term_font_size = 0.0f;
    
    if (!canvas->text_engine) {
        fprintf(stderr, "[VAXPUI] Warning: Failed to create text engine\n");
    }
    
    return VAXP_OK_PTR(canvas);
}

/* ============================================================================
 * 3D DRAWING API - PUBLIC
 * ============================================================================ */

void vaxp_gl_draw_cube(VaxpCanvas* canvas, float x, float y, float z, 
                        float size, float rotX, float rotY, float rotZ) {
    VaxpGLCanvas* c = (VaxpGLCanvas*)canvas;
    if (!c || c->base.ops != &gl_canvas_ops) return;
    
    /* Setup 3D projection */
    float projection[16];
    mat4_perspective(projection, 45.0f * (float)M_PI / 180.0f, 
                     (float)c->base.width / (float)c->base.height, 0.1f, 100.0f);
    
    /* View matrix - camera looking at origin */
    float view[16];
    mat4_look_at(view, 0.0f, 0.0f, 5.0f, 0.0f, 0.0f, 0.0f, 0.0f, 1.0f, 0.0f);
    
    /* Model matrix - position and rotate cube */
    float model[16];
    mat4_identity(model);
    mat4_translate(model, x, y, z);
    mat4_rotate_x(model, rotX);
    mat4_rotate_y(model, rotY);
    mat4_rotate_z(model, rotZ);
    mat4_scale(model, size, size, size);
    
    glEnable(GL_DEPTH_TEST);
    
    glUseProgram(c->prog_3d);
    glUniformMatrix4fv(glGetUniformLocation(c->prog_3d, "uProjection"), 1, GL_FALSE, projection);
    glUniformMatrix4fv(glGetUniformLocation(c->prog_3d, "uView"), 1, GL_FALSE, view);
    glUniformMatrix4fv(glGetUniformLocation(c->prog_3d, "uModel"), 1, GL_FALSE, model);
    glUniform3f(glGetUniformLocation(c->prog_3d, "uLightPos"), 2.0f, 2.0f, 3.0f);
    glUniform3f(glGetUniformLocation(c->prog_3d, "uLightColor"), 1.0f, 1.0f, 1.0f);
    glUniform3f(glGetUniformLocation(c->prog_3d, "uViewPos"), 0.0f, 0.0f, 5.0f);
    
    glBindVertexArray(c->cube_vao);
    glDrawArrays(GL_TRIANGLES, 0, 36);
    
    glDisable(GL_DEPTH_TEST);
}

void vaxp_gl_begin_3d(VaxpCanvas* canvas) {
    (void)canvas;
    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void vaxp_gl_end_3d(VaxpCanvas* canvas) {
    (void)canvas;
    glDisable(GL_DEPTH_TEST);
}
