////////////////////////////////////////////////////////////
//
// SFML - Simple and Fast Multimedia Library
// Copyright (C) 2007-2009 Laurent Gomila (laurent.gom@gmail.com)
//
// This software is provided 'as-is', without any express or implied warranty.
// In no event will the authors be held liable for any damages arising from the use of this software.
//
// Permission is granted to anyone to use this software for any purpose,
// including commercial applications, and to alter it and redistribute it freely,
// subject to the following restrictions:
//
// 1. The origin of this software must not be misrepresented;
//    you must not claim that you wrote the original software.
//    If you use this software in a product, an acknowledgment
//    in the product documentation would be appreciated but is not required.
//
// 2. Altered source versions must be plainly marked as such,
//    and must not be misrepresented as being the original software.
//
// 3. This notice may not be removed or altered from any source distribution.
//
////////////////////////////////////////////////////////////

////////////////////////////////////////////////////////////
// Headers
////////////////////////////////////////////////////////////
#include <SFML/Graphics/RenderImageImplFBO.hpp>
#include <SFML/Graphics/Image.hpp>
#include <SFML/Graphics/GLCheck.hpp>
#include <iostream>


namespace sf
{
namespace priv
{
////////////////////////////////////////////////////////////
/// Default constructor
////////////////////////////////////////////////////////////
RenderImageImplFBO::RenderImageImplFBO() :
myFrameBuffer(0),
myDepthBuffer(0)
{

}


////////////////////////////////////////////////////////////
/// Destructor
////////////////////////////////////////////////////////////
RenderImageImplFBO::~RenderImageImplFBO()
{
    // Destroy the depth buffer
    if (myDepthBuffer)
    {
        GLuint depthBuffer = static_cast<GLuint>(myDepthBuffer);
        GLCheck(glDeleteFramebuffersEXT(1, &depthBuffer));
    }

    // Destroy the frame buffer
    if (myFrameBuffer)
    {
        GLuint frameBuffer = static_cast<GLuint>(myFrameBuffer);
        GLCheck(glDeleteFramebuffersEXT(1, &frameBuffer));
    }
}


////////////////////////////////////////////////////////////
/// Check whether the system supports FBOs or not
////////////////////////////////////////////////////////////
bool RenderImageImplFBO::IsSupported()
{
    // Make sure that GLEW is initialized
    EnsureGlewInit();

    return glewIsSupported("GL_EXT_framebuffer_object") != 0;
}


////////////////////////////////////////////////////////////
/// /see RenderImageImpl::Create
////////////////////////////////////////////////////////////
bool RenderImageImplFBO::Create(unsigned int width, unsigned int height, unsigned int textureId, bool depthBuffer)
{
    // Create the framebuffer object if not already done
    if (!myFrameBuffer)
    {
        GLuint frameBuffer = 0;
        GLCheck(glGenFramebuffersEXT(1, &frameBuffer));
        myFrameBuffer = static_cast<unsigned int>(frameBuffer);
        if (!myFrameBuffer)
        {
            std::cerr << "Impossible to create render image (failed to create the frame buffer object)" << std::endl;
            return false;
        }
    }

    // Bind the framebuffer
    GLCheck(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, myFrameBuffer));

    // Create the depth buffer
    if (myDepthBuffer)
    {
        GLuint depth = static_cast<GLuint>(myDepthBuffer);
        GLCheck(glDeleteRenderbuffersEXT(1, &depth));
    }
    if (depthBuffer)
    {
        GLuint depth = 0;
        GLCheck(glGenRenderbuffersEXT(1, &depth));
        myDepthBuffer = static_cast<unsigned int>(depth);
        if (!myDepthBuffer)
        {
            std::cerr << "Impossible to create render image (failed to create the attached depth buffer)" << std::endl;
            return false;
        }
        GLCheck(glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, myDepthBuffer));
        GLCheck(glRenderbufferStorageEXT(GL_RENDERBUFFER_EXT, GL_DEPTH_COMPONENT, width, height));
        GLCheck(glFramebufferRenderbufferEXT(GL_FRAMEBUFFER_EXT, GL_DEPTH_ATTACHMENT_EXT, GL_RENDERBUFFER_EXT, myDepthBuffer));
    }

    // Link the image to the frame buffer
    GLCheck(glFramebufferTexture2DEXT(GL_FRAMEBUFFER_EXT, GL_COLOR_ATTACHMENT0_EXT, GL_TEXTURE_2D, textureId, 0));

    // A final check, just to be sure...
    if (glCheckFramebufferStatusEXT(GL_FRAMEBUFFER_EXT) != GL_FRAMEBUFFER_COMPLETE_EXT)
    {
        GLCheck(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0));
        std::cerr << "Impossible to create render image (failed to link the target image to the frame buffer)" << std::endl;
        return false;
    }

    // Unbind the buffers
    Activate(false);

    return true;
}


////////////////////////////////////////////////////////////
/// /see RenderImageImpl::Activate
////////////////////////////////////////////////////////////
bool RenderImageImplFBO::Activate(bool active)
{
    if (active)
    {
        // Bind the buffers
        GLCheck(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, myFrameBuffer));
        GLCheck(glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, myDepthBuffer));
    }
    else
    {
        // Unbind the buffers
        GLCheck(glBindFramebufferEXT(GL_FRAMEBUFFER_EXT, 0));
        GLCheck(glBindRenderbufferEXT(GL_RENDERBUFFER_EXT, 0));
    }

    return true;
}


////////////////////////////////////////////////////////////
/// /see RenderImageImpl::UpdateTexture
////////////////////////////////////////////////////////////
bool RenderImageImplFBO::UpdateTexture(unsigned int)
{
    // Nothing to do: the FBO draws directly into the target image
    return true;
}

} // namespace priv

} // namespace sf