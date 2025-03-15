#include "mainwindow.h"
#include "./ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent)
    , ui(new Ui::MainWindow)
{
    ui->setupUi(this);
    ui->lineEdit->setVisible(false);
    ui->comboBox->setVisible(false);
}

MainWindow::~MainWindow()
{
    delete ui;
}

std::vector<QString> names;
int previous_ticket = -1;

void MainWindow::on_spinBox_valueChanged(int arg1)
{
    previous_ticket = -1;
    ui->label->setText("");
    ui->label_2->setText("");
    ui->lineEdit->setVisible(false);
    ui->lineEdit->setText("");
    ui->comboBox->setVisible(false);
    ui->comboBox->setCurrentIndex(0);
    ui->progressBar->setValue(0);
    ui->progressBar->setMaximum(arg1);
    ui->progressBar_2->setValue(0);
    ui->progressBar_2->setMaximum(arg1);
    ui->listWidget->clear();
    names.resize(arg1);
    for (int i = 0; i < arg1; i++) {
        names[i] = "Билет " + QString::number(i + 1);
        ui->listWidget->addItem(names[i]);
        ui->listWidget->item(i)->setBackground(Qt::white);
    }
}


void MainWindow::on_listWidget_itemClicked(QListWidgetItem *item)
{
    ui->lineEdit->setVisible(true);
    ui->comboBox->setVisible(true);
    ui->label->setText(QString::number(ui->listWidget->currentRow() + 1));
    ui->label_2->setText(names[ui->listWidget->currentRow()]);
}


void MainWindow::on_listWidget_itemDoubleClicked(QListWidgetItem *item)
{
    if (item->background() == Qt::white) {
        item->setBackground(Qt::green);
        ui->progressBar->setValue(ui->progressBar->value() + 1);
        ui->progressBar_2->setValue(ui->progressBar_2->value() + 1);
    } else if (item->background() == Qt::yellow) {
        item->setBackground(Qt::green);
        ui->progressBar_2->setValue(ui->progressBar_2->value() + 1);
    } else {
        item->setBackground(Qt::yellow);
        ui->progressBar_2->setValue(ui->progressBar_2->value() - 1);
    }
}


void MainWindow::on_lineEdit_editingFinished()
{
    if (ui->lineEdit->text() != "") {
        names[ui->listWidget->currentRow()] = ui->lineEdit->text();
        ui->listWidget->currentItem()->setText(ui->lineEdit->text());
        ui->label_2->setText(ui->lineEdit->text());
    }
}


void MainWindow::on_comboBox_currentIndexChanged(int index)
{
    switch (index) {
    case 1:
        if (ui->listWidget->currentItem()->background() == Qt::yellow) {
            ui->progressBar->setValue(ui->progressBar->value() - 1);
        } else if (ui->listWidget->currentItem()->background() == Qt::green) {
            ui->progressBar->setValue(ui->progressBar->value() - 1);
            ui->progressBar_2->setValue(ui->progressBar_2->value() - 1);
        }
        ui->listWidget->currentItem()->setBackground(Qt::white);
        break;
    case 2:
        if (ui->listWidget->currentItem()->background() == Qt::white) {
            ui->progressBar->setValue(ui->progressBar->value() + 1);
        } else if (ui->listWidget->currentItem()->background() == Qt::green) {
            ui->progressBar_2->setValue(ui->progressBar_2->value() - 1);
        }
        ui->listWidget->currentItem()->setBackground(Qt::yellow);
        break;
    case 3:
        if (ui->listWidget->currentItem()->background() == Qt::white) {
            ui->progressBar->setValue(ui->progressBar->value() + 1);
            ui->progressBar_2->setValue(ui->progressBar_2->value() + 1);
        } else if (ui->listWidget->currentItem()->background() == Qt::yellow) {
            ui->progressBar_2->setValue(ui->progressBar_2->value() + 1);
        }
        ui->listWidget->currentItem()->setBackground(Qt::green);
        break;
    }
}


void MainWindow::on_pushButton_2_clicked()
{
    std::vector<int> notgreen;
    for (int i = 0; i < ui->listWidget->count(); i++) {
        if (ui->listWidget->item(i)->background() != Qt::green && i != ui->listWidget->currentRow()) {
            notgreen.push_back(i);
        }
    }
    if (notgreen.size() != 0) {
        previous_ticket = ui->listWidget->currentRow();
        ui->listWidget->setCurrentRow(notgreen[rand() % notgreen.size()]);
        ui->label->setText(QString::number(ui->listWidget->currentRow() + 1));
        ui->label_2->setText(names[ui->listWidget->currentRow()]);
    }
}


void MainWindow::on_pushButton_clicked()
{
    if (previous_ticket != -1) {
        ui->listWidget->setCurrentRow(previous_ticket);
        ui->label->setText(QString::number(ui->listWidget->currentRow() + 1));
        ui->label_2->setText(names[ui->listWidget->currentRow()]);
    }
}

