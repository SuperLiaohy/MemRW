//
// Created by liaohy on 9/4/25.
//

#ifndef MEMRW_CHARTTABWIDGET_H
#define MEMRW_CHARTTABWIDGET_H

#include <QWidget>
#include "ChartWidget/chartwidget.h"

#include "groupitemadddialog.h"


QT_BEGIN_NAMESPACE
namespace Ui { class ChartTabWidget; }
QT_END_NAMESPACE

class ChartTabWidget : public QWidget {
Q_OBJECT

public:
    explicit ChartTabWidget(const std::shared_ptr<GroupTreeWidget::Group>& group, QWidget *parent = nullptr);

    ~ChartTabWidget() override;
public:
    ChartWidget* chartWidget();
    void UpdateFreq();
    void writeCsv(const QList<QStringList> &data);
    void startBtnEnable(bool able);

    float TimeStamp(std::chrono::high_resolution_clock::time_point time) {return std::chrono::duration_cast<std::chrono::microseconds>((time - start_time)).count() / 1000.f;}
    std::shared_ptr<GroupTreeWidget::Group>& GroupBound() {return group;}
    bool isRun() {return isRunning;}
    bool isLog() {return islog;}
    const QList<QLineSeries *>& seriesList() {return series_list;}

public slots:
    void on_logcfgBtn_clicked();
    void on_startBtn_clicked();
    void on_stopBtn_clicked();
    void on_setBtn_clicked();
    void timerUpdate();


private:
    Ui::ChartTabWidget *ui;
    bool islog = false;
    bool isRunning = false;
    QString file_name;
    std::shared_ptr<GroupTreeWidget::Group> group;
    QList<QLineSeries *> series_list;
    QScatterSeries* scatterSeries;
    QLineSeries* dashLine;
    QLabel* tips;


    QTimer* timer;
    std::chrono::high_resolution_clock::time_point start_time;
    std::shared_ptr<QFile> logfile;
    uint32_t last_time;
    int freq = 0;

    int findClosestPointIndex(const QVector<QPointF>& points, double x, double error);

};


#endif //MEMRW_CHARTTABWIDGET_H
