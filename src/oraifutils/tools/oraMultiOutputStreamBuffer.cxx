

#include "oraMultiOutputStreamBuffer.h"

#include <iostream>


namespace ora
{


// initialization
MultiOutputStreamBuffer moutbuf(&std::cout);
std::ostream mout(&moutbuf);
MultiOutputStreamBuffer merrbuf(&std::cerr);
std::ostream merr(&merrbuf);


MultiOutputStreamBuffer
::MultiOutputStreamBuffer(std::size_t buffersize)
  : std::streambuf(), m_Buffer(buffersize + 1)
{
  m_ForceFlush = true;
  m_OStreams.clear();
  char *beg = &m_Buffer.front();
  this->std::streambuf::setp(beg, beg + m_Buffer.size() - 1);
}

MultiOutputStreamBuffer
::MultiOutputStreamBuffer(std::ostream *os, std::size_t buffersize)
  : std::streambuf(), m_Buffer(buffersize + 1)
{
  m_ForceFlush = true;
  m_OStreams.clear();
  char *beg = &m_Buffer.front();
  this->std::streambuf::setp(beg, beg + m_Buffer.size() - 1);
  this->AddMultipleOutput(os);
}

MultiOutputStreamBuffer
::~MultiOutputStreamBuffer()
{
  m_OStreams.clear();
  m_Buffer.clear();
}

void
MultiOutputStreamBuffer
::AddMultipleOutput(std::ostream *os)
{
  if (!os)
    return;

  os->flush();
  os->clear();
  m_OStreams.push_back(os);
}

void
MultiOutputStreamBuffer
::RemoveMultipleOutput(std::ostream *os)
{
  if (!os)
    return;

  std::vector<std::ostream *>::iterator it = m_OStreams.begin();
  for (it = m_OStreams.begin(); it != m_OStreams.end(); ++it)
    if (*it == os)
    {
      (*it)->flush(); // flush before removing
      m_OStreams.erase(it); // remove all duplicates too; do not break
    }
}

void
MultiOutputStreamBuffer
::ClearMultipleOutputs()
{
  m_OStreams.clear();
}

unsigned int
MultiOutputStreamBuffer
::GetNumberOfMultiOutputs()
{
  return m_OStreams.size();
}

std::ostream *
MultiOutputStreamBuffer
::GetNthsMultiOutput(unsigned int n)
{
  if (n >= 0 && n < m_OStreams.size())
    return m_OStreams[n];
  else
    return NULL;
}

MultiOutputStreamBuffer::int_type
MultiOutputStreamBuffer
::overflow(int_type ch)
{
  if (m_OStreams.size() > 0 && ch != std::streambuf::traits_type::eof())
  {
    *pptr() = ch; // write to current put-array position
    pbump(1); // advance write position
    if (ExecuteFlush()) // do the real flush
      return ch;
  }
  return std::streambuf::traits_type::eof();
}

int
MultiOutputStreamBuffer
::sync()
{
  return (ExecuteFlush() ? 0 : -1); // simply flush
}

bool
MultiOutputStreamBuffer
::ExecuteFlush()
{
  std::ptrdiff_t sz = pptr() - pbase(); // size of flush content

  pbump(-sz); // set back write position

  unsigned x = m_OStreams.size();
  if (x > 0)
  {
    bool success = true;

    // write to all added output streams
    for (unsigned int i = 0; i < x; i++)
    {
      success = success && m_OStreams[i]->write(pbase(), sz);
      if (m_ForceFlush)
        m_OStreams[i]->flush(); // force them to flush as well
    }

    return success;
  }
  else
    return false;
}


}

