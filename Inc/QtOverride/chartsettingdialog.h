//
// Created by liaohy on 9/8/25.
//

#ifndef MEMRW_CHARTSETTINGDIALOG_H
#define MEMRW_CHARTSETTINGDIALOG_H

#include <QDialog>


QT_BEGIN_NAMESPACE
namespace Ui { class ChartSettingDialog; }
QT_END_NAMESPACE

class ChartSettingDialog : public QDialog {
Q_OBJECT

public:
    explicit ChartSettingDialog(QWidget *parent = nullptr);

    ~ChartSettingDialog() override;
public:
    double xMin();
    double xMax();
    double yMin();
    double yMax();
    int bufferSize();
private:
    Ui::ChartSettingDialog *ui;
};


#endif //MEMRW_CHARTSETTINGDIALOG_H
