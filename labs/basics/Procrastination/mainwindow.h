#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QListWidgetItem>
#include <QString>
#include <vector>
#include <ctime>

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

private slots:
    void on_spinBox_valueChanged(int arg1);

    void on_listWidget_itemClicked(QListWidgetItem *item);

    void on_listWidget_itemDoubleClicked(QListWidgetItem *item);

    void on_lineEdit_editingFinished();

    void on_comboBox_currentIndexChanged(int index);

    void on_pushButton_2_clicked();

    void on_pushButton_clicked();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
