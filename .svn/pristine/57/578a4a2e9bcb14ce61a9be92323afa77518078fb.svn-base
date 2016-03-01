

#ifndef ORAVTKSTRUCTURE_H_
#define ORAVTKSTRUCTURE_H_

#include <map>
#include <vector>

#include "vtkORAMemoryStructureSource.h"
// ORAIFTools
#include "oraSimpleMacros.h"

#include <vtkSmartPointer.h>
#include <vtkPolyData.h>
#include <vtkActor.h>
#include <vtkLODActor.h>
#include <vtkPolyDataMapper.h>
#include <vtkProperty.h>
#include <vtkCutter.h>


// Forward declarations
class vtkImplicitFunction;
class vtkStripper;

namespace ora
{


class VTKStructure; // forward declaration (see below)

/**
 * This class wraps a typical open radART structure set in VTK representation.
 *
 * FIXME (minimal implementation -> must later be transferred to a
 * structure set pool where the structures automatically detected to which
 * structure set they refer to)
 *
 * @author phil 
 * @author Markus 
 * @version 1.1
 */
class VTKStructureSet
{
public:
  /** Default constructor **/
  VTKStructureSet();
  /** Destructor **/
  virtual ~VTKStructureSet();

  /**
   * Get class-wide static default structure set which is intended to contain
   * all structures that have no dedicated structure set.
   * ; it can be retrieved without instantiation; SINGLETON.
   **/
  static VTKStructureSet *GetDefaultStructureSet();

  /**
   * Find a specified structure in the structure set.
   * @param UID identifier of the structure
   * @return the structure if found, NULL otherwise
   **/
  VTKStructure *FindStructure(const std::string UID);

  /**
   * Add a structure to the structure set. NOTE: the referencing for the
   * structure-object is automatically added.
   * @param structure the structure to be added
   * @return TRUE if the structure could be added
   */
  bool AddStructure(VTKStructure *structure);

  /**
   * Remove a structure from the structure set. NOTE: the dereferencing for the
   * structure-object is automatically applied.
   * @param structure the structure to be removed
   * @return TRUE if the structure could be removed
   */
  bool RemoveStructure(VTKStructure *structure);

  /**
   * Remove a structure from the structure set. NOTE: the dereferencing for the
   * structure-object is automatically applied.
   * @param UID the UID of the structure to be removed
   * @return TRUE if the structure could be removed
   */
  bool RemoveStructure(std::string UID);

  /**
   * Clear the complete structure set.
   */
  void Clear();

  /**
   * @return the UIDs of the contained structures.
   */
  std::vector<std::string> GetStructureUIDs();

  /**
   * @return a vector of the contained structures.
   */
  std::vector<VTKStructure *> GetStructures();

  /** @return the number of structures contained in this structure set **/
  unsigned int GetNumberOfStructures();

  SimpleVTKSetter(StructureSetUID, std::string)
  SimpleVTKGetter(StructureSetUID, std::string)

protected:
  /** Structure Set map type **/
  typedef std::map<std::string, VTKStructure *> STSMapType;

  /** global default structure set **/
  static VTKStructureSet *DefaultStructureSet;

  /** a map referencing the ORA structures by their UIDs **/
  STSMapType STSMap;

  // FIXME: later FOR reference and so on ...!
  std::string StructureSetUID;

};


/**
 * This class wraps a typical open radART structure in VTK representation
 * (vtkPolyData, vtkActor).
 *
 * FIXME (minimal implementation)
 *
 * @author phil 
 * @version 1.1
 */
class VTKStructure
{
public:
  /**
   * Generate a new VTK-structure instance by using shared ORA memory
   * blocks.
   * @param structSource an ORA memory structure source which is initialized
   * with the relevant blocks of memory
   * @return pointer to the generated VTK-structure (if successful),
   * NULL otherwise
   **/
  static VTKStructure *LoadFromORAMemory(
      VSP(structSource, vtkORAMemoryStructureSource));

  /** Default constructor **/
  VTKStructure();
  /** Destructor **/
  virtual ~VTKStructure();

  SimpleVTKGetter(StructurePolyData, vtkSmartPointer<vtkPolyData>)
  SimpleVTKSetter(StructurePolyData, vtkSmartPointer<vtkPolyData>)

  SimpleVTKGetter(StructureActor, vtkSmartPointer<vtkActor>)
  SimpleVTKSetter(StructureActor, vtkSmartPointer<vtkActor>)

  SimpleVTKGetter(StructureMapper, vtkSmartPointer<vtkPolyDataMapper>)
  SimpleVTKSetter(StructureMapper, vtkSmartPointer<vtkPolyDataMapper>)

  SimpleVTKGetter(ContourProperty, vtkSmartPointer<vtkProperty>)
  SimpleVTKSetter(ContourProperty, vtkSmartPointer<vtkProperty>)

  SimpleVTKGetter(StructureLODActor, vtkSmartPointer<vtkLODActor>)
  SimpleVTKSetter(StructureLODActor, vtkSmartPointer<vtkLODActor>)

  // FIXME: meta-data later!

  SimpleVTKGetter(StructureSet, VTKStructureSet*)
  SimpleVTKSetter(StructureSet, VTKStructureSet*)

  SimpleVTKGetter(StructureSetUID, std::string)
  SimpleVTKSetter(StructureSetUID, std::string)

  SimpleVTKGetter(StructureUID, std::string)
  SimpleVTKSetter(StructureUID, std::string)

  SimpleVTKGetter(StructureName, std::string)
  SimpleVTKSetter(StructureName, std::string)

  SimpleVTKGetter(ORAModificationTimeStamp, unsigned int)
  SimpleVTKSetter(ORAModificationTimeStamp, unsigned int)

  SimpleVTKGetter(FileName, std::string)
  SimpleVTKSetter(FileName, std::string)

  /**
   * Cut this structure object by a specified implicit function (e.g. a plane).
   * @param ifunc the implicit function defining the cut-surface
   * @param copy if TRUE the generated poly data output will be (deep) copied
   * and will, therefore, not depend on this structure's life-time; if FALSE
   * the poly data output will be a pointer to the internal cutter's output;
   * NOTE: if TRUE the generated poly data output must externally be freed!
   * Moreover - if FALSE - the output will be valid until this method is
   * called again.
   * @param simplify if TRUE the generated cut contours will be simplified as
   * far as possible; this means that the contours are stripped and manually
   * re-assembled in order to form continuous lines per contour; this may be
   * essential for some applications; activating this features will slightly
   * slow-down the cutting procedure
   * @param didSimplification relevant if simplify=TRUE: returns whether or not
   * simplification algorithms was invoked; it this parameter returns TRUE, you
   * have to delete (call Delete()-method) the returned contour poly data after
   * usage!
   * @return the cut poly data output or NULL (if failed); have a look on the
   * copy-parameter
   */
  vtkPolyData *CutByImplicitFunction(vtkImplicitFunction *ifunc,
      bool copy, bool simplify, bool &didSimplification);

protected:
  /** pure underlying poly data representation **/
  VSP(StructurePolyData, vtkPolyData);
  /** actor representation (containing additional appearance) **/
  VSP(StructureActor, vtkActor);
  /** internal poly data mapper for actor representation **/
  VSP(StructureMapper, vtkPolyDataMapper);
  /** internal property for contour representation (cutting) **/
  VSP(ContourProperty, vtkProperty);
  /**
   * level of detail actor representation (containing additional appearance)
   * for time-critical rendering
   **/
  VSP(StructureLODActor, vtkLODActor);
  /** internal cutter helper (instantiated once) **/
  VSP(Cutter, vtkCutter);
  /** internal stripper helper (instantiated once) **/
  VSP(Stripper, vtkStripper);

  // FIXME: transfer to a meta-data-construct later!
  /** object-reference to the parent structure set **/
  VTKStructureSet *StructureSet;
  /** UID of the parent structure set **/
  std::string StructureSetUID;
  /** structure's unique identifier **/
  std::string StructureUID;
  /** structure's name **/
  std::string StructureName;
  /** ORA last modification time stamp (ms since start of day) **/
  unsigned int ORAModificationTimeStamp;
  /** Associate file name (if existing) **/
  std::string FileName;

  /**
   * Force the structure to (re-)create an actor representation based on a
   * structure source's internal settings and its internal poly data.
   * @param structSource a required structure source (ORA shared memory)
   * @return TRUE if successful
   * @see GetStructureActor()
   */
  virtual bool CreateNewStructureActor(
      VSP(structSource, vtkORAMemoryStructureSource));

  /**
   * Force the structure to (re-)create a property for contour (cutting)
   * representation based on a structure source's internal settings.
   * @param structSource a required structure source (ORA shared memory)
   * @return TRUE if successful
   * @see GetContourProperty()
   */
  virtual bool CreateNewContourProperty(
      VSP(structSource, vtkORAMemoryStructureSource));

};


}


#endif
