#ifndef ORAOPENCLWRAPPER_CXX_
#define ORAOPENCLWRAPPER_CXX_

#include "oraOpenCLWrapper.h"

namespace ora
{

OpenCLWrapper::OpenCLWrapper()
{
  ;
}

OpenCLWrapper::~OpenCLWrapper()
{
  ;
}

bool OpenCLWrapper::Initalize()
{
  try{
	cl::Platform::get(&(this->m_PlatformList));
	}
  catch(cl::Error err)
  {
	return false;
  }
  return true;
}

bool OpenCLWrapper::CommitKernel(int *a, int *b, int *c, int bufferSize)
{
	static char kernelSourceCode[] =
	  "__kernel void                                                    \n"
	  "vadd(__global int * a, global int * b, __global int * c)         \n"
	  "{                                                                \n"
	  "   size_t i = get_global_id(0);                                  \n"
	  "                                                                 \n"
	  "   c[i] = a[i] + b[i];                                            \n"
	  "}                                                                \n"
	  ;
	try{
	  //FIXME: check platforms and don't chose statically the first one (ie.
	  // use the platform that has a GPU)
	  cl_context_properties cprops[] = {
			  CL_CONTEXT_PLATFORM,
			  (cl_context_properties)(this->m_PlatformList[0])(),
			  0 };
	  cl::Context context(CL_DEVICE_TYPE_GPU, cprops);
	  // Query the set of devices attached to the context
	  std::vector<cl::Device> devices = context.getInfo<CL_CONTEXT_DEVICES>();
	  // Create command-queue
	  cl::CommandQueue queue(context, devices[0], 0);
	  // Create the program from source
	  cl::Program::Sources sources(
			  1,
			  std::make_pair(kernelSourceCode, 0));
	  cl::Program program(context, sources);
	  //Build program
	  program.build(devices);
	  //Create buffer for A and copy host contents
	  cl::Buffer aBuffer = cl::Buffer(
		context,
		CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
		bufferSize * sizeof(int),
		(void *) &a[0]);
	  // Create buffer for B and copy host contents
	  cl::Buffer bBuffer = cl::Buffer(
		context,
		CL_MEM_READ_ONLY | CL_MEM_COPY_HOST_PTR,
		bufferSize * sizeof(int),
		(void *) &b[0]);
	  // Create buffer that uses the host ptr C
	  cl::Buffer cBuffer = cl::Buffer(
		context,
		CL_MEM_WRITE_ONLY | CL_MEM_COPY_HOST_PTR,
		bufferSize * sizeof(int),
		(void *) &c[0]);
	  // Create kernel object
	  cl::Kernel kernel(program, "vadd");
	  // Set kernel args
	  kernel.setArg(0, aBuffer);
	  kernel.setArg(1, bBuffer);
	  kernel.setArg(2, cBuffer);
	  queue.enqueueNDRangeKernel(
	    kernel,
	    cl::NullRange,
	    cl::NDRange(bufferSize),
	    cl::NullRange);
	  // Map cBuffer to host pointer. This enforces a sync with
	  // the host backing space; remember we chose a GPU device.
	  int * output = (int *) queue.enqueueMapBuffer(
	    cBuffer,
	    CL_TRUE,
	    CL_MAP_READ,
	    0,
	    bufferSize*sizeof(int)
	    );
	  for(int i = 0; i < bufferSize; i++)
	  {
		  std::cout << output[i] << " ";
	  }
	  std::cout << "\n";
	  // Finally release our hold on accessing the memory
	  cl_uint err = queue.enqueueUnmapMemObject(
			  cBuffer,
			  (void *) output);
	  if(err != CL_SUCCESS)
		  return false;
	}
	catch (cl::Error err)
	{
		std::cerr << err.what() <<  "(" << err.err() << ")" << std::endl;
		return false;
	}
	return true;
}

}

#endif /* ORAOPENCLWRAPPER_CXX_ */
