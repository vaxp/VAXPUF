/*
 * VENOMUI - High Performance GUI Framework
 * 
 * venom_canvas_opengl.c - OpenGL 3.3 Core Profile implementation of canvas
 * 
 * Hardware-accelerated 2D/3D rendering with GLSL shaders.
 */

#define _USE_MATH_DEFINES
#include <math.h>

#ifndef M_PI
#define M_PI 3.14159265358979323846
#endif

#include "venom/graphics/venom_canvas.h"
#include "venom/core/venom_memory.h"

#include <X11/Xlib.h>
#include <X11/Xutil.h>
#include <GL/gl.h>
#include <GL/glx.h>
#include <GL/glext.h>
#include <stdio.h>
#include <string.h>
#include <stdlib.h>

/* stb_truetype for font rendering */
#define STB_TRUETYPE_IMPLEMENTATION
#include "venom/third_party/stb_truetype.h"

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
static PFNGLDELETEBUFFERSPROC glDeleteBuffers = NULL;
static PFNGLVERTEXATTRIBPOINTERPROC glVertexAttribPointer = NULL;
static PFNGLENABLEVERTEXATTRIBARRAYPROC glEnableVertexAttribArray = NULL;

/* Framebuffer functions */
static PFNGLGENFRAMEBUFFERSPROC glGenFramebuffers = NULL;
static PFNGLBINDFRAMEBUFFERPROC glBindFramebuffer = NULL;
static PFNGLDELETEFRAMEBUFFERSPROC glDeleteFramebuffers = NULL;
static PFNGLFRAMEBUFFERTEXTURE2DPROC glFramebufferTexture2D = NULL;
static PFNGLCHECKFRAMEBUFFERSTATUSPROC glCheckFramebufferStatus = NULL;

static VenomBool gl_functions_loaded = VENOM_FALSE;

static VenomBool load_gl_functions(void) {
    if (gl_functions_loaded) return VENOM_TRUE;
    
    #define LOAD_GL(name) do { \
        name = (void*)glXGetProcAddress((const GLubyte*)#name); \
        if (!name) { fprintf(stderr, "Failed to load " #name "\n"); return VENOM_FALSE; } \
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
    LOAD_GL(glDeleteBuffers);
    LOAD_GL(glVertexAttribPointer);
    LOAD_GL(glEnableVertexAttribArray);
    LOAD_GL(glGenFramebuffers);
    LOAD_GL(glBindFramebuffer);
    LOAD_GL(glDeleteFramebuffers);
    LOAD_GL(glFramebufferTexture2D);
    LOAD_GL(glCheckFramebufferStatus);
    
    #undef LOAD_GL
    
    gl_functions_loaded = VENOM_TRUE;
    return VENOM_TRUE;
}

/* ============================================================================
 * SHADER SOURCES
 * ============================================================================ */

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
    "out vec2 vTexCoord;\n"
    "void main() {\n"
    "    gl_Position = uProjection * vec4(aPos, 0.0, 1.0);\n"
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

typedef struct VenomGLCanvas {
    VenomCanvas base;
    
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
    
    /* Font rendering */
    GLuint font_texture;
    stbtt_bakedchar font_chars[96];  /* ASCII 32-127 */
    int font_loaded;
    float font_size;
    
    /* State stack */
    float transform_stack[32][16]; /* 4x4 matrices */
    int stack_depth;
    
    /* Current transform */
    float model_matrix[16];
    float projection_matrix[16];
    
} VenomGLCanvas;

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
static void gl_make_current(VenomGLCanvas* c) {
    glXMakeCurrent(c->display, c->window, c->glx_context);
}

/* Load font and create texture atlas */
static int gl_load_font(VenomGLCanvas* c, const char* font_path, float font_size) {
    FILE* f = fopen(font_path, "rb");
    if (!f) {
        fprintf(stderr, "Could not open font: %s\n", font_path);
        return 0;
    }
    
    fseek(f, 0, SEEK_END);
    long size = ftell(f);
    fseek(f, 0, SEEK_SET);
    
    unsigned char* font_data = (unsigned char*)malloc(size);
    if (!font_data) {
        fclose(f);
        return 0;
    }
    
    if (fread(font_data, 1, size, f) != (size_t)size) {
        free(font_data);
        fclose(f);
        return 0;
    }
    fclose(f);
    
    /* Create bitmap for font */
    int tex_w = 512, tex_h = 512;
    unsigned char* bitmap = (unsigned char*)calloc(tex_w * tex_h, 1);
    
    stbtt_BakeFontBitmap(font_data, 0, font_size, bitmap, tex_w, tex_h, 
                         32, 96, c->font_chars);
    
    free(font_data);
    
    /* Create OpenGL texture */
    glGenTextures(1, &c->font_texture);
    glBindTexture(GL_TEXTURE_2D, c->font_texture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, tex_w, tex_h, 0, 
                 GL_RED, GL_UNSIGNED_BYTE, bitmap);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    free(bitmap);
    
    c->font_size = font_size;
    c->font_loaded = 1;
    printf("Font loaded: %s (%.0fpx)\n", font_path, font_size);
    return 1;
}

static void gl_canvas_destroy(VenomCanvas* canvas) {
    VenomGLCanvas* c = (VenomGLCanvas*)canvas;
    gl_make_current(c);
    
    if (c->prog_solid) glDeleteProgram(c->prog_solid);
    if (c->prog_rounded) glDeleteProgram(c->prog_rounded);
    if (c->prog_circle) glDeleteProgram(c->prog_circle);
    if (c->prog_3d) glDeleteProgram(c->prog_3d);
    if (c->quad_vao) glDeleteVertexArrays(1, &c->quad_vao);
    if (c->quad_vbo) glDeleteBuffers(1, &c->quad_vbo);
    if (c->cube_vao) glDeleteVertexArrays(1, &c->cube_vao);
    if (c->cube_vbo) glDeleteBuffers(1, &c->cube_vbo);
    
    if (c->glx_context) {
        glXMakeCurrent(c->display, None, NULL);
        glXDestroyContext(c->display, c->glx_context);
    }
}

static void gl_canvas_save(VenomCanvas* canvas) {
    VenomGLCanvas* c = (VenomGLCanvas*)canvas;
    if (c->stack_depth < 31) {
        mat4_copy(c->transform_stack[c->stack_depth], c->model_matrix);
        c->stack_depth++;
    }
}

static void gl_canvas_restore(VenomCanvas* canvas) {
    VenomGLCanvas* c = (VenomGLCanvas*)canvas;
    if (c->stack_depth > 0) {
        c->stack_depth--;
        mat4_copy(c->model_matrix, c->transform_stack[c->stack_depth]);
    }
}

static void gl_canvas_translate(VenomCanvas* canvas, VenomF32 dx, VenomF32 dy) {
    VenomGLCanvas* c = (VenomGLCanvas*)canvas;
    mat4_translate(c->model_matrix, dx, dy, 0.0f);
}

static void gl_canvas_scale(VenomCanvas* canvas, VenomF32 sx, VenomF32 sy) {
    VenomGLCanvas* c = (VenomGLCanvas*)canvas;
    mat4_scale(c->model_matrix, sx, sy, 1.0f);
}

static void gl_canvas_rotate(VenomCanvas* canvas, VenomF32 degrees) {
    VenomGLCanvas* c = (VenomGLCanvas*)canvas;
    mat4_rotate_z(c->model_matrix, degrees * (float)M_PI / 180.0f);
}

static void gl_canvas_clip_rect(VenomCanvas* canvas, VenomRectF rect) {
    glScissor((GLint)rect.x, (GLint)rect.y, (GLsizei)rect.width, (GLsizei)rect.height);
    glEnable(GL_SCISSOR_TEST);
    (void)canvas;
}

static void gl_canvas_clip_rounded_rect(VenomCanvas* canvas, VenomRectF rect, VenomF32 radius) {
    /* For simplicity, use regular clipping. Full implementation would use stencil. */
    gl_canvas_clip_rect(canvas, rect);
    (void)radius;
}

static void gl_canvas_clear(VenomCanvas* canvas, VenomColor color) {
    VenomGLCanvas* c = (VenomGLCanvas*)canvas;
    gl_make_current(c);
    glClearColor(color.r / 255.0f, color.g / 255.0f, color.b / 255.0f, color.a / 255.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
}

static void setup_quad_geometry(VenomGLCanvas* c, VenomRectF rect) {
    float x = rect.x, y = rect.y, w = rect.width, h = rect.height;
    
    float vertices[] = {
        x,     y,     0.0f, 0.0f,
        x + w, y,     1.0f, 0.0f,
        x + w, y + h, 1.0f, 1.0f,
        x,     y,     0.0f, 0.0f,
        x + w, y + h, 1.0f, 1.0f,
        x,     y + h, 0.0f, 1.0f,
    };
    
    glBindVertexArray(c->quad_vao);
    glBindBuffer(GL_ARRAY_BUFFER, c->quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
}

static void gl_canvas_draw_rect(VenomCanvas* canvas, VenomRectF rect, const VenomPaint* paint) {
    VenomGLCanvas* c = (VenomGLCanvas*)canvas;
    
    glUseProgram(c->prog_solid);
    glUniformMatrix4fv(glGetUniformLocation(c->prog_solid, "uProjection"), 
                       1, GL_FALSE, c->projection_matrix);
    glUniformMatrix4fv(glGetUniformLocation(c->prog_solid, "uModel"), 
                       1, GL_FALSE, c->model_matrix);
    glUniform4f(glGetUniformLocation(c->prog_solid, "uColor"),
                paint->color.r / 255.0f, paint->color.g / 255.0f,
                paint->color.b / 255.0f, paint->color.a / 255.0f);
    
    setup_quad_geometry(c, rect);
    glDrawArrays(GL_TRIANGLES, 0, 6);
}

static void gl_canvas_draw_rounded_rect(VenomCanvas* canvas, VenomRectF rect, 
                                         VenomF32 radius, const VenomPaint* paint) {
    VenomGLCanvas* c = (VenomGLCanvas*)canvas;
    
    /* Create geometry centered at origin for SDF */
    float vertices[] = {
        0.0f,        0.0f,         0.0f, 0.0f,
        rect.width,  0.0f,         rect.width, 0.0f,
        rect.width,  rect.height,  rect.width, rect.height,
        0.0f,        0.0f,         0.0f, 0.0f,
        rect.width,  rect.height,  rect.width, rect.height,
        0.0f,        rect.height,  0.0f, rect.height,
    };
    
    /* Update model matrix for position */
    float saved[16];
    mat4_copy(saved, c->model_matrix);
    mat4_translate(c->model_matrix, rect.x, rect.y, 0.0f);
    
    glUseProgram(c->prog_rounded);
    glUniformMatrix4fv(glGetUniformLocation(c->prog_rounded, "uProjection"), 
                       1, GL_FALSE, c->projection_matrix);
    glUniformMatrix4fv(glGetUniformLocation(c->prog_rounded, "uModel"), 
                       1, GL_FALSE, c->model_matrix);
    glUniform4f(glGetUniformLocation(c->prog_rounded, "uColor"),
                paint->color.r / 255.0f, paint->color.g / 255.0f,
                paint->color.b / 255.0f, paint->color.a / 255.0f);
    glUniform2f(glGetUniformLocation(c->prog_rounded, "uSize"), rect.width, rect.height);
    glUniform1f(glGetUniformLocation(c->prog_rounded, "uRadius"), radius);
    
    glBindVertexArray(c->quad_vao);
    glBindBuffer(GL_ARRAY_BUFFER, c->quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    mat4_copy(c->model_matrix, saved);
}

static void gl_canvas_draw_circle(VenomCanvas* canvas, VenomF32 cx, VenomF32 cy, 
                                   VenomF32 radius, const VenomPaint* paint) {
    VenomGLCanvas* c = (VenomGLCanvas*)canvas;
    
    /* Create quad centered at origin */
    float r = radius;
    float vertices[] = {
        -r, -r, -r, -r,
         r, -r,  r, -r,
         r,  r,  r,  r,
        -r, -r, -r, -r,
         r,  r,  r,  r,
        -r,  r, -r,  r,
    };
    
    float saved[16];
    mat4_copy(saved, c->model_matrix);
    mat4_translate(c->model_matrix, cx, cy, 0.0f);
    
    glUseProgram(c->prog_circle);
    glUniformMatrix4fv(glGetUniformLocation(c->prog_circle, "uProjection"), 
                       1, GL_FALSE, c->projection_matrix);
    glUniformMatrix4fv(glGetUniformLocation(c->prog_circle, "uModel"), 
                       1, GL_FALSE, c->model_matrix);
    glUniform4f(glGetUniformLocation(c->prog_circle, "uColor"),
                paint->color.r / 255.0f, paint->color.g / 255.0f,
                paint->color.b / 255.0f, paint->color.a / 255.0f);
    glUniform1f(glGetUniformLocation(c->prog_circle, "uRadius"), radius);
    
    glBindVertexArray(c->quad_vao);
    glBindBuffer(GL_ARRAY_BUFFER, c->quad_vbo);
    glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
    glDrawArrays(GL_TRIANGLES, 0, 6);
    
    mat4_copy(c->model_matrix, saved);
}

static void gl_canvas_draw_oval(VenomCanvas* canvas, VenomRectF rect, const VenomPaint* paint) {
    /* Draw as stretched circle */
    VenomF32 cx = rect.x + rect.width / 2.0f;
    VenomF32 cy = rect.y + rect.height / 2.0f;
    VenomF32 r = VENOM_MIN(rect.width, rect.height) / 2.0f;
    gl_canvas_draw_circle(canvas, cx, cy, r, paint);
}

static void gl_canvas_draw_line(VenomCanvas* canvas, VenomF32 x1, VenomF32 y1, 
                                 VenomF32 x2, VenomF32 y2, const VenomPaint* paint) {
    VenomGLCanvas* c = (VenomGLCanvas*)canvas;
    
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

static void gl_canvas_draw_path(VenomCanvas* canvas, const VenomPath* path, const VenomPaint* paint) {
    /* Path rendering requires tessellation - not implemented for simplicity */
    (void)canvas; (void)path; (void)paint;
}

static void gl_canvas_draw_text(VenomCanvas* canvas, const char* text, VenomF32 x, VenomF32 y,
                                 const VenomFont* font, const VenomPaint* paint) {
    VenomGLCanvas* c = (VenomGLCanvas*)canvas;
    (void)font;
    
    if (!text || !*text) return;
    
    /* If font not loaded, try to load default font */
    if (!c->font_loaded) {
        /* Try common font paths */
        static const char* font_paths[] = {
            "/usr/share/fonts/truetype/dejavu/DejaVuSans.ttf",
            "/usr/share/fonts/truetype/liberation/LiberationSans-Regular.ttf",
            "/usr/share/fonts/TTF/DejaVuSans.ttf",
            "/usr/share/fonts/noto/NotoSans-Regular.ttf",
            NULL
        };
        
        for (int i = 0; font_paths[i]; i++) {
            if (gl_load_font(c, font_paths[i], 18.0f)) break;
        }
        
        if (!c->font_loaded) {
            /* Fallback: draw rectangles */
            VenomF32 cursor = x;
            for (const char* p = text; *p; p++) {
                VenomRectF r = { cursor, y - 12, 8, 14 };
                gl_canvas_draw_rect(canvas, r, paint);
                cursor += 10;
            }
            return;
        }
    }
    
    /* Use text shader */
    glUseProgram(c->prog_text);
    glUniformMatrix4fv(glGetUniformLocation(c->prog_text, "uProjection"), 
                       1, GL_FALSE, c->projection_matrix);
    glUniform4f(glGetUniformLocation(c->prog_text, "uColor"),
                paint->color.r / 255.0f, paint->color.g / 255.0f,
                paint->color.b / 255.0f, paint->color.a / 255.0f);
    
    glActiveTexture(GL_TEXTURE0);
    glBindTexture(GL_TEXTURE_2D, c->font_texture);
    glUniform1i(glGetUniformLocation(c->prog_text, "uTexture"), 0);
    
    float cursor_x = x;
    
    for (const char* p = text; *p; p++) {
        if (*p < 32 || *p >= 127) continue;
        
        stbtt_bakedchar* ch = &c->font_chars[*p - 32];
        
        float x0 = cursor_x + ch->xoff;
        float y0 = y + ch->yoff;
        float x1 = x0 + (ch->x1 - ch->x0);
        float y1 = y0 + (ch->y1 - ch->y0);
        
        float s0 = ch->x0 / 512.0f;
        float t0 = ch->y0 / 512.0f;
        float s1 = ch->x1 / 512.0f;
        float t1 = ch->y1 / 512.0f;
        
        float vertices[] = {
            x0, y0, s0, t0,
            x1, y0, s1, t0,
            x1, y1, s1, t1,
            x0, y0, s0, t0,
            x1, y1, s1, t1,
            x0, y1, s0, t1,
        };
        
        glBindVertexArray(c->quad_vao);
        glBindBuffer(GL_ARRAY_BUFFER, c->quad_vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_DYNAMIC_DRAW);
        glDrawArrays(GL_TRIANGLES, 0, 6);
        
        cursor_x += ch->xadvance;
    }
}

static void gl_canvas_draw_image(VenomCanvas* canvas, const VenomImage* image, VenomF32 x, VenomF32 y) {
    (void)canvas; (void)image; (void)x; (void)y;
    /* TODO: texture rendering */
}

static void gl_canvas_draw_image_rect(VenomCanvas* canvas, const VenomImage* image,
                                       VenomRectF src, VenomRectF dst, const VenomPaint* paint) {
    (void)canvas; (void)image; (void)src; (void)dst; (void)paint;
    /* TODO: texture rendering */
}

static void gl_canvas_flush(VenomCanvas* canvas) {
    VenomGLCanvas* c = (VenomGLCanvas*)canvas;
    gl_make_current(c);
    glFlush();
    glXSwapBuffers(c->display, c->window);
}

static VenomSize2D gl_canvas_get_size(VenomCanvas* canvas) {
    return (VenomSize2D){ .width = canvas->width, .height = canvas->height };
}

/* ============================================================================
 * CANVAS VTABLE
 * ============================================================================ */

static const VenomCanvasOps gl_canvas_ops = {
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
    .draw_image = gl_canvas_draw_image,
    .draw_image_rect = gl_canvas_draw_image_rect,
    .flush = gl_canvas_flush,
    .get_size = gl_canvas_get_size,
};

/* ============================================================================
 * CANVAS CREATION - PUBLIC API
 * ============================================================================ */

static void canvas_destructor(void* self) {
    gl_canvas_destroy((VenomCanvas*)self);
}

VenomResultPtr venom_canvas_create_opengl(Display* display, Window window, 
                                           VenomU32 width, VenomU32 height) {
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
        return VENOM_ERR_PTR(VENOM_ERROR_SURFACE_CREATE);
    }
    
    GLXFBConfig best_fbc = fbc[0];
    XFree(fbc);
    
    /* Create OpenGL 3.3 Core context */
    typedef GLXContext (*glXCreateContextAttribsARBProc)(Display*, GLXFBConfig, GLXContext, Bool, const int*);
    glXCreateContextAttribsARBProc glXCreateContextAttribsARB = 
        (glXCreateContextAttribsARBProc)glXGetProcAddress((const GLubyte*)"glXCreateContextAttribsARB");
    
    if (!glXCreateContextAttribsARB) {
        fprintf(stderr, "glXCreateContextAttribsARB not available\n");
        return VENOM_ERR_PTR(VENOM_ERROR_CANVAS_CREATE);
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
        return VENOM_ERR_PTR(VENOM_ERROR_CANVAS_CREATE);
    }
    
    /* Make context current */
    if (!glXMakeCurrent(display, window, glx_context)) {
        glXDestroyContext(display, glx_context);
        return VENOM_ERR_PTR(VENOM_ERROR_CANVAS_CREATE);
    }
    
    /* Load GL functions */
    if (!load_gl_functions()) {
        glXDestroyContext(display, glx_context);
        return VENOM_ERR_PTR(VENOM_ERROR_CANVAS_CREATE);
    }
    
    printf("OpenGL Version: %s\n", glGetString(GL_VERSION));
    printf("OpenGL Renderer: %s\n", glGetString(GL_RENDERER));
    
    /* Create canvas */
    VenomGLCanvas* canvas = (VenomGLCanvas*)VENOM_REF_NEW(VenomGLCanvas, canvas_destructor);
    if (!canvas) {
        glXDestroyContext(display, glx_context);
        return VENOM_ERR_PTR(VENOM_ERROR_OUT_OF_MEMORY);
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
    
    /* Initialize font state */
    canvas->font_loaded = 0;
    canvas->font_texture = 0;
    
    if (!canvas->prog_solid || !canvas->prog_rounded || !canvas->prog_circle || !canvas->prog_3d) {
        fprintf(stderr, "Failed to create shader programs\n");
        venom_unref(canvas);
        return VENOM_ERR_PTR(VENOM_ERROR_CANVAS_CREATE);
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
    
    return VENOM_OK_PTR(canvas);
}

/* ============================================================================
 * 3D DRAWING API - PUBLIC
 * ============================================================================ */

void venom_gl_draw_cube(VenomCanvas* canvas, float x, float y, float z, 
                        float size, float rotX, float rotY, float rotZ) {
    VenomGLCanvas* c = (VenomGLCanvas*)canvas;
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

void venom_gl_begin_3d(VenomCanvas* canvas) {
    (void)canvas;
    glEnable(GL_DEPTH_TEST);
    glClear(GL_DEPTH_BUFFER_BIT);
}

void venom_gl_end_3d(VenomCanvas* canvas) {
    (void)canvas;
    glDisable(GL_DEPTH_TEST);
}
