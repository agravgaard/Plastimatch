#ifndef ORAXYPLOTWIDGET_H
#define ORAXYPLOTWIDGET_H

#include <QMap>
#include <QPixmap>
#include <QVector>
#include <QWidget>

// Forward declarations
class QToolButton;
class QComboBox;
namespace ora
{
class PlotSettings;
}

namespace ora 
{

/**
 * A widget which visualizes a simple xy-plot. It displays one or more curves
 * specified as vectors of coordinates. Any number of curves are supported.
 * The user can zoom in on the area enclosed by a rubber band (QRubberBand
 * was not used to have finer control over the look).
 * For zooming a zoom out button and zoom in button is provided and appear the
 * on first time a rubber band is drawn (zoom in).
 * It maintains a stack of PlotSettings objects which correspond to a particular
 * zoom level.
 * In addition, curve data can be exported to CSV sheets.
 * Moreover, visibility of the curves can be controlled by a combo box.
 *
 * This class i based on an example in:
 * C++ GUI Programming with Qt 4 (second edition)
 * by Jasmin Blanchette and Mark Summerfield.
 * ISBN 0-13-235416-0
 *
 * @author Markus 
 * @author Philipp Steininger 
 * @version 1.2.1
 */
class XYPlotWidget: public QWidget
{
Q_OBJECT

public:
  typedef enum
  {
    /** zoom-in button tool tip **/
    CTT_ZOOM_IN_TT = 0,
    /** zoom-out button tool tip **/
    CTT_ZOOM_OUT_TT = 1,
    /** zoom-full button tool tip **/
    CTT_ZOOM_FULL_TT = 2,
    /** export CSV button tool tip **/
    CTT_CSV_TT = 3,
    /** visibility combo tool tip **/
    CTT_VIS_CMB_TT = 4,
    /** export csv file save dialog title **/
    CTT_CSV_SAVE_DLG_TITLE = 5,
    /** export csv file save dialog default file name **/
    CTT_CSV_SAVE_DLG_DEF_FILE = 6,
    /** export csv file save dialog file filter **/
    CTT_CSV_SAVE_DLG_FILE_FILTER = 7
  } CustomTextType;

  XYPlotWidget(QWidget *parent = 0);

  /** Specify the settings to use for displaying the plot.
   * The XYPlotWidget initially has a default zoom level. When a zooms in is
   * performed a new PlotSettings instance is created and put onto the zoom stack.
   * Zooming is controlled with #zoomStack and #curZoom.
   * When #SetPlotSettings() is called the zoom stack is cleared and the zoom
   * is set to the provided \p settings (zoom in/out buttons hidden).
   * @param settings PlotSettings that should be used for xy-chart.
   */
  void SetPlotSettings(const PlotSettings &settings);
  /** Sets the curve \p data for a given curve \p id.
   * If a curve with the same ID already exists it is replaced with the new one,
   * otherwise, the new curve is simply inserted.
   * @see #curveMap
   * @param id Curve id in the #curveMap.
   * @param data Curve data to plot.
   * @param updateSettings Determines the value-ranges of all curves, creates
   *    a PlotSettings instance and calls #SetPlotSettings(). Sets the view to
   *    the range of the data. Note: Is only performed if currently no manual
   *    zooming is performed.
   * @param name name of the curve
   * @param visibility determine whether or not the curve is initially rendered
   */
  void SetCurveData(int id, const QVector<QPointF> &data,
      const bool updateSettings = true, QString name = "",
      bool visibility = false);

  /** @return current number of curves **/
  int GetNumberOfCurves();

  /** Set the name of the specified curve. **/
  void SetCurveName(int id, QString name);
  /** Get the name of the specified curve. **/
  QString GetCurveName(int id);

  /** Set the visibility state of the specified curve. **/
  void SetCurveVisibility(int id, bool visibility);
  /** Get the visibility state of the specified curve. **/
  bool GetCurveVisibility(int id);

  /**
   * @param id Curve id in the #curveMap that should be returned.
   * @return The data of the specified curve from the curve map.
   */
  QVector<QPointF> & GetCurveData(int id);
  /** Removes the specified curve from the curve map.
   * @param id Curve id in the #curveMap that should be removed.
   * @param updateSettings Determines the value-ranges of all curves, creates
   *    a PlotSettings instance and calls #SetPlotSettings() after teh specified
   *    curve data was removed. Sets the view to the range of the remaining data.
   */
  void ClearCurve(int id, const bool updateSettings = true);
  /** Overridden function to specify the widget's ideal minimum size. The layout
   * never resizes a widget below its minimum size hint.
   * @see QWidget::minimumSizeHint()
   * @return Returns 200 x 133 (3:2 aspect ratio) to allow for the margin (50)
   *    on all four sides and some space for the plot itself. Below that size,
   *    the plot is too small to be useful.
   */
  QSize minimumSizeHint() const;
  /** Overridden function to specify a widget's ideal size.
   * @see QWidget::sizeHint()
   * @return The "ideal" size in proportion to the margin (50) with a
   *    3:2 aspect ratio.
   */
  QSize sizeHint() const;

  /** Set descriptive label for x-axis **/
  void SetXAxisLabel(const std::string &label);
  /** Set descriptive label for y-axis **/
  void SetYAxisLabel(const std::string &label);

  /** Set format character for number-to-string conversion (x-axis):
   * 'e','E','f', 'g', 'G', 'i' (=integer) **/
  void SetXTicksNumberFormat(char format);
  /** Set format character for number-to-string conversion (y-axis):
   * 'e','E','f', 'g', 'G', 'i' (=integer) **/
  void SetYTicksNumberFormat(char format);
  /** Set format precision for number-to-string-conversion (x-axis). **/
  void SetXTicksNumberPrecision(int precision);
  /** Set format precision for number-to-string-conversion (y-axis). **/
  void SetYTicksNumberPrecision(int precision);

  /** Set visibility (accessibility) of CSV export tool button. **/
  void SetCSVExportButtonVisibility(bool visible);
  /** Get visibility (accessibility) of CSV export tool button. **/
  bool GetCSVExportButtonVisibility();
  /** Set enabled state of CSV export tool button. **/
  void SetCSVExportButtonEnabled(bool enable);
  /** Get enabled of CSV export tool button. **/
  bool GetCSVExportButtonEnabled();

  /** Set visibility (accessibility) of visibility combo box. **/
  void SetVisibilityComboVisibility(bool visible);
  /** Get visibility (accessibility) of visibility combo box. **/
  bool GetVisibilityComboVisibility();
  /** Set enabled state of visibility combo box. **/
  void SetVisibilityComboEnabled(bool enable);
  /** Get enabled of visibility combo box. **/
  bool GetVisibilityComboEnabled();

  /** Set custom text (internationalization strings) for specified items (e.g.
   * tool tips). **/
  void SetCustomText(CustomTextType id, QString text);

public slots:
/** Zooms in if there is a zoom level in the zoom stack (previously zoomed in
 * and then zoom out). When the zoom stack has no next zoom level the rubber
 * band must be used to zoom in.
 * It moves one level deeper into the zoom stack (increments #curZoom) and
 * enables/disables the zoom in button and enables and shows the Zoom Out button.
 * The display is updated with a call to #RefreshPixmap().
 */
  void ZoomIn();
  /** Zoom out if the graph is zoomed in. It decrements the current zoom level
   * and enables the zoom in/out button (if possible).
   * The display is updated with a call to #RefreshPixmap().
   */
  void ZoomOut();
  /** Zoom to the data range.
   */
  void ZoomFull();
  /** Export current curve data into a CSV sheet.
   */
  void CSVExport();
  /** A visibility item (curve) changed (was activated).
   */
  void VisibilityChanged(int index);

protected:
  /** Normally performs all the drawing. But here all the plot drawing is done
   * beforehand in #RefreshPixmap(). The entire plot is rendered by copying the
   * pixmap onto the widget at position (0, 0).
   * If the rubber band is visible it is drawn on top of the plot (leaving the
   * off-screen pixmap untouched).
   * If the Widget has focus, a focus rectangle is drawn.
   * @see QWidget::paintEvent()
   * @see #rubberBandIsShown
   */
  void paintEvent(QPaintEvent *event);
  /** Places the zoom in/out buttons at the top right of the widget.
   * The display is updated with a call to #RefreshPixmap().
   * @see QWidget::resizeEvent()
   */
  void resizeEvent(QResizeEvent *event);
  /** Displays a rubber band when the user presses the left mouse button.
   * Sets #rubberBandIsShown and #rubberBandRect to the current mouse position.
   * Paints the rubber band with #UpdateRubberBandRegion() and changes the mouse
   * cursor to a crosshair.
   * @see QWidget::mousePressEvent()
   */
  void mousePressEvent(QMouseEvent *event);
  /** When the user moves the mouse cursor while holding down the left button
   * the rubber band region is updated by  #UpdateRubberBandRegion() and then
   * the #rubberBandRect is updated and repainted a second time.
   * @see QWidget::mouseMoveEvent()
   */
  void mouseMoveEvent(QMouseEvent *event);
  /** When the user releases the left mouse button the rubber band is erased and
   * the standard arrow cursor is restored.
   * If the rubber band is at least 4 x 4 a zoom is performed,else we do nothing.
   * @see QWidget::resizeEvent()
   */
  void mouseReleaseEvent(QMouseEvent *event);
  /** Implements key events for:
   * <ul>
   * <li>+: Zoom In</li>
   * <li>-: Zoom Out</li>
   * <li>Up: Scroll Up</li>
   * <li>Down: Scroll Down</li>
   * <li>Left: Scroll Left</li>
   * <li>Right: Scroll Right</li>
   * </ul>
   *
   * @see QWidget::keyPressEvent()
   */
  void keyPressEvent(QKeyEvent *event);
  /** Scroll by the requested number of ticks based on the the distance the
   * wheel was rotated.
   * @see QWidget::wheelEvent()
   */
  void wheelEvent(QWheelEvent *event);

  /** Update visibility combo content. **/
  void UpdateVisibilityCombo();

private:
  /** Forces a repaint of the area covered by the rubber band.
   * Erases or redraws the rubber band.
   */
  void UpdateRubberBandRegion();
  /** Update the display. Do not call QWidget::update() directly.
   * To keep the internal QPixmap up-to-date this function must be called.
   * It regenerates the internal pixmap and calls update() to copy the
   * pixmap onto the widget.
   */
  void RefreshPixmap();
  /** Updates the plot area and the text measures (#textHeight, #textWidth)
   * @painter QPainter to get text metrics.
   */
  void UpdatePlotArea(QPainter *painter);
  /** Draws a grid behind the curves and the axes.
   * @painter QPainter to plot the grid.
   */
  void DrawGrid(QPainter *painter);
  /** Draws the curves on top of the grid.
   * @painter QPainter to plot the curves.
   */
  void DrawCurves(QPainter *painter);

  enum
  {
    /** Constant used to provide some spacing around the text labels. */
    separator = 5,
    /** Constant used to provide some spacing around the graph. */
    border = 10
  };

  QToolButton *zoomInButton;
  QToolButton *zoomOutButton;
  QToolButton *zoomFullButton;
  QToolButton *csvExportButton;
  QComboBox *visibilityBox;
  QMap<int, QVector<QPointF> > curveMap;
  QMap<int, QString> curveNameMap;
  QMap<int, bool> curveVisibilityMap;
  /** Stack that holds the different zoom settings (for zoom in/out). **/
  QVector<PlotSettings> zoomStack;
  /** Current index in the zoomStack of active PlotSettings (current zoom). */
  int curZoom;
  /** Indicates if a rubber band for zooming is shown. */
  bool rubberBandIsShown;
  /** Represents a rubber band for zooming based on mouse events. */
  QRect rubberBandRect;
  /** Copy of the whole widget's rendering that is identical to what is shown
   * on the screen. The xy-plot is always drawn onto this off-screen pixmap
   * first and then copied onto the widget.
   */
  QPixmap pixmap;
  /** Descriptive label for x-axis **/
  QString xAxisLabel;
  /** Descriptive label for y-axis **/
  QString yAxisLabel;
  /** Format character for number-to-string conversion (x-axis):
   * 'e','E','f', 'g', 'G', 'i' (=integer) **/
  char xTicksNumberFormat;
  /** Format character for number-to-string conversion (y-axis):
   * 'e','E','f', 'g', 'G', 'i' (=integer) **/
  char yTicksNumberFormat;
  /** Format precision for number-to-string-conversion (x-axis). **/
  int xTicksNumberPrecision;
  /** Format precision for number-to-string-conversion (x-axis). **/
  int yTicksNumberPrecision;
  /** Area taht determine sthe plotting area. */
  QRect plotArea;
  /** Heigth of the plot labels (required for plotting). */
  int textHeight;
  /** Width of the y-axis plot labels (required for plotting). */
  int textWidth;
  /** Custom text strings for CSV export **/
  QVector<QString> csvCustomStrings;

};

/** Specifies the range of the x- and y-axes in plot coordinates and the
 * number of ticks.
 * By convention, numXTicks and numYTicks are off by one!
 * If numXTicks is 10 the XYPlotWidget will draw 11 tick marks on the x-axis.
 */
class PlotSettings
{
public:
  /** Constructor that initializes both axes to the range 0 to 10 with five
   * tick marks.
   */
  PlotSettings();

  /** Increments (or decrements) #minX, #maxX, #minY, and #maxY by the
   * interval between two ticks times a given number.
   * This function is used to implement scrolling in XYPlotWidget::keyPressEvent().
   */
  void Scroll(int dx, int dy);
  /** Rounds the #minX, #maxX, #minY, and #maxY values to "nice" values
   * determines the number of ticks appropriate for each axis (uses
   * #AdjustAxis() on each axis).
   * This function is used in mouseReleaseEvent().
   */
  void Adjust();

  double SpanX() const
  {
    return maxX - minX;
  }
  double SpanY() const
  {
    return maxY - minY;
  }

  double minX;
  double maxX;
  int numXTicks;
  double minY;
  double maxY;
  int numYTicks;

private:
  /** Converts its \a min and \a max parameters into "nice" numbers and sets
   * its \a numTicks parameter to the number of ticks it calculates to be
   * appropriate for the given [min, max] range.
   * The "nice" step values are numbers of the form 10^n, 2*10^n, or 5*10^n.
   * Note: modifies all input parameters.
   * This function will give sub-optimal results in some cases. A more
   * sophisticated is implemented in #AdjustAxisNice()
   * @param[in,out] min Minimum value in graph coordinates to adjust.
   * @param[in,out] max Maximum value in graph coordinates to adjust.
   * @param[in,out] numTicks Number of tics.
   */
  static void AdjustAxis(double &min, double &max, int &numTicks);

  /** Converts its \a min and \a max parameters into "nice" numbers and sets
   * its \a numTicks parameter to the number of ticks it calculates to be
   * appropriate for the given [min, max] range.
   * Based on "Nice Numbers for Graph Labels" by Paul Heckbert from
   * "Graphics Gems", Academic Press, 1990
   * @param[in,out] min Minimum value in graph coordinates to adjust.
   * @param[in,out] max Maximum value in graph coordinates to adjust.
   * @param[in,out] numTicks Number of tics.
   */
  static void AdjustAxisNice(double &min, double &max, int &numTicks);

  /** NiceNum: find a "nice" number approximately equal to x.
   * Based on "Nice Numbers for Graph Labels" by Paul Heckbert from
   * "Graphics Gems", Academic Press, 1990
   * @param x Number for which to find a "nice" value.
   * @param round If TRUE the number is rounded, else take ceiling.
   * @return Number approximately equal to x.
   */
  static double NiceNum(const double &x, const int &round);

};

} 

#endif  // ORAXYPLOTWIDGET_H
