#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QAbstractItemModel>
#include <QMainWindow>

#include <memory>
#include "RingBuffer.h"
#include "DAPReader.h"
class TreeModel;

QT_BEGIN_NAMESPACE
namespace Ui {
class MainWindow;
}
QT_END_NAMESPACE

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();
public slots:
    void on_openBtn_clicked();
    void on_reloadBtn_clicked();
    void on_actioncreate_group_triggered();
    void on_actiondelete_group_triggered();
    void on_actiondelete_item_triggered();
    void on_actiondisplay_group_dock_triggered();
    void on_actiondisplay_variable_dock_triggered();
    void on_actionadd_chart_tab_triggered();
    void on_actionadd_table_tab_triggered();
    void on_actionconnect_triggered();

    void on_dwarf_treeView_doubleClicked(const QModelIndex &index);
    void on_group_treeWidget_doubleClicked(const QModelIndex &index);
    void customGroupMenuRequested(const QPoint& pos);
private:
    TreeModel* model = nullptr;
public:
    QList<QList<RingBuffer<8000,QPointF,QList<QPointF>>>> rb;
    std::unique_ptr<DAPReader> link;

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
