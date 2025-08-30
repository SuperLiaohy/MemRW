#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QAbstractItemModel>
#include <QMainWindow>

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

    void on_dwarf_treeView_doubleClicked(const QModelIndex &index);
    void customGroupMenuRequested(const QPoint& pos);
private:
    TreeModel* model = nullptr;

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
