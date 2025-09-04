//
// Created by liaohy on 9/4/25.
//

#ifndef MEMRW_DATALOGDIALOG_H
#define MEMRW_DATALOGDIALOG_H

#include <QDialog>


QT_BEGIN_NAMESPACE
namespace Ui { class DataLogDialog; }
QT_END_NAMESPACE

class DataLogDialog : public QDialog {
Q_OBJECT

public:
    explicit DataLogDialog(const QString& default_name, bool islog, QWidget *parent = nullptr);

    ~DataLogDialog() override;
public:
    QString fileName();
    bool islog();
private:
    Ui::DataLogDialog *ui;
};


#endif //MEMRW_DATALOGDIALOG_H
