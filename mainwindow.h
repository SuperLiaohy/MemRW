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

    void create_group();
    void delete_group(QTreeWidgetItem* group);

    void add_item(GroupItemAddDialog* dlg);
    void remove_item(QTreeWidgetItem* item);
    void writeCsv(const std::shared_ptr<QFile>& file, const QList<QStringList> &data);
    TreeModel* model = nullptr;
    using rb = RingBuffer<8000,QPointF,QList<QPointF>>;

    struct variable {
        uint64_t address{};
        GroupItemAddDialog::Type type{GroupItemAddDialog::Type::INT32};
        QColor color;
        rb ring_buffers;
    };
    struct group {
        // QList<rb> ring_buffers;
        int bound{};
        int used{};
        std::unordered_map<QString, variable> variables;
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
        QString logfile_path;
        std::shared_ptr<QFile> logfile;
        uint32_t freq;
        uint32_t last_time;
        QLabel* freqLabel;
        QCheckBox* logfileCheckBox;
        // QList<qlonglong> addr;
        QList<QLineSeries *> series_list;
        std::chrono::high_resolution_clock::time_point start_time;
        ~chartTab() {
            if (logfile!=nullptr) {
                logfile->close();
                logfile.reset();
            }
        }
    };
    std::unordered_map<QString, chartTab> chartTabs;
    std::unique_ptr<std::thread> link_thread;
    std::unique_ptr<DAPReader> link;
    bool is_closing{false};
    bool is_disconnect{true};

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
