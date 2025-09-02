#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QAbstractItemModel>
#include <QMainWindow>

#include <memory>
#include <thread>
#include <unordered_map>
#include "RingBuffer.h"
#include "DAPReader.h"
class QTreeWidgetItem;
class QFrame;
class QLineSeries;
class QLabel;
class TreeModel;
class GroupItemAddDialog;

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
    void delete_chart(QFrame* frame);

    void create_group();
    void delete_group(QTreeWidgetItem* group);

    void add_item(GroupItemAddDialog* dlg);
    void remove_item(QTreeWidgetItem* item);
    TreeModel* model = nullptr;
    using RingBuffer = RingBuffer<8000,QPointF,QList<QPointF>>;
    struct group {
        QList<RingBuffer> ring_buffers;
        int used{};
    };
    std::unordered_map<QString, group> groups;
    struct chartTab {
        enum class State {
            Stop,
            Running,
            Closed,
        };
        State state;
        QTimer* timer;
        QString group;
        QList<qlonglong> addr;
        QList<QLineSeries *> series_list;
        std::chrono::high_resolution_clock::time_point start_time;
    };
    std::unordered_map<QString, chartTab> chartTabs;
    std::thread raad_thread;
    bool is_closing{false};
    uint32_t freq{0};
    QLabel* freqLabel;
    uint32_t last_time{0};
    std::unique_ptr<DAPReader> link;

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
