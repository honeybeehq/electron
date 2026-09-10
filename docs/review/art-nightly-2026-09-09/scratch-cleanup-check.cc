#include <cassert>
#include <cstdint>
#include <cstdio>
using GLuint=unsigned; using GLenum=unsigned; using GLint=int; using GLsizei=int;
constexpr unsigned GL_PIXEL_UNPACK_BUFFER=1, GL_STREAM_DRAW=2;
namespace angle { enum class Result { Continue, Stop }; enum class SubjectMessage { ObjectReallocated }; struct FeaturesGL {struct Flag{bool enabled=false;};Flag reattachFboDepthStencilOnReallocation,resetTexImage2DBaseLevel;}; }
bool IsError(angle::Result r){return r==angle::Result::Stop;}
#define ANGLE_UNLIKELY(x) (x)
#define ANGLE_CHECK_GL_MATH(c,x) do {if(!(x))return angle::Result::Stop;}while(0)
#define ASSERT(x) assert(x)
extern int failureStage;namespace gl {struct Context{};enum class TextureTarget{TwoD};enum class BufferBinding{PixelUnpack};struct Extents{int width=2,height=2;};struct PixelUnpackState{};struct InternalFormat{bool computePackUnpackEndByte(GLenum,const Extents&,const PixelUnpackState&,bool,GLuint*n)const{*n=16;return failureStage!=4;}};InternalFormat fmt;const InternalFormat& GetInternalFormatInfo(GLenum,GLenum){return fmt;}const InternalFormat& GetSizedInternalFormatInfo(GLenum){return fmt;}}
extern int acquired,deleted;int submitted=0;struct ContextGL{void markWorkSubmitted(){assert(acquired==deleted);++submitted;}}; ContextGL ctx;
template<class T>T* GetImplAs(const gl::Context*){return &ctx;}
int acquired=0,deleted=0,bound=0;int failureStage=0;bool errorPending=false;
struct FunctionsGL {void genBuffers(int,GLuint*p)const{*p=++acquired;}void bufferData(GLenum,GLuint,const uint8_t*,GLenum)const{errorPending=failureStage==1;}void compressedTexImage2D(GLenum,GLint,GLenum,int,int,int,GLsizei,const void*)const{errorPending=failureStage==3;}void texImage2D(GLenum,GLint,GLenum,int,int,int,GLenum,GLenum,const void*)const{errorPending=failureStage==3;}} functions;
struct StateManagerGL {void bindBuffer(gl::BufferBinding,GLuint id){bound=id;}angle::Result setPixelUnpackState(const gl::Context*,const gl::PixelUnpackState&){return failureStage==2?angle::Result::Stop:angle::Result::Continue;}void bindTexture(int,GLuint){}void deleteBuffer(GLuint id){assert(id==bound);bound=0;++deleted;}} state;
const FunctionsGL* GetFunctionsGL(const gl::Context*){return &functions;}StateManagerGL* GetStateManagerGL(const gl::Context*){return &state;}angle::FeaturesGL features;const angle::FeaturesGL& GetFeaturesGL(const gl::Context*){return features;}
void ClearErrors(const gl::Context*,const char*,const char*,unsigned){errorPending=false;}angle::Result CheckError(const gl::Context*,const char*,const char*,const char*,unsigned){return errorPending?angle::Result::Stop:angle::Result::Continue;}
namespace nativegl {struct CompressedTexImageFormat{GLenum internalFormat=1;};struct TexImageFormat{GLenum internalFormat=1,format=1,type=1;};CompressedTexImageFormat GetCompressedTexImageFormat(const FunctionsGL*,const angle::FeaturesGL&,GLenum){return{};}TexImageFormat GetTexImageFormat(const FunctionsGL*,const angle::FeaturesGL&,GLenum,GLenum,GLenum){return{};}GLenum GetTextureBindingTarget(gl::TextureTarget){return 1;}}
struct LevelInfoGL{struct {bool enabled=false;}lumaWorkaround;};LevelInfoGL GetLevelInfo(const angle::FeaturesGL&,const gl::InternalFormat&,GLenum){return{};}
class TextureGL {public: GLuint mTextureID=1;int getType(){return 0;}void onStateChange(angle::SubjectMessage){}angle::Result setBaseLevel(const gl::Context*,int){return angle::Result::Continue;}void setLevelInfo(const gl::Context*,gl::TextureTarget,size_t,int,LevelInfoGL){}angle::Result setImageViaScratchUnpackBuffer(const gl::Context*,gl::TextureTarget,size_t,GLenum,const gl::Extents&,GLenum,GLenum,const gl::PixelUnpackState&,bool,size_t,const uint8_t*);};
#include "scratch-exact-macros.inc"
#ifndef FUNCTION_FILE
#define FUNCTION_FILE "scratch-exact-function.inc"
#endif
#include FUNCTION_FILE
int main(){TextureGL texture;gl::Context context;bool pass=true;for(bool compressed:{false,true}){for(int stage:{0,1,2,3,4}){
#ifndef ANGLE_ENABLE_ASSERTS
if(stage==1)continue;
#endif
if(compressed&&stage==4)continue;
acquired=deleted=submitted=bound=0;failureStage=stage;auto r=texture.setImageViaScratchUnpackBuffer(&context,gl::TextureTarget::TwoD,1,1,{},1,1,{},compressed,16,nullptr);bool ok=acquired==(stage==4?0:1)&&deleted==acquired&&bound==0&&IsError(r)==(stage!=0)&&submitted==(stage==0?1:0);pass&=ok;std::printf("compressed=%d failure_stage=%d acquired=%d deleted=%d submitted=%d cleanup=%s\n",compressed,stage,acquired,deleted,submitted,ok?"PASS":"FAIL");}}return pass?0:1;}
