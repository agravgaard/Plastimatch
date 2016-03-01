

#ifndef ORAINIACCESS_H
#define ORAINIACCESS_H


#include <string.h>
#include <vector>
#include <map>

#include "oraStringTools.h"


namespace ora
{

/** \class IniAccessSectionEntry
 *  \brief This class implements the abstraction of an INI file section.
 *
 * <p>This class is implemented to abstract from typical (Windows) INI file
 * sections and provides access to them. It more or less encapsulates the
 * indents with their values and/or optional comments of a section.</p>
 *
 * @author phil 
 * @author Markus 
 * @version 1.3.1
 */
class IniAccessSectionEntry
{

public:
  /**
   * Structure with central operator for sorting the indents within the
   * internal map.
   */
  struct AscStringComp
  {
    /** Just an ordinary string-literal-comparator. **/
    bool operator()(const std::string s1, const std::string s2) const
    {
      return (strcmp(s1.c_str(), s2.c_str()) < 0);
    }
  };
  /** Map-type of the internal map used for indent-/comment-management. **/
  typedef std::map<std::string, std::string, AscStringComp> IndentMapType;

  /**
    * Construct new logical INI file section instance.
    * @param sectionName name of the section (it is expected to be unique
    * within the INI file)
    */
  IniAccessSectionEntry(std::string sectionName);

  /**
    * Destroy this logical INI file section instance.
    */
  ~IniAccessSectionEntry();

  /**
    * Adds an indent (or comment) to the logical space of this section.
    * @param indent name of the new indent (indents starting with the [
    * character are comments - then value parameter is ignored and the rest
    * of indent parameter after [ character is expected to be the comment
    * itself)
    * @param value value of the indent (can also be the empty string)
    */
  void AddIndent(std::string indent, std::string value);

  /**
    * Deletes the specified indent (or comment) from the logical space of this
    * section.
    * @param indent the indent to be deleted (if found in logical section space)
    */
  void DeleteIndent(std::string indent);

  /**
    * @return returns a pointer to internal indents-map containing the indents
    * together with their values and optional comments
    */
  IndentMapType *GetIndents();

  /**
    * @return returns the name of this logical section space as string
    */
  std::string GetSectionName();

private:
  /** holds the name of this logical section space **/
  std::string m_SectionName;
  /** internal indents-map containing the indents, their values and comments **/
  IndentMapType *m_Indents;

};


/** \class IniAccess
 *  \brief This class implements the abstraction of a typical INI file.
 *
 * <p>This class is implemented to abstract from typical (Windows) INI files
 * and to provide access to the sections, keys (indents), value, comments
 * and pre-headers.</p>
 *
 * <p><b>General Format of an INI file:</b><br>
 * pre-header-space<br>
 * [section1-name]<br>
 * indent1-name=value<br>
 * indent2-name=value<br>
 * ...<br>
 * optional-comments<br>
 * [section2-name]<br>
 * indent1-name=value<br>
 * indent2-name=value<br>
 * ...<br>
 * optional-comments<br>
 * ...<br></p>
 *
 * <p><b>Some intuitive rules on INI files:</b>
 * <ul>
 * <li>primary comments are introduced using ; or // characters</li>
 * <li>comments, sections and indents are case-sensitive</li>
 * <li>pre-header entries generally should contain comments concerning the
 *     the complete INI file</li>
 * <li>sections, keys, comments and values must not exceed the region of
 *     one line (must not include carriage return or line feed) </li>
 * <li>sections form groups of logically related indents (keys, values)</li>
 * <li>the order of sections witin an INI file or the order of indents
 *     within a section plays no role primary</li>
 * <li>section-names themselves must not contain [ or ] characters</li>
 * <li>indent-names must not contain [ or ] at the first position,
 *     furthermore they must not contain the = character at any position</li>
 * <li>there are no special restrictions on values except the ones already
 *     denoted (values are generally represented as strings) </li>
 * <li>indents and values are separated by = character, there must not be
 *     any spaces between the = character and the indent or value
 *     respectively (otherwise the spaces are discovered being part of indent
 *     or value respectively)</li>
 * <li>optional comments can be added to each section, but are expected
 *     to be unique within the belonging section (the easiest way to achive
 *     uniqueness is to consecutively number the comment lines)</li>
 * <li>white lines are allowed everywhere</li>
 * </ul></p>
 *
 * @author Philipp Steininger 
 * @version 1.1
 */
class IniAccess
{

public:
  /**
   * Construct new instance. Configuration file's name must be set later
   * using @see SetFileName(std::string).
   */
  IniAccess();
  /**
   * Construct new instance.
   * @param fileName name of configuration file to be abstracted from
   */
  IniAccess(std::string fileName);
  /**
   * Destroy this instance.
   */
  ~IniAccess();

  /**
   * Add a new pre-header entry (one line). It is recommended to introduce
   * pre-header entries using // or ; characters.
   * @param newHeaderEntry header entry to be added
   */
  void AddPreHeader(const std::string newHeaderEntry);

  /**
   * Delete all comments of a specific section.
   * @param section the section under consideration
   * @return true if there have been any comments (and have therefore been
   * deleted), false otherwise
   */
  bool DeleteComments(const std::string section);

  /**
   * Delete a specified indent of a specific section.
   * @param section the section under consideration
   * @param indent the indent to be deleted
   * @return true if the indent has been found (and has therefore been
   * deleted), false otherwise
   */
  bool DeleteIndent(const std::string section, const std::string indent);

  /**
    * Delete the pre-header space of the INI file.
    * @return true if any pre-header entries have been found and deleted,
    * false otherwise
    */
  bool DeletePreHeader();

  /**
    * Delete a specified section of the INI file.
    * @return true if the section has been found and deleted,
    * false otherwise
    */
  bool DeleteSection(const std::string section);

  /**
    * Discards the changes made to the INI file since load. This means
    * that no changes will be written back to INI file during destruction
    * of object or calling @see Update().
    */
  void DiscardChanges();

  /**
    * @return current configured file name of INI file (you can load a
    * specified INI file, change its name and then call @see Update()
    * to write it back to another file)
    */
  std::string GetFileName();

  /**
    * Returns a copy of the internal comments-vector (list of optional
    * comments) of a specified section.<br>
    * <b>The returned vector must be externally released using the
    * delete-keyword after usage!</b>
    * @param section the specified section
    * @return copy of internal comment-vector of the specified section
    */
  std::vector<std::string> *GetComments(const std::string section);

  /**
    * Returns a copy of the internal indents-vector (list of
    * indents) of a specified section.<br>
    * <b>The returned vector must be externally released using the
    * delete-keyword after usage!</b>
    * @param section the specified section
    * @return copy of internal indent-vector of the specified section
    */
  std::vector<std::string> *GetIndents(const std::string section);

  /**
    * Returns a copy of the internal pre-header-entries-vector (list of
    * pre-header-entries) of the INI file.<br>
    * <b>The returned vector must be externally released using the
    * delete-keyword after usage!</b>
    * @return copy of internal pre-header-entries-vector
    */
  std::vector<std::string> *GetPreHeader();

  /**
    * Returns a copy of the internal sections-vector (list of
    * sections) of the INI file.<br>
    * <b>The returned vector must be externally released using the
    * delete-keyword after usage!</b>
    * @return copy of internal sections-vector
    */
  std::vector<std::string> *GetSections();

  /**
   * Returns a direct pointer to the internal section-maps (list of
   * IniAccessSectionEntry pointers).<br>
   * <b>Be careful using the returned pointers! Should be read-only!</b>
   * @return direct pointer to the internal section-maps
   */
  std::vector<IniAccessSectionEntry *> *GetSectionMaps();

  /**
    * Returns whether specified section is existing or not.
    * @param section searched section
    * @return true if specified section is found, false otherwise
    */
  bool IsSectionExisting(std::string section);

  /**
    * Returns whether specified indent in specified section is existing or not.
    * @param section specified sections
    * @param indent searched indents
    * @return true if specified indent is found, false otherwise
    */
  bool IsIndentExisting(std::string section, std::string indent);

  /**
    * Loads the header, sections, indents, values and comments from configured
    * file and creates accompanying vectors in memory.
    */
  void LoadValues();

  /**
    * Returns the value of the specified section-indent-combination. The value
    * is represented as std::string.
    * @param section name of the section (must not be empty string)
    * @param indent name of the indent
    * @param defaultValue default value to be returned if the indent is not
    * found in the section (empty string is default)
    * @param uncUNIXConversion if TRUE then the read string is automatically
    * UNC-UNIX-converted
    * @return the value represented as std::string if found, defaultValue
    * otherwise
    */
  std::string ReadString(const std::string section, const std::string indent,
    const std::string defaultValue = "", const bool uncUNIXConversion = false);

  /**
    * Returns the value of the specified section-indent-combination. The value
    * is represented as template type T.<br>
    * Due to some ANSI-C++ isues this template method is directly implemented
    * within this header file.
    * @param section name of the section (must not be empty string)
    * @param indent name of the indent
    * @param defaultValue default value to be returned if the indent is not
    * found in the section (the type of defaultValue is template type T)
    * @return the value represented as template type T if found, defaultValue
    * otherwise
    */
  template<typename T> T ReadValue(const std::string section,
    const std::string indent, const T defaultValue)
  {
    std::string s = ReadString(section, indent, "");
    if (s.length() > 0)
      return StreamConvert<T, std::string>(s);
    else
      return defaultValue;
  }

  /**
    * Set the file name of INI file. This can be used if the default constructor
    * is called before to set the file name afterwards and then manually load
    * the file (if you would like to read an INI file). It can also used to
    * set the file name of a completely new INI file if want to write-only an
    * INI file. Or you can also use it to implicitely rename an existing INI
    * file read before.
    * @param fileName new file name to be set
    */
  void SetFileName(std::string fileName);

  /**
    * Updates (synchronizes) physical file content with memory content. This
    * means that all changes made since load or last @see DiscardChanges() are
    * applied to the configured file @see SetFileName(std::string).
    */
  bool Update();

  /**
    * Writes a value to the specified section-indent-combination. The value
    * is always represented as std::string. Also comments can be created using
    * this method.
    * @param section the specified section the value should be written to
    * @param indent the specified indent the value should be written to;
    * if you apply a [ character at the beginning of the indent parameter, the
    * rest of the parameter is assumed to be a comment (e.g. ]// comment) - the
    * value parameter is then ignored; the indent parameter can also be empty
    * (--> only the section is written)
    * @param value the value as std::string that should be written; the value
    * is also allowed to be an empty string
    * @return true if value has been written to the specified indent in
    * specified section (or section has been written if indent parameter is
    * empty string or comment has been written if indent parameter starts with
    * [ character), false otherwise
    */
  bool WriteString(const std::string section, const std::string indent,
    const std::string value);

  /**
    * Writes a value to the specified section-indent-combination. The value
    * is represented as template type T. Also comments can be created using
    * this method.<br>
    * Due to some ANSI-C++ isues this template method is directly implemented
    * within this header file.
    * @param section the specified section the value should be written to
    * @param indent the specified indent the value should be written to;
    * if you apply a [ character at the beginning of the indent parameter, the
    * rest of the parameter is assumed to be a comment (e.g. ]// comment) - the
    * value parameter is then ignored; the indent parameter can also be empty
    * (--> only the section is written)
    * @param value the value as template type T that should be written
    * @return true if value has been written to the specified indent in
    * specified section (or section has been written if indent parameter is
    * empty string or comment has been written if indent parameter starts with
    * [ character), false otherwise
    */
  template<typename T> bool WriteValue(const std::string section,
    const std::string indent, const T value)
  {
    return WriteString(section, indent, StreamConvert(value));
  }

  /**
   * Set whether automatic UNC vs. UNIX conversion should be applied when
   * reading string-values from config file. NOTE: depending on the size of
   * the global conversion table and on the frequency of ReadString() or
   * ReadValue() invocation, this flag can influence application performance!
   * Furthermore, this mode is quite dangerous as it really converts each string
   * into file format, e.g. " " (space) is transformed to "\ "! Be careful!
   */
  void SetInvokeUNCvsUNIXOnRead(bool invoke)
  {
    m_InvokeUNCvsUNIXOnRead = invoke;
  }
  /**
   * Get whether automatic UNC vs. UNIX conversion should be applied when
   * reading string-values from config file.
   */
  bool GetInvokeUNCvsUNIXOnRead()
  {
    return m_InvokeUNCvsUNIXOnRead;
  }

  /**
   * Set flag indicating whether an open radART conformant file checksum should
   * be generated or not.
   */
  void SetAddORACheckSum(bool addCS)
  {
    m_AddORACheckSum = addCS;
  }

  /**
   * Get flag indicating whether an open radART conformant file checksum should
   * be generated or not.
   */
  bool GetAddORACheckSum()
  {
    return m_AddORACheckSum;
  }

protected:
  /** stored file name of INI **/
  std::string m_Filename;
  /** vector containing pointers to actual abstracted INI section entries **/
  std::vector<IniAccessSectionEntry *> *m_Sections;
  /** vector containing actual pre-header space **/
  std::vector<std::string> *m_PreHeader;
  /** flag indicating whether content has been changed or no since load
    * or last Update() **/
  bool m_ContentChanged;
  /**
   * flag indicating whether global UNC vs. UNIX conversions should
   * automatically be invoked on read processes
   **/
  bool m_InvokeUNCvsUNIXOnRead;
  /**
   * flag indicating whether an open radART conformant file checksum should
   * be generated or not
   */
  bool m_AddORACheckSum;

  /**
    * Prints out actual contents of internal vectors (pre-header, sections,
    * indents, values and comments) into an output-stream (typically a file
    * stream).
    * @param os an output stream (typically a file stream)
    */
  void PrintOut(std::ostream &os);

  /**
   * Generate the open radART conformant check sum for current file content.
   * NOTE: this method must be invoked before updating memory with the
   * physical file.
   */
  void GenerateORACheckSum();

private:
  /**
    * Initializes the object itself.
    */
  void Init();

};

}

#endif /* ORAINIACCESS_H */
