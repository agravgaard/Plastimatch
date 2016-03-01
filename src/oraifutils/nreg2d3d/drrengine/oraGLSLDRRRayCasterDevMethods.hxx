
#ifndef ORAGLSLDRRRAYCASTERDEVMETHODS_HXX_
#define ORAGLSLDRRRAYCASTERDEVMETHODS_HXX_


/**
 * Some development headers which are meant for development phase of
 * ora::GLSLDRRRayCaster only.
 * Should not be included in ora::GLSLDRRRayCaster for working releases.
 * NOTE: large fractions of code from vtkGPUVolumeRayCastMapper was taken over!
 * 
 * @author VTK-community
 * @author phil <philipp.steininger (at) pmu.ac.at>
 * @version 1.0
 */


// CALLS

#ifdef PRINT_UNIFORM_VARIABLES
  #undef PRINT_UNIFORM_VARIABLES
#endif
#define PRINT_UNIFORM_VARIABLES(shader) \
  PrintUniformVariables(shader);
#ifdef PRINT_GL_ERRORS
  #undef PRINT_GL_ERRORS
#endif
#define PRINT_GL_ERRORS(title) \
  PrintGLErrors(title);
#ifdef CHECK_FB_STATUS
  #undef CHECK_FB_STATUS
#endif
#define CHECK_FB_STATUS() \
  CheckFrameBufferStatus();
#ifdef BUFF_TO_STRING
  #undef BUFF_TO_STRING
#endif
#define BUFF_TO_STRING(buffer) \
    BufferToString(buffer);
#ifdef DISPLAY_BUFFERS
  #undef DISPLAY_BUFFERS
#endif
#define DISPLAY_BUFFERS() \
  DisplayReadAndDrawBuffers();
#ifdef DISPLAY_FB_ATTACHMENTS
  #undef DISPLAY_FB_ATTACHMENTS
#endif
#define DISPLAY_FB_ATTACHMENTS() \
  DisplayFrameBufferAttachments();
#ifdef DISPLAY_FB_ATTACHMENT
  #undef DISPLAY_FB_ATTACHMENT
#endif
#define DISPLAY_FB_ATTACHMENT(attachment) \
  DisplayFrameBufferAttachment(attachment);


// DEFINITIONS

#ifdef PRINT_UNIFORM_VARIABLES_F
  #undef PRINT_UNIFORM_VARIABLES_F
#endif
#define PRINT_UNIFORM_VARIABLES_F \
  /** \
   * Print all uniform variables of a specified program shader. \
   */ \
  void PrintUniformVariables(unsigned int programShader) \
  { \
    GLint params; \
    GLuint prog = static_cast<GLuint>(programShader); \
    vtkgl::GetProgramiv(prog,vtkgl::ACTIVE_UNIFORMS, &params); \
    cout << "There are " << params << " active uniform variables" << endl; \
    GLuint i = 0; \
    GLuint c = static_cast<GLuint>(params); \
    vtkgl::GetProgramiv(prog, vtkgl::OBJECT_ACTIVE_UNIFORM_MAX_LENGTH_ARB, \
      &params); \
    GLint buffSize = params; \
    char *name = new char[buffSize + 1]; \
    GLint size; \
    GLenum type; \
    while (i < c) \
    { \
      vtkgl::GetActiveUniform(prog, i, buffSize, 0, &size, &type, name); \
      cout << i << " "; \
      switch (type) \
      { \
        case GL_FLOAT: \
          cout << "float"; \
          break; \
        case vtkgl::FLOAT_VEC2_ARB: \
          cout << "vec2"; \
          break; \
        case vtkgl::FLOAT_VEC3_ARB: \
          cout << "vec3"; \
          break; \
        case vtkgl::FLOAT_VEC4_ARB: \
          cout << "vec4"; \
          break; \
        case GL_INT: \
          cout << "int"; \
          break; \
        case vtkgl::INT_VEC2_ARB: \
          cout << "ivec2"; \
          break; \
        case vtkgl::INT_VEC3_ARB: \
          cout << "ivec3"; \
          break; \
        case vtkgl::INT_VEC4_ARB: \
          cout << "ivec4"; \
          break; \
        case vtkgl::BOOL_ARB: \
          cout << "bool"; \
          break; \
        case vtkgl::BOOL_VEC2_ARB: \
          cout << "bvec2"; \
          break; \
        case vtkgl::BOOL_VEC3_ARB: \
          cout << "bvec3"; \
          break; \
        case vtkgl::BOOL_VEC4_ARB: \
          cout << "bvec4"; \
          break; \
        case vtkgl::FLOAT_MAT2_ARB: \
          cout << "mat2"; \
          break; \
        case vtkgl::FLOAT_MAT3_ARB: \
          cout << "mat3"; \
          break; \
        case vtkgl::FLOAT_MAT4_ARB: \
          cout << "mat4"; \
          break; \
        case vtkgl::SAMPLER_1D_ARB: \
          cout << "sampler1D"; \
          break; \
        case vtkgl::SAMPLER_2D_ARB: \
          cout << "sampler2D"; \
          break; \
        case vtkgl::SAMPLER_3D_ARB: \
          cout << "sampler3D"; \
          break; \
        case vtkgl::SAMPLER_CUBE_ARB: \
          cout << "samplerCube"; \
          break; \
        case vtkgl::SAMPLER_1D_SHADOW_ARB: \
          cout << "sampler1Dshadow"; \
          break; \
        case vtkgl::SAMPLER_2D_SHADOW_ARB: \
          cout << "sampler2Dshadow"; \
          break; \
      } \
      cout << " " << name << endl; \
      ++i; \
    } \
    delete[] name; \
  }

#ifdef GL_ERROR_TO_STRING_F
  #undef GL_ERROR_TO_STRING_F
#endif
#define GL_ERROR_TO_STRING_F \
  /** Generate openGL error messages to clear text. **/ \
  const char *OpenGLErrorMessage(unsigned int errorCode) \
  { \
    const char *result; \
    switch(static_cast<GLenum>(errorCode)) \
    { \
      case GL_NO_ERROR: \
        result = "No error"; \
        break; \
      case GL_INVALID_ENUM: \
        result = "Invalid enum"; \
        break; \
      case GL_INVALID_VALUE: \
        result = "Invalid value"; \
        break; \
      case GL_INVALID_OPERATION: \
        result = "Invalid operation"; \
        break; \
      case GL_STACK_OVERFLOW: \
        result = "stack overflow"; \
        break; \
      case GL_STACK_UNDERFLOW: \
        result = "stack underflow"; \
        break; \
      case GL_OUT_OF_MEMORY: \
        result = "out of memory"; \
        break; \
      case vtkgl::TABLE_TOO_LARGE: \
        result = "Table too large"; \
        break; \
      case vtkgl::INVALID_FRAMEBUFFER_OPERATION_EXT: \
        result = "invalid framebuffer operation ext"; \
        break; \
      case vtkgl::TEXTURE_TOO_LARGE_EXT: \
        result = "Texture too large"; \
        break; \
      default: \
        result = "unknown error"; \
    } \
    return result; \
  } \

#ifdef PRINT_GL_ERRORS_F
  #undef PRINT_GL_ERRORS_F
#endif
#define PRINT_GL_ERRORS_F \
  /** \
   * Print GL errors if any and display title for that print. \
   */ \
  void PrintGLErrors(const char *title) \
  { \
    GLenum errorCode = glGetError(); \
    if (errorCode != GL_NO_ERROR) \
    { \
      if (title) \
        cout << title << ": "; \
      cout << "ERROR (x" << hex << errorCode << ") " << dec; \
      cout << OpenGLErrorMessage(static_cast<unsigned int>(errorCode)); \
      cout << endl; \
    } \
  }

#ifdef CHECK_FB_STATUS_F
  #undef CHECK_FB_STATUS_F
#endif
#define CHECK_FB_STATUS_F \
   /** \
    * Display the status of the current frame buffer on the standard output. \
    */ \
   void CheckFrameBufferStatus() \
   { \
     GLenum status; \
     status = vtkgl::CheckFramebufferStatusEXT(vtkgl::FRAMEBUFFER_EXT); \
     switch(status) \
     { \
       case 0: \
         cout << "call to vtkgl::CheckFramebufferStatusEXT generates an error."\
              << endl; \
         break; \
       case vtkgl::FRAMEBUFFER_COMPLETE_EXT: \
         break; \
       case vtkgl::FRAMEBUFFER_UNSUPPORTED_EXT: \
         cout << "framebuffer is unsupported" << endl; \
         break; \
       case vtkgl::FRAMEBUFFER_INCOMPLETE_ATTACHMENT_EXT: \
         cout << "framebuffer has an attachment error" << endl; \
         break; \
       case vtkgl::FRAMEBUFFER_INCOMPLETE_MISSING_ATTACHMENT_EXT: \
         cout << "framebuffer has a missing attachment" << endl; \
         break; \
       case vtkgl::FRAMEBUFFER_INCOMPLETE_DIMENSIONS_EXT: \
         cout << "framebuffer has bad dimensions" << endl; \
         break; \
       case vtkgl::FRAMEBUFFER_INCOMPLETE_FORMATS_EXT: \
         cout << "framebuffer has bad formats" << endl; \
         break; \
       case vtkgl::FRAMEBUFFER_INCOMPLETE_DRAW_BUFFER_EXT: \
         cout << "framebuffer has bad draw buffer" << endl; \
         break; \
       case vtkgl::FRAMEBUFFER_INCOMPLETE_READ_BUFFER_EXT: \
         cout << "framebuffer has bad read buffer" << endl; \
         break; \
       default: \
         cout << "Unknown framebuffer status=0x" << hex << status << dec << endl; \
     } \
   }

#ifdef BUFF_TO_STRING_F
  #undef BUFF_TO_STRING_F
#endif
#define BUFF_TO_STRING_F \
  /** \
   * Create a string from a buffer id. The result has to be free by the caller. \
   */ \
  vtkStdString BufferToString(int buffer) \
  { \
    vtkStdString result; \
    vtksys_ios::ostringstream ost; \
    GLint size; \
    GLint b = static_cast<GLint>(buffer); \
    switch (b) \
    { \
      case GL_NONE: \
        ost << "GL_NONE"; \
        break; \
      case GL_FRONT_LEFT: \
        ost << "GL_FRONT_LEFT"; \
        break; \
      case GL_FRONT_RIGHT: \
        ost << "GL_FRONT_RIGHT"; \
        break; \
      case GL_BACK_LEFT: \
        ost << "GL_BACK_LEFT"; \
        break; \
      case GL_BACK_RIGHT: \
        ost << "GL_BACK_RIGHT"; \
        break; \
      case GL_FRONT: \
        ost << "GL_FRONT"; \
        break; \
      case GL_BACK: \
        ost << "GL_BACK"; \
        break; \
      case GL_LEFT: \
        ost << "GL_LEFT"; \
        break;  \
      case GL_RIGHT:  \
        ost << "GL_RIGHT"; \
        break; \
      case GL_FRONT_AND_BACK: \
        ost << "GL_FRONT_AND_BACK"; \
        break; \
      default: \
        glGetIntegerv(GL_AUX_BUFFERS, &size); \
        if (buffer >= GL_AUX0 && buffer < (GL_AUX0 + size)) \
        { \
          ost << "GL_AUX" << (buffer - GL_AUX0); \
        } \
        else \
        {  \
          glGetIntegerv(vtkgl::MAX_COLOR_ATTACHMENTS_EXT, &size); \
          if (static_cast<GLuint>(buffer) >= vtkgl::COLOR_ATTACHMENT0_EXT && \
              static_cast<GLuint>(buffer) < \
              (vtkgl::COLOR_ATTACHMENT0_EXT + static_cast<GLuint>(size))) \
            ost << "GL_COLOR_ATTACHMENT" \
                << (static_cast<GLuint>(buffer) - vtkgl::COLOR_ATTACHMENT0_EXT) \
                << "_EXT"; \
          else \
            ost << "unknown color buffer type=0x" << hex << buffer << dec; \
        } \
        break; \
    } \
    result=ost.str(); \
    return result; \
  }

#ifdef DISPLAY_BUFFERS_F
  #undef DISPLAY_BUFFERS_F
#endif
#define DISPLAY_BUFFERS_F \
  /** \
   * Display the buffers assigned for drawing and reading operations. \
   */ \
  void DisplayReadAndDrawBuffers() \
  { \
    GLint value; \
    glGetIntegerv(vtkgl::MAX_DRAW_BUFFERS, &value); \
    GLenum max=static_cast<GLenum>(value); \
    vtkStdString s; \
    GLenum i = 0; \
    while (i < max) \
    { \
      glGetIntegerv(vtkgl::DRAW_BUFFER0 + i, &value); \
      s = this->BufferToString(static_cast<int>(value)); \
      cout << "draw buffer " << i << "=" << s << endl; \
      ++i; \
    } \
    glGetIntegerv(GL_READ_BUFFER, &value); \
    s = this->BufferToString(static_cast<int>(value)); \
    cout << "read buffer=" << s << endl; \
  }

#ifdef DISPLAY_FB_ATTACHMENTS_F
  #undef DISPLAY_FB_ATTACHMENTS_F
#endif
#define DISPLAY_FB_ATTACHMENTS_F \
  /** \
   * Display all the attachments of the current frame buffer object. \
   */ \
  void DisplayFrameBufferAttachments() \
  { \
    GLint framebufferBinding; \
    glGetIntegerv(vtkgl::FRAMEBUFFER_BINDING_EXT, &framebufferBinding); \
    this->PrintGLErrors("after getting FRAMEBUFFER_BINDING_EXT"); \
    if (framebufferBinding == 0) \
    { \
      cout << "Current framebuffer is bind to the system one" << endl; \
    } \
    else \
    { \
      cout << "Current framebuffer is bind to framebuffer object " \
          << framebufferBinding<<endl; \
      GLint value; \
      glGetIntegerv(vtkgl::MAX_COLOR_ATTACHMENTS_EXT,&value); \
      GLenum maxColorAttachments = static_cast<GLenum>(value); \
      this->PrintGLErrors("after getting MAX_COLOR_ATTACHMENTS_EXT"); \
      GLenum i = 0; \
      while (i < maxColorAttachments) \
      { \
        cout << "color attachement " << i << ":" << endl; \
        this->DisplayFrameBufferAttachment(vtkgl::COLOR_ATTACHMENT0_EXT + i); \
        ++i; \
      } \
      cout << "depth attachement :" << endl; \
      this->DisplayFrameBufferAttachment(vtkgl::DEPTH_ATTACHMENT_EXT); \
      cout << "stencil attachement :" << endl; \
      this->DisplayFrameBufferAttachment(vtkgl::STENCIL_ATTACHMENT_EXT); \
    } \
  }

#ifdef DISPLAY_FB_ATTACHMENT_F
  #undef DISPLAY_FB_ATTACHMENT_F
#endif
#define DISPLAY_FB_ATTACHMENT_F \
  /** \
   * Display a given attachment for the current frame buffer object. \
   */ \
  void DisplayFrameBufferAttachment(unsigned int uattachment) \
  { \
    GLenum attachment=static_cast<GLenum>(uattachment); \
    GLint params; \
    vtkgl::GetFramebufferAttachmentParameterivEXT( \
      vtkgl::FRAMEBUFFER_EXT, attachment, \
      vtkgl::FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_EXT, &params); \
    this->PrintGLErrors("after getting FRAMEBUFFER_ATTACHMENT_OBJECT_TYPE_EXT"); \
    switch (params) \
    { \
      case GL_NONE: \
        cout << " this attachment is empty" << endl; \
        break; \
      case GL_TEXTURE: \
        vtkgl::GetFramebufferAttachmentParameterivEXT( \
          vtkgl::FRAMEBUFFER_EXT, attachment, \
          vtkgl::FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_EXT, &params); \
        this->PrintGLErrors("after getting FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_EXT"); \
        cout << " this attachment is a texture with name: " << params << endl; \
        vtkgl::GetFramebufferAttachmentParameterivEXT( \
          vtkgl::FRAMEBUFFER_EXT, attachment, \
          vtkgl::FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL_EXT, &params); \
        this->PrintGLErrors( \
          "after getting FRAMEBUFFER_ATTACHMENT_TEXTURE_LEVEL_EXT"); \
        cout << " its mipmap level is: " << params << endl; \
        vtkgl::GetFramebufferAttachmentParameterivEXT( \
          vtkgl::FRAMEBUFFER_EXT, attachment, \
          vtkgl::FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE_EXT, &params); \
         this->PrintGLErrors( \
           "after getting FRAMEBUFFER_ATTACHMENT_TEXTURE_CUBE_MAP_FACE_EXT"); \
        if (params == 0) \
        { \
          cout << " this is not a cube map texture." << endl; \
        } \
        else \
        { \
          cout << " this is a cube map texture and the image is contained in face " \
               << params << endl; \
        } \
         vtkgl::GetFramebufferAttachmentParameterivEXT( \
           vtkgl::FRAMEBUFFER_EXT, attachment, \
           vtkgl::FRAMEBUFFER_ATTACHMENT_TEXTURE_3D_ZOFFSET_EXT, &params); \
          this->PrintGLErrors( \
            "after getting FRAMEBUFFER_ATTACHMENT_TEXTURE_3D_ZOFFSET_EXT"); \
        if (params == 0) \
          cout << " this is not 3D texture." << endl; \
        else \
          cout << " this is a 3D texture and the zoffset of the attached image is " \
              << params << endl; \
        break; \
      case vtkgl::RENDERBUFFER_EXT: \
        cout << " this attachment is a renderbuffer" << endl; \
        vtkgl::GetFramebufferAttachmentParameterivEXT( \
          vtkgl::FRAMEBUFFER_EXT, attachment, \
          vtkgl::FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_EXT, &params); \
        this->PrintGLErrors("after getting FRAMEBUFFER_ATTACHMENT_OBJECT_NAME_EXT"); \
        cout << " this attachment is a renderbuffer with name: " << params << endl; \
        vtkgl::BindRenderbufferEXT(vtkgl::RENDERBUFFER_EXT, \
                                   static_cast<GLuint>(params)); \
        this->PrintGLErrors( \
          "after getting binding the current RENDERBUFFER_EXT to params"); \
        vtkgl::GetRenderbufferParameterivEXT(vtkgl::RENDERBUFFER_EXT, \
                                             vtkgl::RENDERBUFFER_WIDTH_EXT, \
                                             &params); \
        this->PrintGLErrors("after getting RENDERBUFFER_WIDTH_EXT"); \
        cout << " renderbuffer width=" << params << endl; \
        vtkgl::GetRenderbufferParameterivEXT(vtkgl::RENDERBUFFER_EXT, \
                                             vtkgl::RENDERBUFFER_HEIGHT_EXT, \
                                             &params); \
        this->PrintGLErrors("after getting RENDERBUFFER_HEIGHT_EXT"); \
        cout << " renderbuffer height=" << params << endl; \
        vtkgl::GetRenderbufferParameterivEXT( \
          vtkgl::RENDERBUFFER_EXT,vtkgl::RENDERBUFFER_INTERNAL_FORMAT_EXT, \
          &params); \
        this->PrintGLErrors("after getting RENDERBUFFER_INTERNAL_FORMAT_EXT"); \
        cout << " renderbuffer internal format=0x" << hex << params << dec << endl; \
        vtkgl::GetRenderbufferParameterivEXT(vtkgl::RENDERBUFFER_EXT, \
                                             vtkgl::RENDERBUFFER_RED_SIZE_EXT, \
                                             &params); \
        this->PrintGLErrors("after getting RENDERBUFFER_RED_SIZE_EXT"); \
        cout << " renderbuffer actual resolution for the red component=" << params \
            << endl; \
        vtkgl::GetRenderbufferParameterivEXT(vtkgl::RENDERBUFFER_EXT, \
                                             vtkgl::RENDERBUFFER_GREEN_SIZE_EXT, \
                                             &params); \
        this->PrintGLErrors("after getting RENDERBUFFER_GREEN_SIZE_EXT"); \
        cout << " renderbuffer actual resolution for the green component=" << params \
            << endl; \
        vtkgl::GetRenderbufferParameterivEXT(vtkgl::RENDERBUFFER_EXT, \
                                             vtkgl::RENDERBUFFER_BLUE_SIZE_EXT, \
                                             &params); \
        this->PrintGLErrors("after getting RENDERBUFFER_BLUE_SIZE_EXT"); \
        cout << " renderbuffer actual resolution for the blue component=" << params \
            << endl; \
        vtkgl::GetRenderbufferParameterivEXT(vtkgl::RENDERBUFFER_EXT, \
                                             vtkgl::RENDERBUFFER_ALPHA_SIZE_EXT, \
                                             &params); \
        this->PrintGLErrors("after getting RENDERBUFFER_ALPHA_SIZE_EXT"); \
        cout << " renderbuffer actual resolution for the alpha component=" << params \
            << endl; \
        vtkgl::GetRenderbufferParameterivEXT(vtkgl::RENDERBUFFER_EXT, \
                                             vtkgl::RENDERBUFFER_DEPTH_SIZE_EXT, \
                                             &params); \
        this->PrintGLErrors("after getting RENDERBUFFER_DEPTH_SIZE_EXT"); \
        cout << " renderbuffer actual resolution for the depth component=" << params \
            << endl; \
        vtkgl::GetRenderbufferParameterivEXT( \
          vtkgl::RENDERBUFFER_EXT,vtkgl::RENDERBUFFER_STENCIL_SIZE_EXT, &params); \
        this->PrintGLErrors("after getting RENDERBUFFER_STENCIL_SIZE_EXT"); \
        cout << " renderbuffer actual resolution for the stencil component=" \
            << params << endl; \
        break; \
      default: \
        cout << " unexcepted value." << endl; \
        break; \
    } \
  }


#endif /* ORAGLSLDRRRAYCASTERDEVMETHODS_HXX_ */
