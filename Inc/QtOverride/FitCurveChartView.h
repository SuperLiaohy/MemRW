#ifndef FITCURVECHARTVIEW_H
#define FITCURVECHARTVIEW_H
#include <QtCharts>

class FitCurveChartView : public QChartView {
    Q_OBJECT

public:
    FitCurveChartView(QWidget *parent = Q_NULLPTR);
    ~FitCurveChartView();

protected:
    void mousePressEvent(QMouseEvent *event);
    void mouseMoveEvent(QMouseEvent *event);
    void mouseReleaseEvent(QMouseEvent *event);
    void mouseDoubleClickEvent(QMouseEvent *event);
    void wheelEvent(QWheelEvent *event);

    signals:
        void signalMouseEvent(int eventId, QMouseEvent *event);
    void signalWheelEvent(QWheelEvent *event);

};

#endif // FITCURVECHARTVIEW_H