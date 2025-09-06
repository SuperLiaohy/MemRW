#include "chartwidgetplugin.h"
#include "chartwidget.h"

#include <QtPlugin>

ChartWidgetPlugin::ChartWidgetPlugin(QObject *parent)
    : QObject(parent)
{}

void ChartWidgetPlugin::initialize(QDesignerFormEditorInterface * /* core */)
{
    if (m_initialized)
        return;

    // Add extension registrations, etc. here

    m_initialized = true;
}

bool ChartWidgetPlugin::isInitialized() const
{
    return m_initialized;
}

QWidget *ChartWidgetPlugin::createWidget(QWidget *parent)
{
    return new ChartWidget(parent);
}

QString ChartWidgetPlugin::name() const
{
    return QLatin1String("ChartWidget");
}

QString ChartWidgetPlugin::group() const
{
    return QLatin1String("");
}

QIcon ChartWidgetPlugin::icon() const
{
    return QIcon(QLatin1String(":/chart.svg"));
}

QString ChartWidgetPlugin::toolTip() const
{
    return QLatin1String("");
}

QString ChartWidgetPlugin::whatsThis() const
{
    return QLatin1String("");
}

bool ChartWidgetPlugin::isContainer() const
{
    return false;
}

QString ChartWidgetPlugin::domXml() const
{
    return QLatin1String(R"(<widget class="ChartWidget" name="chartWidget">
</widget>)");
}

QString ChartWidgetPlugin::includeFile() const
{
    return QLatin1String("ChartWidget/chartwidget.h");
}
