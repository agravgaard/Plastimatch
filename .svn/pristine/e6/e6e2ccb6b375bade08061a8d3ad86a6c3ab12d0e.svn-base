

#ifndef ORAMULTIOUTPUTSTREAMBUFFER_H_
#define ORAMULTIOUTPUTSTREAMBUFFER_H_


#include <streambuf>
#include <vector>
#include <ostream>


namespace ora
{


/**
 * Provides a stream buffer that pipes the content (overflows and flushes) to
 * a variable number of configured output streams. Therefore, this class forms
 * a multiple output fitting into the STL streaming framework.
 *
 * In addition, global 'standard' error and output objects (cerr/cout
 * equivalents) and the according MultiOutputStreamBuffer-stream-buffers are
 * provided.
 * @see moutbuf
 * @see mout
 * @see merrbuf
 * @see merr
 *
 * <p>
 * Example of usage:<br>
 * <code>
 * ora::moutbuf.ClearMultipleOutputs(); <br>
 * ora::moutbuf.AddMultipleOutput(&std::cout); <br>
 * std::ofstream fs; <br>
 * fs.open("test.txt", std::ofstream::app); <br>
 * ora::moutbuf.AddMultipleOutput(&fs); <br>
 * ora::mout << "Hello World " << std::endl; // written to cout and fs ! <br>
 * // redirect the cerr to mout <br>
 * std::cerr.rdbuf(ora::mout.rdbuf()); <br>
 * std::cerr << "Hello World 2" << std::endl; // written to cout and fs ! <br>
 * </code>
 * </p>
 *
 * <p>
 * <b>NOTE:</b> Be careful using this class! There are several traps. For a
 * example it usually does not make sense to add a specified output stream
 * multiple times when using concatenations of the "<<"-operator. Furthermore,
 * when other output streams are redirected to an instance of
 * MultiOutputStreamBuffer, and the this ouput streams is a multiple output
 * of the buffer at the same time, a cyclic loop is created!
 * </p>
 *
 * @author phil 
 * @version 1.2
 */
class MultiOutputStreamBuffer
  : public std::streambuf
{
public:
  /** Default constructor **/
  MultiOutputStreamBuffer(std::size_t buffersize = 256);
  /** Constructor with one multiple output **/
  MultiOutputStreamBuffer(std::ostream *os, std::size_t buffersize = 256);

  /** Destructor **/
  virtual ~MultiOutputStreamBuffer();

  /**
   * Add a new multiple output stream to this object.
   * @param os pointer to the output stream to be added <br>
   * NOTE: is not protected
   * against duplicates, therefore an ouput stream can be added multiple
   * times) <br>
   * NOTE: the responsibility for memory management is external!
   */
  void AddMultipleOutput(std::ostream *os);

  /**
   * Remove a specified multiple output stream from this object.
   * @param os pointer to the output stream to be removed <br>
   * NOTE: all duplicates are removed <br>
   * NOTE: the memory is not deleted, just the pointer reference!
   */
  void RemoveMultipleOutput(std::ostream *os);

  /**
   * Clear all multiple output streams from this object.
   * NOTE: the memory is not deleted, just the pointer references!
   */
  void ClearMultipleOutputs();

  /** @return Get number of currently configured multiple output streams. **/
  unsigned int GetNumberOfMultiOutputs();

  /**
   * @param n the index of the multiple output stream to be returned (must
   * be in the range of [0; GetNumberOfMultiOutputs()[ )
   * @return a pointer to the n-th configured multiple output stream
   */
  std::ostream *GetNthsMultiOutput(unsigned int n);

  /** Set flag indicating that output streams are force-flushed on buffer flush **/
  void SetForceFlush(bool forceFlush)
  {
    m_ForceFlush = forceFlush;
  }
  /** Get flag indicating that output streams are force-flushed on buffer flush **/
  bool GetForceFlush()
  {
    return m_ForceFlush;
  }

protected:
  /** a list of pointers to the multiple output streams **/
  std::vector<std::ostream *> m_OStreams;
  /** internal byte buffer **/
  std::vector<char> m_Buffer;
  /** flag indicating that output streams are force-flushed on buffer flush **/
  bool m_ForceFlush;

  /**
   * Buffer overflow - flush is required. Called whenever pptr() == epptr().
   * @return something other than traits_type::eof() on success
   * @see std::streambuf::overflow(int_type)
   **/
  int_type overflow(int_type ch);

  /**
   * Write current data to target(s).
   * @return -1 on failure
   * @see std::streambuf::sync()
   */
  int sync();

  /**
   * Do the pure flushing to target output stream(s).
   * @return TRUE on success (had to do something)
   */
  bool ExecuteFlush();

};


/**
 * global multiple 'standard' output object stream buffer
 * (routed to single output std::cout by default)
 **/
extern MultiOutputStreamBuffer moutbuf;
/** global multiple 'standard' output object **/
extern std::ostream mout;
/**
 * global multiple 'standard' error object stream buffer
 * (routed to single output std::cerr by default)
 **/
extern MultiOutputStreamBuffer merrbuf;
/** global multiple 'standard' error object **/
extern std::ostream merr;


}


#endif /* ORAMULTIOUTPUTSTREAMBUFFER_H_ */

