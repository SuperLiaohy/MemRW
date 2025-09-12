//
// Created by liaohy on 9/4/25.
//

// You may need to build the project (run Qt uic code generator) to get "ui_ChartTabWidget.h" resolved

#include "charttabwidget.h"
#include "ui_charttabwidget.h"

#include "datalogdialog.h"
#include "chartsettingdialog.h"
ChartTabWidget::ChartTabWidget(const std::shared_ptr<GroupTreeWidget::Group>& group, QWidget *parent) :
        QWidget(parent), ui(new Ui::ChartTabWidget), group(group) {
    ui->setupUi(this);

    scatterSeries = new QScatterSeries(this);
    dashLine = new QLineSeries(this);

    QPen pen(Qt::DashLine);
    pen.setColor(Qt::red);
    pen.setWidth(2);
    dashLine->setPen(pen);

    scatterSeries->setMarkerSize(7.0);
    ui->chartWidget->addSeriesLine(scatterSeries);
    ui->chartWidget->addSeriesLine(dashLine);

    ui->chartWidget->chart()->legend()->markers()[0]->setVisible(false);
    ui->chartWidget->chart()->legend()->markers()[1]->setVisible(false);

    QStringList show_mode;
    show_mode.append("only lines");
    show_mode.append("lines and points");
    show_mode.append("lines, points and tips");

    ui->showBox->addItems(show_mode);

    connect(ui->showBox, &QComboBox::currentIndexChanged, this, [this](int index){
        if (index == 0) {
            for (auto series: series_list) {
                series->setPointsVisible(false);
                series->setPointLabelsVisible(false);
            }
        } else if (index==1) {
            for (auto series: series_list) {
                series->setPointsVisible(true);
                series->setPointLabelsVisible(false);
            }
        } else if (index==2) {
            for (auto series: series_list) {
                series->setPointsVisible(true);
                series->setPointLabelsFormat("(@xPoint, @yPoint)");
                series->setPointLabelsVisible(true);
            }
        }
    });

    // create the series to paint the chart and load ringbuffer data
    series_list.resize(group->variables.size());

    int index = 0;
    for (auto & variable: group->variables) {
        series_list[index] = new QLineSeries(ui->chartWidget->chart());
        series_list[index]->setName(variable.first);
        series_list[index]->setProperty("color", variable.second.color);
        ui->chartWidget->addSeriesLine(series_list[index]);
        ++index;
    }
    ++group->bound;

    // create the timer to update QChartView
    timer = new QTimer(this);
    timer->stop();
    connect(timer, &QTimer::timeout, this, &ChartTabWidget::timerUpdate);

    auto tipTimer = new QTimer(this);
    connect(tipTimer, &QTimer::timeout, this, [this](){

        const QPoint curPos = QCursor::pos();

        QPoint localPos = ui->chartWidget->mapFromGlobal(curPos);

        if (!ui->chartWidget->rect().contains(localPos)){return ;}
        QPointF curVal = ui->chartWidget->chart()->mapToValue(QPointF(localPos));

        auto x = (curVal.x());
        auto error = ui->chartWidget->xRange()*0.005;
        QString text;
        QList<QPointF> listTip;
        for (auto & series : series_list) {
            int index = findClosestPointIndex(series->points(),x,error);
            if (index<0) {
                text.push_back(QString("%1:(%2)\n").arg(series->name()).arg("N/A"));
                return;
            }
            const auto& point = series->points()[index];
            listTip<<point;
            text.push_back(QString("%1:(%2,%3)\n").arg(series->name()).arg(point.x()).arg(point.y()));
        }
        QToolTip::showText(QCursor::pos(), text);
        scatterSeries->replace(listTip);
        QList<QPointF> list{QPointF(x,static_cast<QValueAxis*>(ui->chartWidget->chart()->axisY())->min()),
                            QPointF(x,static_cast<QValueAxis*>(ui->chartWidget->chart()->axisY())->max())};

        dashLine->replace(list);

    });
    tipTimer->start(8);

}

ChartTabWidget::~ChartTabWidget() {
    --group->bound;
    if (logfile != nullptr) {
        logfile->close();
        logfile.reset();
    }
    delete ui;
}

ChartWidget *ChartTabWidget::chartWidget() {
    return ui->chartWidget;
}

void ChartTabWidget::UpdateFreq() {
    auto time = duration_cast<std::chrono::microseconds>((std::chrono::high_resolution_clock::now() - start_time)).count()/1000.f;
    if ((time - last_time)>1000) {
        ui->freqLabel->setText(QString("freq: %1").arg(freq));
        last_time = time;
        freq = 0;
    }
    ++freq;
}

void ChartTabWidget::on_logcfgBtn_clicked() {
    DataLogDialog *dlg = new DataLogDialog(file_name, islog, this);
    if (dlg->exec()==QDialog::Accepted) {
        file_name = dlg->fileName();
        islog = dlg->islog();
    }
}

void ChartTabWidget::on_startBtn_clicked() {
    if (group->variables.empty()) {
        QMessageBox::critical(this, "MESSAGE", "you can not start because the group you bound is empty!",QMessageBox::Ok);
        return;
    }

    if (islog) {
        if (logfile!=nullptr) {
            logfile->close();
            logfile.reset();
        }
        if (file_name.isEmpty()) {
            QMessageBox::critical(this,"MESSAGE","can not open logfile!",QMessageBox::Ok);
            return;
        }
        logfile = std::make_shared<QFile>(file_name);
        if (!logfile->open(QIODevice::WriteOnly | QIODevice::Text)) {
            QMessageBox::critical(this,"MESSAGE","can not open logfile!",QMessageBox::Ok);
            logfile.reset();
            return;
        }
    }


    for (auto series: series_list) {ui->chartWidget->chart()->removeSeries(series);}

    series_list.resize(group->variables.size());

    QStringList csv_header;
    csv_header << "TimeStamp";

    int index = 0;
    for (auto & variable: group->variables) {
        variable.second.ring_buffers.reset();
        if (islog) csv_header << variable.first;
        series_list[index] = new QLineSeries(ui->chartWidget->chart());
        series_list[index]->setName(variable.first);
        series_list[index]->setProperty("color", variable.second.color);
        ui->chartWidget->addSeriesLine(series_list[index]);
        ++index;
    }
    if (islog) writeCsv({csv_header});
    ui->showBox->setEnabled(false);
    ui->setBtn->setEnabled(false);
    ui->logcfgBtn->setEnabled(false);
    ++group->used;

    last_time = 0;
    freq = 0;

    ui->chartWidget->loadDefaultRange();
    ui->showBox->setCurrentIndex(0);

    start_time = std::chrono::high_resolution_clock::now();
    timer->start(30);
    isRunning= true;


}

void ChartTabWidget::on_stopBtn_clicked() {
    isRunning= false;
    ui->logcfgBtn->setEnabled(true);
    ui->showBox->setEnabled(true);
    ui->setBtn->setEnabled(true);
    if (logfile != nullptr) {
        logfile->close();
        logfile.reset();
    }
    timer->stop();
    --group->used;
}

void ChartTabWidget::on_setBtn_clicked() {
    auto xMin = ui->chartWidget->x_min();
    auto xMax = ui->chartWidget->x_max();
    auto yMin = ui->chartWidget->y_min();
    auto yMax = ui->chartWidget->y_max();

    ChartSettingDialog *dlg = new ChartSettingDialog(xMin,xMax,yMin,yMax,group->variables[0].ring_buffers.capacity(),
                                                     this);
    if (dlg->exec()==QDialog::Accepted) {
        ui->chartWidget->changeDefaultX(dlg->xMin(),dlg->xMax());
        ui->chartWidget->changeDefaultY(dlg->yMin(),dlg->yMax());
        for (auto& variable : group->variables) {
            variable.second.ring_buffers.change_capacity(dlg->bufferSize());

        }
    }
}


void ChartTabWidget::timerUpdate() {
    for (auto series : series_list) {
        auto &ringbuffer = group->variables.at(series->name()).ring_buffers;
//        if (ringbuffer.is_full())
//            series->replace(ringbuffer.get_container());
//        else
            series->replace(ringbuffer.get_valid_container());
    }
    auto time = std::chrono::duration_cast<std::chrono::microseconds>(
            (std::chrono::high_resolution_clock::now() - start_time)).count() / 1000.f;
    // 自动滚动视图
    if (time > ui->chartWidget->xRange()) {
        ui->chartWidget->chart()->axisX()->setRange(time - ui->chartWidget->xRange(), time);
    }


}

void ChartTabWidget::writeCsv(const QList<QStringList> &data) {
    QTextStream out(logfile.get());
    for (const QStringList &row: data) {
        out << row.join(',') << "\n";
    }
}

void ChartTabWidget::startBtnEnable(bool able) {
    ui->startBtn->setEnabled(able);
}

// 假设points已按x坐标升序排序
int ChartTabWidget::findClosestPointIndex(const QVector<QPointF>& points, double x, double error) {
    // 特殊情况处理
    if (points.isEmpty())
        return -1;

    int left = 0;
    int right = points.size() - 1;

    // 如果x小于最小值或大于最大值，直接返回边界点
    if (x <= points[left].x())
        return left;
    if (x >= points[right].x())
        return right;

    // 二分查找
    while (left <= right) {
        int mid = left + (right - left) / 2;

        // 如果找到精确匹配（在误差范围内）
        if (fabs(points[mid].x() - x) < error)
            return mid;

        // 决定搜索哪半部分
        if (points[mid].x() < x)
            left = mid + 1;
        else
            right = mid - 1;
    }

    // 此时left > right，需要比较哪个点更接近x
    // left可能等于points.size()，需要检查边界
    if (left >= points.size())
        return right;
    if (right < 0)
        return left;

    // 返回更接近x的点的索引
    return (fabs(points[left].x() - x) < fabs(points[right].x() - x)) ? left : right;
}