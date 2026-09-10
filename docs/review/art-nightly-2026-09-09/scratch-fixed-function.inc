angle::Result TextureGL::setImageViaScratchUnpackBuffer(const gl::Context *context,
                                                        gl::TextureTarget target,
                                                        size_t level,
                                                        GLenum internalFormat,
                                                        const gl::Extents &size,
                                                        GLenum format,
                                                        GLenum type,
                                                        const gl::PixelUnpackState &unpack,
                                                        bool isCompressed,
                                                        size_t imageSize,
                                                        const uint8_t *pixels)
{
    ContextGL *contextGL              = GetImplAs<ContextGL>(context);
    const FunctionsGL *functions      = GetFunctionsGL(context);
    StateManagerGL *stateManager      = GetStateManagerGL(context);
    const angle::FeaturesGL &features = GetFeaturesGL(context);

    if (features.reattachFboDepthStencilOnReallocation.enabled)
    {
        onStateChange(angle::SubjectMessage::ObjectReallocated);
    }

    GLuint uploadBytes = 0;
    if (isCompressed)
    {
        uploadBytes = static_cast<GLuint>(imageSize);
    }
    else
    {
        ANGLE_CHECK_GL_MATH(contextGL, gl::GetInternalFormatInfo(format, type)
                                           .computePackUnpackEndByte(type, size, unpack,
                                                                     /*is3D=*/false, &uploadBytes));
    }

    GLuint scratch = 0;
    functions->genBuffers(1, &scratch);
    struct ScopedScratchBuffer
    {
        StateManagerGL *stateManager;
        GLuint buffer;
        ~ScopedScratchBuffer() { reset(); }
        void reset()
        {
            if (buffer != 0)
            {
                stateManager->deleteBuffer(buffer);
                buffer = 0;
            }
        }
    } scopedScratch{stateManager, scratch};
    stateManager->bindBuffer(gl::BufferBinding::PixelUnpack, scratch);
    // Regardless of whether the user supplied data (pixels != nullptr), the pixel unpack buffer
    // must be allocated with the expected amount of data.
    if (uploadBytes > 0)
    {
        ANGLE_GL_TRY(context, functions->bufferData(GL_PIXEL_UNPACK_BUFFER, uploadBytes, pixels,
                                                    GL_STREAM_DRAW));
    }

    ANGLE_TRY(stateManager->setPixelUnpackState(context, unpack));

    stateManager->bindTexture(getType(), mTextureID);

    if (features.resetTexImage2DBaseLevel.enabled)
    {
        (void)setBaseLevel(context, 0);
    }

    if (isCompressed)
    {
        const gl::InternalFormat &originalInternalFormatInfo =
            gl::GetSizedInternalFormatInfo(internalFormat);
        nativegl::CompressedTexImageFormat compressedTexImageFormat =
            nativegl::GetCompressedTexImageFormat(functions, features, internalFormat);

        ANGLE_GL_TRY_ALWAYS_CHECK(
            context, functions->compressedTexImage2D(
                         nativegl::GetTextureBindingTarget(target), static_cast<GLint>(level),
                         compressedTexImageFormat.internalFormat, size.width, size.height, 0,
                         static_cast<GLsizei>(uploadBytes), nullptr));

        LevelInfoGL levelInfo = GetLevelInfo(features, originalInternalFormatInfo,
                                             compressedTexImageFormat.internalFormat);
        ASSERT(!levelInfo.lumaWorkaround.enabled);
        setLevelInfo(context, target, level, 1, levelInfo);
    }
    else
    {
        const gl::InternalFormat &originalInternalFormatInfo =
            gl::GetInternalFormatInfo(internalFormat, type);
        nativegl::TexImageFormat texImageFormat =
            nativegl::GetTexImageFormat(functions, features, internalFormat, format, type);

        ANGLE_GL_TRY_ALWAYS_CHECK(
            context, functions->texImage2D(nativegl::GetTextureBindingTarget(target),
                                           static_cast<GLint>(level), texImageFormat.internalFormat,
                                           size.width, size.height, 0, texImageFormat.format,
                                           texImageFormat.type, nullptr));

        LevelInfoGL levelInfo =
            GetLevelInfo(features, originalInternalFormatInfo, texImageFormat.internalFormat);
        setLevelInfo(context, target, level, 1, levelInfo);
    }

    scopedScratch.reset();

    contextGL->markWorkSubmitted();
    return angle::Result::Continue;
}

