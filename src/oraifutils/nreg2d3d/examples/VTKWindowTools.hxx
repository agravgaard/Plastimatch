#ifndef VTKWINDOWTOOLS_HXX_
#define VTKWINDOWTOOLS_HXX_

/**
 * Some VTK window related tools for examples.
 * @author phil <philipp.steininger (at) pmu.ac.at>
 * @version 1.0
 */

#include <vtksys/SystemTools.hxx>
#include <vtkSmartPointer.h>
#include <vtkCallbackCommand.h>
#include <vtkRenderWindow.h>
#include <vtkRenderer.h>
#include <vtkRenderWindowInteractor.h>
#include <vtkTable.h>
#include <vtkContextView.h>
#include <vtkContextScene.h>
#include <vtkChartXY.h>
#include <vtkPlot.h>
#include <vtkAxis.h>
#include <vtkLookupTable.h>
#include <vtkImageMapToColors.h>
#include <vtkImageBlend.h>
#include <vtkDataSetMapper.h>
#include <vtkActor.h>
#include <vtkImageData.h>
#include <vtkImageImport.h>
#include <vtkTooltipItem.h>

#include <itkImageRegionConstIterator.h>
#include <itkVTKImageExport.h>

/** VTK callback function type **/
typedef void(*VTKCallbackFunctionType)(vtkObject *, unsigned long, void *,
    void *);

typedef struct OverlayViewerObjectsStruct
{
  vtkRenderWindow *renWin;
  vtkLookupTable *colorMap1;
  vtkLookupTable *colorMap2;
  vtkImageMapToColors *colorMapper1;
  vtkImageMapToColors *colorMapper2;
  vtkImageBlend *blender;
  double level1;
  double window1;
  double level2;
  double window2;

  OverlayViewerObjectsStruct()
  {
    renWin = NULL;
    colorMap1 = NULL;
    colorMap2 = NULL;
    colorMapper1 = NULL;
    colorMapper2 = NULL;
    blender = NULL;
    level1 = 0; // if level==window: full range
    window1 = 0;
    level2 = 0;
    window2 = 0;
  }

} OverlayViewerObjects;

/** Window geometry information **/
typedef struct WindowGeometryStruct
{
  int posX;
  int posY;
  int width;
  int height;
  // optional
  double level1;
  double window1;
  double level2;
  double window2;

  WindowGeometryStruct()
  {
    posX = -1;
    posY = -1;
    width = -1;
    height = -1;
    level1 = 0; // if level==window: full range
    window1 = 0;
    level2 = 0;
    window2 = 0;
  }

} WindowGeometry;

/**
 * Create a minimal 2D VTK image (3rd dimension has 1 pixel) from an ITK image
 * (2D or 3D).
 * NOTE: the pixel type will be set to FLOAT!<br>
 * NOTE: no metrics or spatial information will be taken over!<br>
 * NOTE: the image will be constructed on the heap - must be deleted externally!
 **/
template<typename ITKImageType> vtkImageData *Create2DVTKImageFromITKImage(
    ITKImageType *itkImage)
{
  if (itkImage)
  {
    vtkImageData *image = vtkImageData::New();
    image->SetNumberOfScalarComponents(1);
    image->SetScalarTypeToFloat();
    image->SetDimensions(itkImage->GetLargestPossibleRegion().GetSize()[0],
        itkImage->GetLargestPossibleRegion().GetSize()[1], 1);
    image->AllocateScalars();
    typedef itk::ImageRegionConstIterator<ITKImageType> IteratorType;
    IteratorType it(itkImage, itkImage->GetLargestPossibleRegion());
    float *pixels = static_cast<float *> (image->GetScalarPointer());
    int x = 0;
    // simply copy the pixels:
    for (it.GoToBegin(); !it.IsAtEnd(); ++it, ++x)
      pixels[x] = static_cast<float> (it.Get());

    return image;
  }
  else
  {
    return NULL;
  }
}

/**
 * Cast ITK image to VTK image (is a "snapshot").
 * NOTE: the image will be constructed on the heap - must be deleted externally!
 **/
template<typename ITKImageType>
vtkImageData *ConnectVTKImageToITKImage(const ITKImageType *itkImage)
{
  if (!itkImage)
    return NULL;

  typedef itk::VTKImageExport<ITKImageType> ITKExporterType;
  typedef typename ITKExporterType::Pointer ITKExporterPointer;
  typedef vtkSmartPointer<vtkImageImport> VTKImporterPointer;

  // connect VTK-pipeline to ITK-pipeline:
  ITKExporterPointer itkExporter = ITKExporterType::New();

  itkExporter->SetInput(itkImage); // export the ITK image object

  VTKImporterPointer vtkImporter = VTKImporterPointer::New();

  // most important: connect the callbacks of both pipelines
  vtkImporter->SetUpdateInformationCallback(
      itkExporter->GetUpdateInformationCallback());
  vtkImporter->SetPipelineModifiedCallback(
      itkExporter->GetPipelineModifiedCallback());
  vtkImporter->SetWholeExtentCallback(itkExporter->GetWholeExtentCallback());
  vtkImporter->SetSpacingCallback(itkExporter->GetSpacingCallback());
  vtkImporter->SetOriginCallback(itkExporter->GetOriginCallback());
  vtkImporter->SetScalarTypeCallback(itkExporter->GetScalarTypeCallback());
  vtkImporter->SetNumberOfComponentsCallback(
      itkExporter->GetNumberOfComponentsCallback());
  vtkImporter->SetPropagateUpdateExtentCallback(
      itkExporter->GetPropagateUpdateExtentCallback());
  vtkImporter->SetUpdateDataCallback(itkExporter->GetUpdateDataCallback());
  vtkImporter->SetDataExtentCallback(itkExporter->GetDataExtentCallback());
  vtkImporter->SetBufferPointerCallback(itkExporter->GetBufferPointerCallback());
  vtkImporter->SetCallbackUserData(itkExporter->GetCallbackUserData());

  // import the VTK image object
  vtkImporter->Update(); // update immediately

  vtkImageData *vtkImage = vtkImageData::New();
  vtkImage->ShallowCopy(vtkImporter->GetOutput());

  return vtkImage;
}

/**
 * Create a standard VTK Window with some optional props.
 */
vtkRenderWindow *CreateVTKWindow(bool addInteractor = true, std::string title =
    "", int posX = 0, int posY = 0, int width = 1, int height = 1,
    bool initialize = true, VTKCallbackFunctionType customCB = NULL, int event =
        0)
{
  vtkRenderWindow *renWin = vtkRenderWindow::New();
  vtkSmartPointer<vtkRenderer> ren = vtkSmartPointer<vtkRenderer>::New();
  vtkSmartPointer<vtkRenderWindowInteractor> iren = NULL;

  if (addInteractor)
    iren = vtkSmartPointer<vtkRenderWindowInteractor>::New();
  renWin->AddRenderer(ren);
  if (addInteractor)
    iren->SetRenderWindow(renWin);

  renWin->SetWindowName(title.c_str());
  renWin->SetPosition(posX, posY);
  renWin->SetSize(width, height);

  if (addInteractor && initialize)
    iren->Initialize();

  if (customCB)
  {
    vtkSmartPointer<vtkCallbackCommand> cmd = vtkSmartPointer<
        vtkCallbackCommand>::New();
    cmd->SetClientData(renWin);
    cmd->SetCallback(customCB);
    iren->AddObserver(event, cmd);
  }

  return renWin;
}

/** Create an overlay image viewer. **/
OverlayViewerObjects CreateOverlayViewer(std::string title = "", int posX = 0,
    int posY = 0, int width = 1, int height = 1)
{
  vtkImageData *dummy1 = vtkImageData::New();
  dummy1->SetNumberOfScalarComponents(1);
  dummy1->SetScalarTypeToFloat();
  dummy1->SetDimensions(10, 10, 1);
  dummy1->SetSpacing(1, 1, 1);
  dummy1->AllocateScalars();
  float *pixels = static_cast<float *> (dummy1->GetScalarPointer());
  for (int i = 0; i < 100; i++)
    pixels[i] = i;
  vtkImageData *dummy2 = vtkImageData::New();
  dummy2->SetNumberOfScalarComponents(1);
  dummy2->SetScalarTypeToFloat();
  dummy2->SetDimensions(10, 10, 1);
  dummy2->SetSpacing(1, 1, 1);
  dummy2->AllocateScalars();
  pixels = static_cast<float *> (dummy2->GetScalarPointer());
  for (int i = 0; i < 100; i++)
    pixels[i] = i;

  vtkRenderWindow *viewer = CreateVTKWindow(true, title, posX, posY, width,
      height, true, NULL, 0);
  vtkSmartPointer<vtkLookupTable> cm1 = vtkSmartPointer<vtkLookupTable>::New();
  cm1->SetTableRange(0, 99); // to be adjusted to image scalar range!
  cm1->SetValueRange(0, 1);
  cm1->SetHueRange(0, 0);
  cm1->SetSaturationRange(1, 1);
  cm1->SetAlphaRange(1, 1);
  cm1->Build();
  vtkSmartPointer<vtkLookupTable> cm2 = vtkSmartPointer<vtkLookupTable>::New();
  cm2->SetTableRange(0, 99); // to be adjusted to image scalar range!
  cm2->SetValueRange(0, 1);
  cm2->SetHueRange(0.3333333, 0.3333333);
  cm2->SetSaturationRange(1, 1);
  cm2->SetAlphaRange(1, 1);
  cm2->Build();
  vtkSmartPointer<vtkImageMapToColors> colorMapper1 = vtkSmartPointer<
      vtkImageMapToColors>::New();
  colorMapper1->SetInput(dummy1); // connect image here!
  colorMapper1->SetLookupTable(cm1);
  vtkSmartPointer<vtkImageMapToColors> colorMapper2 = vtkSmartPointer<
      vtkImageMapToColors>::New();
  colorMapper2->SetInput(dummy2); // connect image here!
  colorMapper2->SetLookupTable(cm2);
  vtkSmartPointer<vtkImageBlend> blender =
      vtkSmartPointer<vtkImageBlend>::New();
  blender->SetOpacity(0, 0.5);
  blender->SetOpacity(1, 0.5);
  blender->SetInput(0, colorMapper1->GetOutput());
  blender->SetInput(1, colorMapper2->GetOutput());

  // visual pipeline
  vtkSmartPointer<vtkDataSetMapper> dsm =
      vtkSmartPointer<vtkDataSetMapper>::New();
  dsm->SetInput(reinterpret_cast<vtkDataSet*> (blender->GetOutput()));
  vtkSmartPointer<vtkActor> actor = vtkSmartPointer<vtkActor>::New();
  actor->SetMapper(dsm);
  viewer->GetRenderers()->GetFirstRenderer()->AddActor(actor);
  viewer->GetRenderers()->GetFirstRenderer()->SetBackground(1, 1, 1);

  OverlayViewerObjects info; // return info
  info.renWin = viewer;
  info.colorMap1 = cm1;
  info.colorMap2 = cm2;
  info.colorMapper1 = colorMapper1;
  info.colorMapper2 = colorMapper2;
  info.blender = blender;

  return info;
}

/** Set up a specified window as simple x/y plot. **/
vtkSmartPointer<vtkRenderWindow> SetupVTKGraphWindow(
    vtkSmartPointer<vtkTable> dataTable, int posX = 0, int posY = 0, int width =
        1, int height = 1, int lineWidth = 1, double lineRed = 0,
    double lineGreen = 0, double lineBlue = 0, double lineAlpha = 255,
    std::string xAxisTitle = "", std::string yAxisTitle = "",
    vtkSmartPointer<vtkTooltipItem> toolTip = NULL)
{
  if (dataTable)
  {
    vtkSmartPointer<vtkContextView> view =
        vtkSmartPointer<vtkContextView>::New();
    view->GetRenderer()->SetBackground(1.0, 1.0, 1.0);

    vtkSmartPointer<vtkChartXY> chart = vtkSmartPointer<vtkChartXY>::New();
    view->GetScene()->AddItem(chart);
    vtkPlot *line = chart->AddPlot(vtkChart::LINE);
    chart->GetAxis(0)->SetTitle(yAxisTitle.c_str());
    chart->GetAxis(0)->SetGridVisible(true);
    chart->GetAxis(1)->SetTitle(xAxisTitle.c_str());
    chart->GetAxis(1)->SetGridVisible(true);
    line->SetInput(dataTable, 0, 1); // iterations vs. similarity measure
    line->SetColor(lineRed, lineGreen, lineBlue, lineAlpha);
    line->SetWidth(lineWidth);
    if (toolTip)
      view->GetScene()->AddItem(toolTip);
    vtkSmartPointer<vtkRenderWindowInteractor> iren = vtkSmartPointer<
        vtkRenderWindowInteractor>::New();
    vtkSmartPointer<vtkRenderWindow> renWin = view->GetRenderWindow();
    iren->SetRenderWindow(renWin);
    renWin->SetPosition(posX, posY);
    renWin->SetSize(width, height);
    iren->Initialize();

    return renWin;
  }
  else
  {
    return NULL;
  }
}

#endif /* VTKWINDOWTOOLS_HXX_ */
