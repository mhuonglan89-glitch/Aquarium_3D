#ifndef GLAD_GL_H
#define GLAD_GL_H

#include <glad/glad.h>

#ifdef gladLoadGL
#undef gladLoadGL
#endif
#define gladLoadGL(proc) gladLoadGLLoader((GLADloadproc)(proc))

#endif // GLAD_GL_H
