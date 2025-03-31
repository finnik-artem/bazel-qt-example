#include "mainwindow.h"
#include "ui_mainwindow.h"

MainWindow::MainWindow(QWidget *parent) :
    QMainWindow(parent),
    ui(new Ui::MainWindow)
{
    ui->setupUi(this);

    centralWidget = new QWidget(this);
    setCentralWidget(centralWidget);

    layout = new QVBoxLayout(centralWidget);
    layout->setAlignment(Qt::AlignLeft);

    start = new QLabel("Создай свой коллоквиум:", this);
    layout->addWidget(start);
    choose_subject = new QLabel("-выберите дисциплину", this);
    choose_subject->setStyleSheet("color: red");
    choose_date = new QLabel("-выберите дату", this);
    choose_date->setStyleSheet("color: red");
    choose_amount = new QLabel("-выберите количество билетов", this);
    choose_amount->setStyleSheet("color: red");
    add_questions = new QLabel("-добавьте содеражание билетов", this);
    add_questions->setStyleSheet("color: red");
    layout->addWidget(choose_subject);
    layout->addWidget(choose_date);
    layout->addWidget(choose_amount);
    layout->addWidget(add_questions);

    hlayout1 = new QHBoxLayout(this);
    vlayout1 = new QVBoxLayout(this);
    hlayout1->addLayout(vlayout1);
    layout->addLayout(hlayout1);


    subject = new QComboBox(this);
    subject->addItem("Выберите дисциплину");
    subject->addItem("Линейная алгебра");
    subject->addItem("Математический анализ");
    subject->addItem("Дискретная математика и математическая логика");
    amount = new QSpinBox(this);
    amount->setMinimum(0);
    amount->setMaximum(100);
    vlayout1->addWidget(subject);
    vlayout1->addWidget(amount);
    date = new QCalendarWidget(this);
    date->setMinimumDate(date->selectedDate());
    hlayout1->addWidget(date);

    hlayout2 = new QHBoxLayout(this);
    layout->addLayout(hlayout2);

    tickets = new QListWidget(this);
    questions = new QTextEdit(this);
    hlayout2->addWidget(tickets);
    hlayout2->addWidget(questions);

    save = new QPushButton("Сохранить в файл", this);
    layout->addWidget(save);

    connect(subject, &QComboBox::currentIndexChanged, this, &MainWindow::subjectchanged);
    connect(date, &QCalendarWidget::selectionChanged, this, &MainWindow::datechanged);
    connect(amount, &QSpinBox::valueChanged, this, &MainWindow::amountchanged);
    connect(tickets, &QListWidget::currentItemChanged, this, &MainWindow::ticketsclicked);
    connect(questions, &QTextEdit::textChanged, this, &MainWindow::questionchanged);
    connect(save, &QPushButton::clicked, this, &MainWindow::saveclicked);
}

MainWindow::~MainWindow()
{
    delete ui;
}

void MainWindow::subjectchanged() {
    if (subject->currentIndex() != 0) {
        choose_subject->setStyleSheet("color: green");
        step1 = true;
    } else {
        choose_subject->setStyleSheet("color: red");
        step1 = false;
    }
}

void MainWindow::datechanged() {
    choose_date->setStyleSheet("color: green");
    step2 = true;
}

void MainWindow::amountchanged() {
    if (amount->value() != 0) {
        choose_amount->setStyleSheet("color: green");
        step3 = true;
    } else {
        choose_amount->setStyleSheet("color: red");
        step3 = false;
    }
    texts.resize(amount->value());
    tickets->clear();
    for (int i = 0; i < amount->value(); i++) {
        texts[i] = "";
        tickets->addItem("Билет " + QString::number(i + 1));
    }
}

void MainWindow::ticketsclicked() {
    questions->setText(texts[tickets->currentRow()]);
}

void MainWindow::questionchanged() {
    texts[tickets->currentRow()] = questions->toPlainText();
    bool flag = true;
    for (int i = 0; i < amount->value(); i++) {
        if (texts[i] == "") {
            flag = false;
            break;
        }
    }
    if (flag) {
        add_questions->setStyleSheet("color: green");
        step4 = true;
    } else {
        add_questions->setStyleSheet("color: red");
        step4 = false;
    }
}

void MainWindow::saveclicked() {
    if (step1 && step2 && step3 && step4) {
        QFile file("colloc.txt");
        if(!file.open(QIODevice::WriteOnly | QIODevice::Text)) {
            return;
        }
        QTextStream fout(&file);
        fout << "Билеты к коллоквиуму " << date->selectedDate().toString("dd.MM.yyyy") << " по дисциплине " << subject->currentText() << "\n\n";
        for (int i = 0; i < amount->value(); i++) {
            fout << "Билет " << QString::number(i + 1) << "\n" << texts[i] << "\n\n";
        }
        file.close();
    }
}

