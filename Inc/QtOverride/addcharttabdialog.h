//
// Created by liaohy on 8/31/25.
//

#ifndef MEMRW_ADDCHARTTABDIALOG_H
#define MEMRW_ADDCHARTTABDIALOG_H

#include <QDialog>

class QTreeWidget;
class QTreeWidgetItem;
QT_BEGIN_NAMESPACE

namespace Ui {
    class AddChartTabDialog;
}

QT_END_NAMESPACE

class AddChartTabDialog : public QDialog {
    Q_OBJECT

public:
    explicit AddChartTabDialog(QTreeWidget* group, QWidget *parent = nullptr);

    ~AddChartTabDialog() override;

public slots:
    void on_groupBox_currentIndexChanged(int index);

public:
    enum mode {
        Simple = 0,
        Full = 1,
        Advanced = 2,
    };

    QTreeWidgetItem* chartGroup();
    int chartMode();

private:
    QTreeWidget* tree;
    QTreeWidgetItem* group;


private:
    Ui::AddChartTabDialog *ui;
};


#endif //MEMRW_ADDCHARTTABDIALOG_H