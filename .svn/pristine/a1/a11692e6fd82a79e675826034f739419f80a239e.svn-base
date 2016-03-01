#ifndef ORAOPENCLWRAPPER_H_
#define ORAOPENCLWRAPPER_H_

// allow c++ like exceptions for openCL functions
#define __CL_ENABLE_EXCEPTIONS
// opencl
#include <CL/cl.hpp>

// std
#include <vector>
#include <iostream>

namespace ora
{

/*
 * oraOpenCLWrapper.h
 */

class OpenCLWrapper
{
public:

  typedef OpenCLWrapper self;

  OpenCLWrapper();
  ~OpenCLWrapper();

  bool Initalize();

  bool CommitKernel(int *a, int *b, int *c, int bufferSize);

protected:
  /**
   *
   */
  std::vector<cl::Platform> m_PlatformList;
  cl_uint m_PlatformNum;


private:

};

}
#endif /* ORAOPENCLWRAPPER_H_ */
