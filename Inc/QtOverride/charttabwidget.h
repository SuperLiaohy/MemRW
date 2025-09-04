//
// Created by liaohy on 9/4/25.
//

#ifndef MEMRW_CHARTTABWIDGET_H
#define MEMRW_CHARTTABWIDGET_H

#include <QWidget>


QT_BEGIN_NAMESPACE
namespace Ui { class ChartTabWidget; }
QT_END_NAMESPACE

class ChartTabWidget : public QWidget {
Q_OBJECT

public:
    explicit ChartTabWidget(QWidget *parent = nullptr);

    ~ChartTabWidget() override;

private:
    Ui::ChartTabWidget *ui;
};


#endif //MEMRW_CHARTTABWIDGET_H
