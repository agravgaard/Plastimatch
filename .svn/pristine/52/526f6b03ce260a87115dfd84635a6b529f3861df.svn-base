

#include <stdlib.h>
#include <vector>
#include <iostream>
#include <fstream>
#include "itksys/SystemTools.hxx"


/**
 * A small tool application that takes some input parameters (an application
 * specification and its arguments), converts them into short-path-
 * representation, and finally calls this configured application with its
 * short-path-arguments.
 * @author phil
 * @version 1.0
 */
int main(int argc, char *argv[])
{

  // shortpathexec.exe application arg1 arg2 ...

  if (argc < 2)
  {
    std::cerr << "Wrong application call." << std::endl;
    std::cerr << argv[0] << " [--generate-empty-file file] application " <<
      "arg1 arg2 ..." << std::endl;
    return EXIT_FAILURE;
  }

  int offset = 1;

  if (argc >= 3 &&
      std::string(argv[1]) == "--generate-empty-file" &&
      std::string(argv[2]).length() > 0)
  {
    // -> create an empty file with a specified name if it does not already
    // exist: this is often useful because short-path-conversion does not work
    // if the file does not exist!
    if (!itksys::SystemTools::FileExists(argv[2], true))
    {
      std::ofstream outfile(argv[2]);
      outfile.close();
      std::cout << "Empty '" << argv[2] << "' generated." << std::endl;
    }

    offset += 2;
  }

  std::vector<std::string> shortpaths;
  shortpaths.clear();
  for (int i = offset; i < argc; i++)
  {
    //std::cout << "argv[" << i << "]=" << argv[i] << std::endl;
    //std::cout << "(len=" << std::string(argv[i]).length() << ")" << std::endl;
    std::string s = "";
    if (itksys::SystemTools::GetShortPath(argv[i], s))
      shortpaths.push_back(s);
    else
      shortpaths.push_back(std::string(argv[i]));
  }

  if (shortpaths.size() < 1)
  {
    std::cerr << "Could not convert to short path(s)." << std::endl;
    return EXIT_FAILURE;
  }

  std::string appstring = shortpaths[0];
  for (unsigned int i = 1; i < shortpaths.size(); i++)
    appstring += " " + shortpaths[i];

  // execute the converted application
  std::cout << std::endl << std::endl << appstring.c_str() << std::endl << std::endl;
  system(appstring.c_str());

  return EXIT_SUCCESS;
}
