void StateManagerGL::deleteBuffer(GLuint buffer)
{
    if (buffer == 0)
    {
        return;
    }

    for (auto target : angle::AllEnums<gl::BufferBinding>())
    {
        if (mBuffers[target] == buffer)
        {
            bindBuffer(target, 0);
        }

        auto &indexedTarget = mIndexedBuffers[target];
        for (size_t bindIndex = 0; bindIndex < indexedTarget.size(); ++bindIndex)
        {
            if (indexedTarget[bindIndex].buffer == buffer)
            {
                bindBufferBase(target, bindIndex, 0);
            }
        }
    }

    if (mVAOState)
    {
        if (mVAOState->elementArrayBuffer == buffer)
        {
            mVAOState->elementArrayBuffer = 0;
        }

        for (VertexBindingGL &binding : mVAOState->bindings)
        {
            if (binding.buffer == buffer)
            {
                binding.buffer = 0;
            }
        }
    }

    mFunctions->deleteBuffers(1, &buffer);
}
