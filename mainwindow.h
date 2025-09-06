#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QAbstractItemModel>
#include <QMainWindow>

#include <memory>
#include <thread>
#include <QFile>
#include <unordered_map>
#include "groupitemadddialog.h"
#include "RingBuffer.h"
#include "DAPReader.h"

class QTreeWidgetItem;
class QFrame;
class QLineSeries;
class QLabel;
class QFile;
class QCheckBox;
class TreeModel;
class ChartTabWidget;
class TableTabWidget;
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
public:
private:
    void create_chart();
    void delete_chart(const QString& tabName);

    void create_table(const std::shared_ptr<GroupTreeWidget::Group>& group, const QString &tabName);
    void delete_table(const QString& tabName);

    void create_group();
    void delete_group(QTreeWidgetItem* group);

    void add_item(GroupItemAddDialog* dlg);
    void remove_item(QTreeWidgetItem* item);
    TreeModel* model = nullptr;

    std::unordered_map<QString, std::shared_ptr<GroupTreeWidget::Group>> groups;
    std::unordered_map<QString, ChartTabWidget*> chartTabs;
    std::unordered_map<QString, TableTabWidget*> tableTabs;
    std::unique_ptr<std::thread> link_thread;
    std::unique_ptr<DAPReader> link;
    bool is_closing{false};
    bool is_disconnect{true};

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
