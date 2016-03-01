#include <QtPlugin>
#include <QDesignerCustomWidgetInterface>
#include "cview_portal_plugin.h"
#include "cview_portal.h"

PortalWidgetPlugin::PortalWidgetPlugin (QObject* parent)
    : QObject (parent)
{
}

QString
PortalWidgetPlugin::name () const
{
    return "PortalWidget";
}

QString
PortalWidgetPlugin::includeFile () const
{
    return "portal_widget.h";
}

QString
PortalWidgetPlugin::group () const
{
    return tr ("CrystalView Widgets");
}

QIcon
PortalWidgetPlugin::icon () const
{
    // TODO: Add an icon to a resourece file for this
    return QIcon ();
//    return QIcon (":/images/portal_widget.png");
}

QString
PortalWidgetPlugin::toolTip () const
{
    return tr ("Volume slice viewing widget");
}

QString
PortalWidgetPlugin::whatsThis () const
{
    return tr ("This widget allows for slice by slice viewing of Plm_image objects");
}

bool
PortalWidgetPlugin::isContainer () const
{
    return false;
}

QWidget*
PortalWidgetPlugin::createWidget (QWidget* parent)
{
    return new PortalWidget (parent);
}

Q_EXPORT_PLUGIN2(portalwidgetplugin, PortalWidgetPlugin);
