#include <QDesignerCustomWidgetInterface>

class PortalWidgetPlugin : public QObject,
                           public QDesignerCustomWidgetInterface
{
    Q_OBJECT;
    Q_INTERFACES(QDesignerCustomWidgetInterface);

public:
    PortalWidgetPlugin(QObject *parent = 0);

    QString name() const;
    QString includeFile() const;
    QString group() const;
    QIcon icon() const;
    QString toolTip() const;
    QString whatsThis() const;
    bool isContainer() const;
    QWidget *createWidget(QWidget* parent);
};
