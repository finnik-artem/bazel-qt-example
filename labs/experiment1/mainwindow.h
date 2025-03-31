#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QCalendarWidget>
#include <QLabel>
#include <QSpinBox>
#include <QListWidget>
#include <QComboBox>
#include <QTextEdit>
#include <QString>
#include <QFile>
#include <QTextStream>
#include <QDebug>

#include <vector>

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

private:
    Ui::MainWindow *ui;
    QWidget *centralWidget;
    QVBoxLayout *layout;
    QLabel *start;
    QLabel *choose_subject;
    QLabel *choose_date;
    QLabel *choose_amount;
    QLabel *add_questions;
    QHBoxLayout *hlayout1;
    QVBoxLayout *vlayout1;
    QComboBox *subject;
    QSpinBox *amount;
    QCalendarWidget *date;
    QHBoxLayout *hlayout2;
    QListWidget *tickets;
    QTextEdit *questions;
    QPushButton *save;

    std::vector<QString> texts;
    bool step1 = false;
    bool step2 = false;
    bool step3 = false;
    bool step4 = false;

private slots:
    void subjectchanged();
    void datechanged();
    void amountchanged();
    void ticketsclicked();
    void questionchanged();
    void saveclicked();
};
#endif // MAINWINDOW_H
