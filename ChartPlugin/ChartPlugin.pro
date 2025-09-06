TEMPLATE = subdirs

SUBDIRS += \
    ChartWidget \
#     ChartWidgetPlugin \
    ChartWidgetPlugin

ChartWidgetPlugin.depends = ChartWidget
