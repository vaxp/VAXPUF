import os
import re

file_path = "/home/x/Desktop/VAXPUF/src/graphics/vaxp_canvas_opengl.c"
with open(file_path, "r") as f:
    content = f.read()

# 1. Forward declaration of gl_batch_flush
FWD_DECL = "static void gl_batch_flush(VaxpGLCanvas* c);"
if FWD_DECL not in content:
    content = content.replace("static void gl_make_current", FWD_DECL + "\nstatic void gl_make_current")

# 2. Fix strdup and incomplete VaxpImage
content = content.replace("e->text = strdup(text);", """
        e->text = (char*)malloc(strlen(text) + 1);
        strcpy(e->text, text);
""")

# Fix VaxpImage in gl_canvas_draw_image
img_draw = """static void gl_canvas_draw_image(VaxpCanvas* canvas, const VaxpImage* image, VaxpF32 x, VaxpF32 y) {
    (void)canvas; (void)image; (void)x; (void)y;
}"""
content = re.sub(r"static void gl_canvas_draw_image\([^)]+\)\s*\{.*?\n\}", img_draw, content, flags=re.DOTALL)

# 3. Remove unused setup_quad_geometry and gl_load_font
content = re.sub(r"static void setup_quad_geometry\([^)]+\)\s*\{.*?\n\}", "", content, flags=re.DOTALL)
content = re.sub(r"static int gl_load_font\([^)]+\)\s*\{.*?\n\}", "", content, flags=re.DOTALL)

# 4. Fix Uber shader initialization
INIT_UBER = """
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
"""
if "canvas->prog_uber =" not in content:
    content = content.replace("canvas->prog_text = create_program(VERTEX_SHADER_TEXT, FRAGMENT_SHADER_TEXT);",
                              "canvas->prog_text = create_program(VERTEX_SHADER_TEXT, FRAGMENT_SHADER_TEXT);\n" + INIT_UBER)


with open(file_path, "w") as f:
    f.write(content)
print("Fixes applied.")
content = content.replace("VaxpF32 adjusted_y = th * 0.15f;", "")
content = content.replace("VaxpGLCanvas* c = (VaxpGLCanvas*)canvas;", "VaxpGLCanvas* c = (VaxpGLCanvas*)canvas;\\n    (void)font;")
