#include "FitCurveChartView.h"

FitCurveChartView::FitCurveChartView(QWidget *parent) {

}

FitCurveChartView::~FitCurveChartView() {

}

void FitCurveChartView::mousePressEvent(QMouseEvent *event) {
    emit signalMouseEvent(0, event);
    QChartView::mousePressEvent(event);
}

void FitCurveChartView::mouseMoveEvent(QMouseEvent *event) {
    emit signalMouseEvent(1, event);
    QChartView::mouseMoveEvent(event);
}

void FitCurveChartView::mouseReleaseEvent(QMouseEvent *event) {
    emit signalMouseEvent(2, event);
    QChartView::mouseReleaseEvent(event);
}

void FitCurveChartView::mouseDoubleClickEvent(QMouseEvent *event) {
    emit signalMouseEvent(3, event);
    QChartView::mouseDoubleClickEvent(event);
}

void FitCurveChartView::wheelEvent(QWheelEvent *event) {
    emit signalWheelEvent(event);
    QChartView::wheelEvent(event);
}