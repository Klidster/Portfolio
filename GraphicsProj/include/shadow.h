#pragma once

#include "bitmap.h"
#include <functional>
#include <GL/gl3w.h>

struct ShadowStruct {
    GLuint fbo = 0;
    GLuint depthTex = 0;
    int width = 1024;
    int height = 1024;
};

ShadowStruct setup_shadowmap(int w, int h) {
    ShadowStruct shadow;
    shadow.width = w;
    shadow.height = h;

    glGenFramebuffers(1, &shadow.fbo);

    glGenTextures(1, &shadow.depthTex);
    glBindTexture(GL_TEXTURE_2D, shadow.depthTex);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_DEPTH_COMPONENT, w, h, 0,
        GL_DEPTH_COMPONENT, GL_FLOAT, NULL);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_BORDER);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_BORDER);
    float borderColor[] = { 1.0, 1.0, 1.0, 1.0 };
    glTexParameterfv(GL_TEXTURE_2D, GL_TEXTURE_BORDER_COLOR, borderColor);

    glBindFramebuffer(GL_FRAMEBUFFER, shadow.fbo);
    glFramebufferTexture2D(GL_FRAMEBUFFER, GL_DEPTH_ATTACHMENT, GL_TEXTURE_2D, shadow.depthTex, 0);
    glDrawBuffer(GL_NONE);
    glReadBuffer(GL_NONE);
    glBindFramebuffer(GL_FRAMEBUFFER, 0);

    return shadow;
}

void renderShadowMap(ShadowStruct& shadow, const glm::mat4& lightViewProj, std::function<void()> drawScene) {
    glViewport(0, 0, shadow.width, shadow.height);
    glBindFramebuffer(GL_FRAMEBUFFER, shadow.fbo);
    glClear(GL_DEPTH_BUFFER_BIT);

    drawScene();

    glBindFramebuffer(GL_FRAMEBUFFER, 0);
}