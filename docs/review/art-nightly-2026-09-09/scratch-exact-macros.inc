#define ANGLE_CONCAT1(x, y) x##y
#define ANGLE_CONCAT2(x, y) ANGLE_CONCAT1(x, y)
#define ANGLE_LOCAL_VAR ANGLE_CONCAT2(_localVar, __LINE__)

#define ANGLE_TRY_TEMPLATE(EXPR, FINALLY, FUNC)       \
    do                                                \
    {                                                 \
        auto ANGLE_LOCAL_VAR = EXPR;                  \
        FINALLY;                                      \
        if (ANGLE_UNLIKELY(IsError(ANGLE_LOCAL_VAR))) \
        {                                             \
            FUNC(ANGLE_LOCAL_VAR);                    \
        }                                             \
    } while (0)

#define ANGLE_RETURN(X) return X;
#define ANGLE_TRY(EXPR) ANGLE_TRY_TEMPLATE(EXPR, static_cast<void>(0), ANGLE_RETURN)
#define ANGLE_TRY_WITH_FINALLY(EXPR, FINALLY) ANGLE_TRY_TEMPLATE(EXPR, FINALLY, ANGLE_RETURN)


#define ANGLE_GL_TRY_ALWAYS_CHECK(context, call)                      \
    (ClearErrors(context, __FILE__, __FUNCTION__, __LINE__), (call)); \
    ANGLE_TRY(CheckError(context, #call, __FILE__, __FUNCTION__, __LINE__))

#define ANGLE_GL_CLEAR_ERRORS(functions) ClearErrors(functions, __FILE__, __FUNCTION__, __LINE__)

#if defined(ANGLE_ENABLE_ASSERTS)
#    define ANGLE_GL_TRY(context, call) ANGLE_GL_TRY_ALWAYS_CHECK(context, call)
#else
#    define ANGLE_GL_TRY(context, call) call
#endif


