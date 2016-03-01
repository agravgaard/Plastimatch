//#include <QtGui>
#include <QPalette>
#include <QSizePolicy>
#include <QIcon>
#include <QStylePainter>
#include <QStyleOptionFocusRect>
#include <QMouseEvent>
#include <QLocale>
#include <QFileDialog>
#include <QComboBox>

#include <QDir>
#include <QFileInfoList>

// Forward declarations
#include <QToolButton>

#include <cmath>
#include <iostream>
#include <limits>
#include <math.h>
#include <fstream>

#include "oraXYPlotWidget.h"

namespace ora 
{

XYPlotWidget::XYPlotWidget(QWidget *parent) :
  QWidget(parent)
{
  /* Use the "dark" component of the palette as the color for "erasing" the
   * widget (instead of the "window" component).
   * This is the default color that is used to fill any newly revealed pixels
   * when the widget is resized to a larger size (before paintEvent()).
   * setAutoFillBackground(true) is required to enable this mechanism.
   * (By default child widgets inherit the background from their parent widget)
   */
  //setBackgroundRole(QPalette::Dark);
  //setAutoFillBackground(true);
  // Tell any layout manager that this widget is especially willing to grow (can also shrink)
  setSizePolicy(QSizePolicy::Expanding, QSizePolicy::Expanding);
  // Make the widget accept focus by clicking or by pressing Tab
  setFocusPolicy(Qt::StrongFocus);
  rubberBandIsShown = false;

  csvCustomStrings.clear();
  csvCustomStrings.push_back("Export curves to CSV sheet ...");
  csvCustomStrings.push_back("curve.csv");
  csvCustomStrings.push_back("CSV sheet (*.csv)");

  // Create buttons to zoom in and out
  // adjustSize() sets the sizes to their size hints
  // Since no layouts are used the buttons' parent must be specified
  zoomInButton = new QToolButton(this);
  zoomInButton->setIcon(QIcon(":xyp/img/zoomin"));
  zoomInButton->adjustSize();
  connect(zoomInButton, SIGNAL(clicked()), this, SLOT(ZoomIn()));

  zoomOutButton = new QToolButton(this);
  zoomOutButton->setIcon(QIcon(":xyp/img/zoomout"));
  zoomOutButton->adjustSize();
  connect(zoomOutButton, SIGNAL(clicked()), this, SLOT(ZoomOut()));

  zoomFullButton = new QToolButton(this);
  zoomFullButton->setIcon(QIcon(":xyp/img/zoomfull"));
  zoomFullButton->adjustSize();
  zoomFullButton->show();
  connect(zoomFullButton, SIGNAL(clicked()), this, SLOT(ZoomFull()));

  csvExportButton = new QToolButton(this);
  csvExportButton->setIcon(QIcon(":xyp/img/save"));
  csvExportButton->adjustSize();
  csvExportButton->setVisible(false); // initially not visible
  connect(csvExportButton, SIGNAL(clicked()), this, SLOT(CSVExport()));

  visibilityBox = new QComboBox(this);
  visibilityBox->setVisible(false); // initially not visible
  connect(visibilityBox, SIGNAL(activated(int)), this, SLOT(VisibilityChanged(int)));

  xTicksNumberFormat = 'f';
  xTicksNumberPrecision = 3;
  yTicksNumberFormat = 'f';
  yTicksNumberPrecision = 3;

  // Initialize settings
  SetPlotSettings(PlotSettings());
}

void XYPlotWidget::SetCustomText(CustomTextType id, QString text)
{
  if (id == CTT_CSV_SAVE_DLG_DEF_FILE)
    csvCustomStrings[1] = text;
  else if (id == CTT_CSV_SAVE_DLG_FILE_FILTER)
    csvCustomStrings[2] = text;
  else if (id == CTT_CSV_SAVE_DLG_TITLE)
    csvCustomStrings[0] = text;
  else if (id == CTT_CSV_TT)
    csvExportButton->setToolTip(text);
  else if (id == CTT_VIS_CMB_TT)
    visibilityBox->setToolTip(text);
  else if (id == CTT_ZOOM_FULL_TT)
    zoomFullButton->setToolTip(text);
  else if (id == CTT_ZOOM_IN_TT)
    zoomInButton->setToolTip(text);
  else if (id == CTT_ZOOM_OUT_TT)
    zoomOutButton->setToolTip(text);
}

void XYPlotWidget::SetPlotSettings(const PlotSettings &settings)
{
  zoomStack.clear();
  zoomStack.append(settings);
  curZoom = 0;
  zoomInButton->hide();
  zoomOutButton->hide();
  RefreshPixmap();
}

void XYPlotWidget::ZoomOut()
{
  if (curZoom > 0)
  {
    --curZoom;
    zoomOutButton->setEnabled(curZoom > 0);
    zoomInButton->setEnabled(true);
    zoomInButton->show();
    RefreshPixmap();
  }
}

void XYPlotWidget::ZoomIn()
{
  if (curZoom < zoomStack.count() - 1)
  {
    ++curZoom;
    zoomInButton->setEnabled(curZoom < zoomStack.count() - 1);
    zoomOutButton->setEnabled(true);
    zoomOutButton->show();
    RefreshPixmap();
  }
}

void XYPlotWidget::CSVExport()
{
  QString fn = QFileDialog::getSaveFileName(this,
      csvCustomStrings[0], csvCustomStrings[1], csvCustomStrings[2]);
  if (!fn.isEmpty())
  {
    QVector<QVector<QString> > records;
    int c = 0;
    QLocale loc;
    QMapIterator<int, QVector<QPointF> > cdit(curveMap);
    QMapIterator<int, QString > cnit(curveNameMap);
    while (cdit.hasNext() && cnit.hasNext())
    {
      c++;
      cdit.next();
      cnit.next();
      QString name = cnit.value();
      if (name.isEmpty())
        name = "c" + QString::number(c, 10);

      QVector<QString> rec;

      // header:
      rec.push_back(name + "-x;" + name + "-y");

      // data lines:
      QVector<QPointF> data = cdit.value();
      for (int j = 0; j < data.count(); ++j)
      {
        rec.push_back(loc.toString(data[j].x(), 'f', 6) + ";" +
            loc.toString(data[j].y(), 'f', 6));
      }

      records.push_back(rec);
    }

    // write to one common CSV sheet!
    std::ofstream f(fn.toStdString().c_str());

    int maxrecs = 0;
    for (int i = 0; i < records.size(); i++)
    {
      if (records[i].size() > maxrecs)
        maxrecs = records[i].size();
    }
    for (int j = 0; j < maxrecs; j++)
    {
      for (int i = 0; i < records.size(); i++)
      {
        if (i > 0)
          f << ";";
        if (records[i].size() > j)
        {
          f << records[i][j].toStdString();
        }
        else
        {
          f << ";";
        }
      }
      f << std::endl;
    }

    f.close();

  }
}

void XYPlotWidget::ZoomFull()
{
  // No data: use default settings
  if(curveMap.isEmpty())
  {
    SetPlotSettings(PlotSettings());
    return;
  }

  PlotSettings settings;
  settings.minX = std::numeric_limits<double>::max();
  settings.minY = std::numeric_limits<double>::max();
  // NOTE: settings.maxX = numeric_limits<>::min is not used as initialization
  // because it behaves differently for floating points and integers.
  // For floating points it will return the smallest positive number and
  // for integers the lowest number. For example:
  // numeric_limits<double>::min() = 2.22507e-308
  // numeric_limits<char>::min() = -128
  settings.maxX = -std::numeric_limits<double>::max();
  settings.maxY = -std::numeric_limits<double>::max();

  QMapIterator<int, QVector<QPointF> > it(curveMap);
  QMapIterator<int, bool > itv(curveVisibilityMap);
  while (it.hasNext() && itv.hasNext())
  {
    it.next();
    itv.next();

    if (!itv.value()) // ignore invisible curves for full-zoom-computation!
      continue;

    QVector<QPointF> data = it.value();
    QPolygonF polyline(data.count());

    for (int j = 0; j < data.count(); ++j)
    {
      settings.minX = std::min(settings.minX, data[j].x());
      settings.minY = std::min(settings.minY, data[j].y());
      settings.maxX = std::max(settings.maxX, data[j].x());
      settings.maxY = std::max(settings.maxY, data[j].y());
    }
  }
  SetPlotSettings(settings);
}

void XYPlotWidget::SetCurveData(int id, const QVector<QPointF> &data,
    const bool updateSettings, QString name, bool visibility)
{
  // Do nothing when data is empty (makes no sense to set empty data vector)
  if (data.isEmpty())
    return;

  curveNameMap[id] = name;
  curveVisibilityMap[id] = visibility;
  curveMap[id] = data;

  UpdateVisibilityCombo();

  if (updateSettings && curZoom == 0)
  {
    ZoomFull();
  }

  RefreshPixmap();
}

QVector<QPointF> & XYPlotWidget::GetCurveData(int id)
{
  return curveMap[id];
}

void XYPlotWidget::SetCurveName(int id, QString name)
{
  if (curveNameMap.find(id) == curveNameMap.end())
    return;
  curveNameMap[id] = name;
  UpdateVisibilityCombo();
}

QString XYPlotWidget::GetCurveName(int id)
{
  if (curveNameMap.find(id) == curveNameMap.end())
    return "";
  return curveNameMap[id];
}

void XYPlotWidget::SetCurveVisibility(int id, bool visibility)
{
  if (curveVisibilityMap.find(id) == curveVisibilityMap.end())
    return;
  curveVisibilityMap[id] = visibility;
  UpdateVisibilityCombo();
  RefreshPixmap();
}

bool XYPlotWidget::GetCurveVisibility(int id)
{
  if (curveVisibilityMap.find(id) == curveVisibilityMap.end())
    return false;
  return curveVisibilityMap[id];
}

int XYPlotWidget::GetNumberOfCurves()
{
  return curveMap.size();
}

void XYPlotWidget::UpdateVisibilityCombo()
{
  visibilityBox->clear();
  QIcon idraw(":xyp/img/draw");
  QIcon inotdraw(":xyp/img/not-draw");

  QMapIterator<int, QString > cnit(curveNameMap);
  QMapIterator<int, bool > cvit(curveVisibilityMap);
  QVariant v;

  while (cnit.hasNext() && cvit.hasNext())
  {
    cnit.next();
    cvit.next();
    v.setValue<int>(cnit.key());
    if (cvit.value())
      visibilityBox->addItem(idraw, cnit.value(), v);
    else
      visibilityBox->addItem(inotdraw, cnit.value(), v);
  }
  visibilityBox->adjustSize();
}

void XYPlotWidget::ClearCurve(int id, bool updateSettings)
{
  curveMap.remove(id);
  curveNameMap.remove(id);
  curveVisibilityMap.remove(id);
  UpdateVisibilityCombo();
  if (updateSettings)
  {
    ZoomFull();
  }
  RefreshPixmap();
}

QSize XYPlotWidget::minimumSizeHint() const
{
  return QSize(200, 133); // 3:2 aspect ratio
}

QSize XYPlotWidget::sizeHint() const
{
  return QSize(200 + (3 * separator + textHeight + textWidth + border), 133
      + (border + 3 * separator + 2 * textHeight));
}

void XYPlotWidget::SetXAxisLabel(const std::string &label)
{
  xAxisLabel = QString::fromStdString(label);
  RefreshPixmap();
}

void XYPlotWidget::SetYAxisLabel(const std::string &label)
{
  yAxisLabel = QString::fromStdString(label);
  RefreshPixmap();
}

void XYPlotWidget::paintEvent(QPaintEvent * /* event */)
{
  QStylePainter painter(this);
  painter.drawPixmap(0, 0, pixmap);

  if (rubberBandIsShown)
  {
    painter.setPen(palette().dark().color());
    // Draw rubber band. QRect::normalized() ensures that the rubber band
    // rectangle has positive width and height (swapping coord. if necessary)
    // and reduce the size of the rectangle by one pixel (1-pixel-wide outline)
    painter.drawRect(rubberBandRect.normalized().adjusted(0, 0, -1, -1));
  }

  if (hasFocus())
  {
    QStyleOptionFocusRect option;
    option.initFrom(this);
    option.backgroundColor = palette().dark().color();
    painter.drawPrimitive(QStyle::PE_FrameFocusRect, option);
  }
}

void XYPlotWidget::resizeEvent(QResizeEvent * /* event */)
{
  // Place buttons at top-right corner, which depends on the size of the widget
  // Qt always generates a resize event before a widget is shown for the first time
  int buttonSpace = 5;
  int x = 1;
  int y = height() - 1 - zoomFullButton->height();
  zoomFullButton->move(x, y);
  zoomOutButton->move(x + zoomFullButton->width() + buttonSpace, y);
  zoomInButton->move(x + zoomFullButton->width() + zoomOutButton->width() + 2
      * buttonSpace, y);

  y = zoomInButton->y();
  x = width() - csvExportButton->width() - 1;
  csvExportButton->move(x, y);
  y = height() - 1 - visibilityBox->height();
  x = x - buttonSpace - visibilityBox->width();
  visibilityBox->move(x, y);

  RefreshPixmap();
}

void XYPlotWidget::SetVisibilityComboVisibility(bool visible)
{
  visibilityBox->setVisible(visible);
}

bool XYPlotWidget::GetVisibilityComboVisibility()
{
  return visibilityBox->isVisible();
}

void XYPlotWidget::SetVisibilityComboEnabled(bool enable)
{
  visibilityBox->setEnabled(enable);
}

bool XYPlotWidget::GetVisibilityComboEnabled()
{
  return visibilityBox->isEnabled();
}

void XYPlotWidget::SetCSVExportButtonVisibility(bool visible)
{
  csvExportButton->setVisible(visible);
}

bool XYPlotWidget::GetCSVExportButtonVisibility()
{
  return csvExportButton->isVisible();
}

void XYPlotWidget::SetCSVExportButtonEnabled(bool enable)
{
  csvExportButton->setEnabled(enable);
}

bool XYPlotWidget::GetCSVExportButtonEnabled()
{
  return csvExportButton->isEnabled();
}

void XYPlotWidget::mousePressEvent(QMouseEvent *event)
{
  if (event->button() == Qt::LeftButton)
  {
    if (plotArea.contains(event->pos()))
    {
      rubberBandIsShown = true;
      rubberBandRect.setTopLeft(event->pos());
      rubberBandRect.setBottomRight(event->pos());
      UpdateRubberBandRegion();
      setCursor(Qt::CrossCursor);
    }
  }
}

void XYPlotWidget::mouseMoveEvent(QMouseEvent *event)
{
  if (rubberBandIsShown)
  {
    UpdateRubberBandRegion();
    rubberBandRect.setBottomRight(event->pos());
    UpdateRubberBandRegion();
  }
}

void XYPlotWidget::mouseReleaseEvent(QMouseEvent *event)
{
  if ((event->button() == Qt::LeftButton) && rubberBandIsShown)
  {
    rubberBandIsShown = false;
    UpdateRubberBandRegion();
    unsetCursor();

    QRect rect = rubberBandRect.normalized();
    if (rect.width() < 4 || rect.height() < 4)
      return;

    // Convert the rubber band from widget to plot window coordinates
    rect.translate(-plotArea.x(), -plotArea.y());
    PlotSettings prevSettings = zoomStack[curZoom];
    PlotSettings settings;
    double dx = prevSettings.SpanX() / (width() - (width() - plotArea.width()));
    double dy = prevSettings.SpanY() / (height() - (height()
        - plotArea.height()));
    settings.minX = prevSettings.minX + dx * rect.left();
    settings.maxX = prevSettings.minX + dx * rect.right();
    settings.minY = prevSettings.maxY - dy * rect.bottom();
    settings.maxY = prevSettings.maxY - dy * rect.top();
    // Round the plot window coordinates and find ticks for each axis
    settings.Adjust();

    // Perform zoom in
    zoomStack.resize(curZoom + 1);
    zoomStack.append(settings);
    ZoomIn();
  }
}

void XYPlotWidget::keyPressEvent(QKeyEvent *event)
{
  switch (event->key())
  {
    case Qt::Key_Plus:
      ZoomIn();
      break;
    case Qt::Key_Minus:
      ZoomOut();
      break;
    case Qt::Key_Left:
      zoomStack[curZoom].Scroll(-1, 0);
      RefreshPixmap();
      break;
    case Qt::Key_Right:
      zoomStack[curZoom].Scroll(+1, 0);
      RefreshPixmap();
      break;
    case Qt::Key_Down:
      zoomStack[curZoom].Scroll(0, -1);
      RefreshPixmap();
      break;
    case Qt::Key_Up:
      zoomStack[curZoom].Scroll(0, +1);
      RefreshPixmap();
      break;
    default:
      QWidget::keyPressEvent(event);
  }
}

void XYPlotWidget::wheelEvent(QWheelEvent *event)
{
  int numDegrees = event->delta() / 8;
  int numTicks = numDegrees / 15;

  if (event->orientation() == Qt::Horizontal)
  {
    zoomStack[curZoom].Scroll(numTicks, 0);
  }
  else
  {
    zoomStack[curZoom].Scroll(0, numTicks);
  }
  RefreshPixmap();
}

void XYPlotWidget::UpdateRubberBandRegion()
{
  // Erase or redraw the rubber band by four calls to update() that schedule
  // a paint event for the four small rectangular areas that are covered by
  // the rubber band (two vertical and two horizontal lines)
  QRect rect = rubberBandRect.normalized();
  update(rect.left(), rect.top(), rect.width(), 1);
  update(rect.left(), rect.top(), 1, rect.height());
  update(rect.left(), rect.bottom(), rect.width(), 1);
  update(rect.right(), rect.top(), 1, rect.height());
}

void XYPlotWidget::RefreshPixmap()
{
  // Resize the pixmap to the same size as the widget
  pixmap = QPixmap(size());
  // Fill it with the widget's erase color
  pixmap.fill(this, 0, 0);

  // Create a QPainter to draw on the pixmap
  QPainter painter(&pixmap);
  // Set painter's pen, background, and font ti the widget settings
  painter.initFrom(this);
  // Perform the drawing
  UpdatePlotArea(&painter);
  DrawGrid(&painter);
  DrawCurves(&painter);
  // Repaint the widget
  update();
}

void XYPlotWidget::DrawGrid(QPainter *painter)
{
  // If the widget is not large enough do nothing
  if (!plotArea.isValid())
    return;

  PlotSettings settings = zoomStack[curZoom];
  QPen dark = palette().dark().color();
  QPen darker = palette().dark().color().darker();

  // Draw the grid's vertical lines and ticks along the x-axis
  QLocale loc;
  for (int i = 0; i <= settings.numXTicks; ++i)
  {
    int x = plotArea.left() + (i * (plotArea.width() - 1) / settings.numXTicks);
    double label = settings.minX + (i * settings.SpanX() / settings.numXTicks);
    painter->setPen(dark);
    painter->drawLine(x, plotArea.top(), x, plotArea.bottom());
    painter->setPen(darker);
    painter->drawLine(x, plotArea.bottom(), x, plotArea.bottom() + 5);
    QString s;
    if (xTicksNumberFormat != 'i')
      s = loc.toString(label, xTicksNumberFormat, xTicksNumberPrecision);
    else
      s = loc.toString((int) label);
    painter->drawText(x - 50, plotArea.bottom() + separator, 100, 20,
        Qt::AlignHCenter | Qt::AlignTop, s);
  }

  // Draw the grid's horizontal lines and ticks along the y-axis
  for (int j = 0; j <= settings.numYTicks; ++j)
  {
    int y = plotArea.bottom() - (j * (plotArea.height() - 1)
        / settings.numYTicks);
    double label = settings.minY + (j * settings.SpanY() / settings.numYTicks);
    painter->setPen(dark);
    painter->drawLine(plotArea.left(), y, plotArea.right(), y);
    painter->setPen(darker);
    painter->drawLine(plotArea.left() - 5, y, plotArea.left(), y);
    QString s;
    if (yTicksNumberFormat != 'i')
      s = loc.toString(label, yTicksNumberFormat, yTicksNumberPrecision);
    else
      s = loc.toString((int) label);
    painter->drawText(plotArea.left() - separator - textWidth, y - 10,
        textWidth, 20, Qt::AlignRight | Qt::AlignVCenter, s);
  }

  // Draw a rectangle along the margins
  painter->drawRect(plotArea.adjusted(0, 0, -1, -1));

  // Draw axis labels
  painter->save();
  painter->setPen(darker);
  painter->setFont(QFont(painter->font().family(), painter->font().pixelSize(),
      QFont::Normal));
  painter->drawText(plotArea.left() + (plotArea.width() / 2.0) - this->width()
      / 2.0, plotArea.bottom() + 2 * separator + textHeight, this->width(), 20,
      Qt::AlignCenter | Qt::AlignTop, xAxisLabel);
  painter->translate(plotArea.left() - textWidth - textHeight - 2 * separator,
      plotArea.bottom() - (plotArea.height() / 2.0) + this->height() / 2.0);
  painter->rotate(-90);
  painter->drawText(0, 0, this->height(), 20, Qt::AlignCenter | Qt::AlignTop,
      yAxisLabel);
  painter->restore();
}

void XYPlotWidget::DrawCurves(QPainter *painter)
{
  static const QColor colorForIds[7] =
  { Qt::blue, Qt::red, Qt::green, Qt::black, Qt::yellow, Qt::magenta, Qt::cyan};
  PlotSettings settings = zoomStack[curZoom];

  // If the widget is not large enough do nothing
  if (!plotArea.isValid())
    return;

  // Set the QPainter's clip region to the rectangle that contains the curves
  // (exclude the margins and the frame around the graph)
  // QPainter will then ignore drawing operations on pixels outside the area
  painter->setClipRect(plotArea.adjusted(+1, +1, -1, -1));

  // Iterate over all the curves
  QMapIterator<int, QVector<QPointF> > i(curveMap);
  QMapIterator<int, bool > iv(curveVisibilityMap);
  while (i.hasNext() && iv.hasNext())
  {
    i.next();
    iv.next();

    if (!iv.value()) // do not show invisible curves!
      continue;

    int id = i.key();
    QVector<QPointF> data = i.value();
    QPolygonF polyline(data.count());

    // Converts each point from plotter coordinates to widget coordinates
    for (int j = 0; j < data.count(); ++j)
    {
      double dx = data[j].x() - settings.minX;
      double dy = data[j].y() - settings.minY;
      double x = plotArea.left() + (dx * (plotArea.width() - 1)
          / settings.SpanX());
      double y = plotArea.bottom() - (dy * (plotArea.height() - 1)
          / settings.SpanY());
      // Store point in polyline
      polyline[j] = QPointF(x, y);
    }

    // Draw the polyline
    QPen p(colorForIds[uint(id) % 7]);
    p.setWidth(1);
    painter->setPen(p);
    painter->drawPolyline(polyline);
  }
}

void XYPlotWidget::SetXTicksNumberFormat(char format)
{
  xTicksNumberFormat = format;
  RefreshPixmap();
}

void XYPlotWidget::SetYTicksNumberFormat(char format)
{
  yTicksNumberFormat = format;
  RefreshPixmap();
}

void XYPlotWidget::SetXTicksNumberPrecision(int precision)
{
  xTicksNumberPrecision = precision;
  RefreshPixmap();
}

void XYPlotWidget::SetYTicksNumberPrecision(int precision)
{
  yTicksNumberPrecision = precision;
  RefreshPixmap();
}

void XYPlotWidget::VisibilityChanged(int index)
{
  QVariant v = visibilityBox->itemData(index, Qt::UserRole);
  if (v != QVariant::Invalid)
  {
    bool ok = false;
    int i = v.toInt(&ok);
    if (ok && i >= 0 && i < curveVisibilityMap.size())
    {
      curveVisibilityMap[i] = !curveVisibilityMap[i]; // toggle
      UpdateVisibilityCombo();
      RefreshPixmap();
    }
  }
}

void XYPlotWidget::UpdatePlotArea(QPainter *painter)
{
  if (zoomStack.size() < 1)
    return;
  PlotSettings settings = zoomStack[curZoom];
  QFontMetrics fm = painter->fontMetrics();
  // Get text height
  textHeight = fm.height();

  // Find required width of y-axis tics with min and max label
  double label = settings.minY;
  QString s;
  QLocale loc;
  if (yTicksNumberFormat != 'i')
    s = loc.toString(label, yTicksNumberFormat, xTicksNumberPrecision);
  else
    s = loc.toString((int) label);
  int textWidthN = fm.width(s);

  label = settings.maxY;
  if (yTicksNumberFormat != 'i')
    s = loc.toString(label, yTicksNumberFormat, xTicksNumberPrecision);
  else
    s = loc.toString((int) label);
  textWidthN = std::max(textWidthN, fm.width(s)) + 5;

  // Only change when > 5px difference (avoids flickering)
  if (std::fabs(static_cast<double>(textWidthN - textWidth)) > 5)
    textWidth = textWidthN;

  // set new plot area
  int leftMargin = 3 * separator + textHeight + textWidth;
  int topMargin = border;
  int width = this->width() - border - leftMargin;
  int height = this->height() - border - 3 * separator - 2 * textHeight;
  plotArea.setRect(leftMargin, topMargin, width, height);
}

PlotSettings::PlotSettings()
{
  minX = 0.0;
  maxX = 10.0;
  numXTicks = 5;

  minY = 0.0;
  maxY = 10.0;
  numYTicks = 5;
}

void PlotSettings::Scroll(int dx, int dy)
{
  double stepX = SpanX() / numXTicks;
  minX += dx * stepX;
  maxX += dx * stepX;

  double stepY = SpanY() / numYTicks;
  minY += dy * stepY;
  maxY += dy * stepY;
}

void PlotSettings::Adjust()
{
  AdjustAxisNice(minX, maxX, numXTicks);
  AdjustAxisNice(minY, maxY, numYTicks);
}

void PlotSettings::AdjustAxis(double &min, double &max, int &numTicks)
{
  // Computing "gross step" (a kind of maximum for the step value)
  const int minTicks = 4;
  double grossStep = (max - min) / minTicks;
  /* Find the corresponding number of the form 10^n that is smaller than or
   * equal to the gross step.
   * Take the decimal logarithm of the gross step, round that value down to a
   * whole number and raise 10 to the power of this rounded number.
   * e.g. gross step = 236, compute log 236 = 2.37291..., round down to 2 and
   * obtain 10^2 = 100 as the candidate step value of the form 10^n.
   */
  double step = std::pow(10.0, std::floor(std::log10(grossStep)));

  // Calculate the other two candidates: 2*10^n and 5*10^n.
  // e.g. 200 and 500. 500 > gross step 236. 200 < 236 => use 200 as step size
  if (5 * step < grossStep)
  {
    step *= 5;
  }
  else if (2 * step < grossStep)
  {
    step *= 2;
  }

  /* The new numTicks is the number of intervals between the rounded min and
   * max values. e.g. min=240, max=1184; new range [200, 1200] with 5 tick marks
   */
  numTicks = int(std::ceil(max / step) - std::floor(min / step));
  if (numTicks < minTicks)
    numTicks = minTicks;
  // Round the original min down to the nearest multiple of the step
  min = std::floor(min / step) * step;
  // Round up to the nearest multiple of the step
  max = std::ceil(max / step) * step;
}

void PlotSettings::AdjustAxisNice(double &min, double &max, int &numTicks)
{
  const int nTick = 5; /* desired number of tick marks */
  double d = 1.0; /* tick mark spacing */
  double range = 1.0;

  /* we expect min!=max */
  range = PlotSettings::NiceNum(max - min, 0);
  d = PlotSettings::NiceNum(range / (nTick - 1), 1);
  if (d < 0)
    return;
  min = std::floor(min / d) * d;
  max = std::ceil(max / d) * d;
  numTicks = std::abs(max - min) / d;
}

double PlotSettings::NiceNum(const double &x, const int &round)
{
  int expv = 1; /* exponent of x */
  double f = 1.0; /* fractional part of x */
  double nf = 1.0; /* nice, rounded fraction */

  if (x <= 0)
    return 1;
  expv = std::floor(std::log10(x));
  f = x / std::pow(10., (double) expv); /* between 1 and 10 */
  if (round)
    if (f < 1.5)
      nf = 1.;
    else if (f < 3.)
      nf = 2.;
    else if (f < 7.)
      nf = 5.;
    else
      nf = 10.;
  else if (f <= 1.)
    nf = 1.;
  else if (f <= 2.)
    nf = 2.;
  else if (f <= 5.)
    nf = 5.;
  else
    nf = 10.;
  return nf * std::pow(10., (double) expv);
}

} 

