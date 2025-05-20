#include "mainwindow.h"
#include <QApplication>
#include <QDebug>
#include <QDir>
#include <QStandardPaths>
#include <algorithm>

MainWindow::MainWindow(QWidget *parent)
    : QMainWindow(parent),
      stackedWidget(nullptr), welcomePage(nullptr), translationExercisePage(nullptr),
      grammarExercisePage(nullptr), welcomeLabel(nullptr), translationQuestionLabel(nullptr),
      translationAnswerEdit(nullptr), submitTranslationButton(nullptr),
      grammarQuestionLabel(nullptr), grammarOptionGroup(nullptr),
      submitGrammarButton(nullptr), progressBar(nullptr), scoreLabel(nullptr),
      timerLabel(nullptr), difficultyLabel(nullptr), fileMenu(nullptr),
      difficultyMenu(nullptr), setDifficultyAction(nullptr), exitAction(nullptr),
      startTranslationBtn(nullptr), startGrammarBtn(nullptr),
      currentDifficulty("Elementary"), currentExerciseType(ExerciseType::None),
      currentQuestionIndex(0), score(0), incorrectAttempts(0),
      exerciseTimer(new QTimer(this)), timeLeftSeconds(0),
      difficultyDialog(nullptr)
{
    for(int i=0; i<4; ++i) grammarOptions[i] = nullptr;

    QString dataDir = QStandardPaths::writableLocation(QStandardPaths::AppDataLocation);
    QDir dir(dataDir);
    if (!dir.exists()) {
        dir.mkpath(".");
    }
    dbPath = dataDir + "/english_tutor.sqlite";
    qDebug() << "Database will be at:" << dbPath;


    if (!initDatabase()) {
        QMessageBox::critical(this, tr("Database Error"), tr("Failed to initialize the application database. The application may not function correctly."));
    }

    setupUI();
    setupMenu();
    resetExerciseState();

    connect(exerciseTimer, &QTimer::timeout, this, &MainWindow::updateTimer);

    stackedWidget->setCurrentWidget(welcomePage);
    setWindowTitle(tr("English Language Tutor"));
    setMinimumSize(700, 500);
}

MainWindow::~MainWindow()
{
    if (appDb.isOpen()) {
        appDb.close();
    }
}

bool MainWindow::initDatabase() {
    appDb = QSqlDatabase::addDatabase("QSQLITE", "english_tutor_connection");
    appDb.setDatabaseName(dbPath);

    if (!appDb.open()) {
        qCritical() << "Failed to open database:" << appDb.lastError().text();
        return false;
    }
    qInfo() << "Database opened successfully at" << dbPath;

    createTablesIfNotExist();
    if (!importInitialDataIfNeeded()) {
         qWarning() << "Initial data import might have encountered issues or was not needed.";
    }
    return true;
}

void MainWindow::createTablesIfNotExist() {
    QSqlQuery query(appDb);

    bool success = query.exec("CREATE TABLE IF NOT EXISTS difficulties ("
                              "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                              "name TEXT UNIQUE NOT NULL)");
    if (!success) qCritical() << "Failed to create difficulties table:" << query.lastError().text();

    success = query.exec("CREATE TABLE IF NOT EXISTS translation_questions ("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                         "difficulty_id INTEGER NOT NULL, "
                         "question_text TEXT NOT NULL, "
                         "answer_text TEXT NOT NULL, "
                         "hint_text TEXT, "
                         "FOREIGN KEY(difficulty_id) REFERENCES difficulties(id))");
    if (!success) qCritical() << "Failed to create translation_questions table:" << query.lastError().text();

    success = query.exec("CREATE TABLE IF NOT EXISTS grammar_questions ("
                         "id INTEGER PRIMARY KEY AUTOINCREMENT, "
                         "difficulty_id INTEGER NOT NULL, "
                         "question_text TEXT NOT NULL, "
                         "option1 TEXT NOT NULL, "
                         "option2 TEXT NOT NULL, "
                         "option3 TEXT NOT NULL, "
                         "option4 TEXT NOT NULL, "
                         "correct_option_index INTEGER NOT NULL, "
                         "hint_text TEXT, "
                         "FOREIGN KEY(difficulty_id) REFERENCES difficulties(id))");
    if (!success) qCritical() << "Failed to create grammar_questions table:" << query.lastError().text();
}

bool MainWindow::isDatabaseEmpty() {
    QSqlQuery query("SELECT COUNT(*) FROM difficulties", appDb);
    if (query.exec() && query.next()) {
        return query.value(0).toInt() == 0;
    }
    qWarning() << "Could not determine if database is empty:" << query.lastError().text();
    return true;
}

bool MainWindow::importDifficulties() {
    QSqlQuery query(appDb);
    QStringList difficulties = {"Elementary", "Intermediate", "Advanced"};
    bool all_ok = true;
    for (const QString& diffName : difficulties) {
        query.prepare("INSERT OR IGNORE INTO difficulties (name) VALUES (:name)");
        query.bindValue(":name", diffName);
        if (!query.exec()) {
            qCritical() << "Failed to insert difficulty" << diffName << ":" << query.lastError().text();
            all_ok = false;
        }
    }
    if (all_ok) qDebug() << "Difficulties populated/verified.";
    return all_ok;
}


bool MainWindow::importInitialDataIfNeeded() {
    if (!isDatabaseEmpty()) {
        qInfo() << "Database already contains data. Skipping import.";
        return true;
    }
    qInfo() << "Database is empty. Starting initial data import from text files...";

    if (!importDifficulties()) {
        qCritical() << "Failed to import difficulties. Aborting further data import.";
        return false;
    }

    QStringList diffNames = {"Elementary", "Intermediate", "Advanced"};
    bool overall_success = true;

    QDir appDir(QCoreApplication::applicationDirPath());
    QString questionsDirRoot = QCoreApplication::applicationDirPath();
    #ifdef Q_OS_MAC
    #endif
    QString questionsPathPrefix;
    if (QDir(questionsDirRoot + "/questions").exists()) {
        questionsPathPrefix = questionsDirRoot + "/questions/";
    } else if (QDir(QDir::cleanPath(questionsDirRoot + "/../questions")).exists()) {
        questionsPathPrefix = QDir::cleanPath(questionsDirRoot + "/../questions/");
    } else if (QDir(QDir::cleanPath(questionsDirRoot + "/../../english_tutor/questions")).exists()){
         questionsPathPrefix = QDir::cleanPath(questionsDirRoot + "/../../english_tutor/questions/");
    } else {
        qWarning() << "Question directory not found near " << questionsDirRoot << ". Trying local 'questions/'";
        questionsPathPrefix = "./questions/";
    }
     qDebug() << "Using questions directory for import: " << questionsPathPrefix;


    for (const QString& diffName : diffNames) {
        QSqlQuery idQuery(appDb);
        idQuery.prepare("SELECT id FROM difficulties WHERE name = :name");
        idQuery.bindValue(":name", diffName);
        if (!idQuery.exec() || !idQuery.next()) {
            qCritical() << "Could not find ID for difficulty" << diffName << idQuery.lastError().text();
            overall_success = false;
            continue;
        }
        int difficultyId = idQuery.value(0).toInt();

        QString transFile = questionsPathPrefix + QString("translate_%1.txt").arg(diffName.toLower());
        if (!importQuestionsFromFile(transFile, ExerciseType::Translation, difficultyId)) {
            qWarning() << "Failed to import some/all translation questions for" << diffName;
            overall_success = false;
        }

        QString grammarFile = questionsPathPrefix + QString("grammar_%1.txt").arg(diffName.toLower());
        if (!importQuestionsFromFile(grammarFile, ExerciseType::Grammar, difficultyId)) {
            qWarning() << "Failed to import some/all grammar questions for" << diffName;
            overall_success = false;
        }
    }

    if (overall_success) qInfo() << "Initial data import completed.";
    else qWarning() << "Initial data import finished with some errors.";
    return overall_success;
}

bool MainWindow::importQuestionsFromFile(const QString& filePath, ExerciseType type, int difficultyId) {
    QFile file(filePath);
    if (!file.exists()){
        qWarning() << "Question file does not exist, cannot import:" << filePath;
        return false;
    }
    if (!file.open(QIODevice::ReadOnly | QIODevice::Text)) {
        qWarning() << "Could not open question file for import:" << filePath << file.errorString();
        return false;
    }

    QTextStream in(&file);
    QSqlQuery query(appDb);
    int count = 0;
    bool all_lines_ok = true;

    appDb.transaction();

    while (!in.atEnd()) {
        QString line = in.readLine().trimmed();
        if (line.isEmpty()) continue;

        QStringList parts = line.split(',');
        if (type == ExerciseType::Translation && parts.size() == 3) {
            query.prepare("INSERT INTO translation_questions (difficulty_id, question_text, answer_text, hint_text) "
                          "VALUES (:did, :q, :a, :h)");
            query.bindValue(":did", difficultyId);
            query.bindValue(":q", parts[0].trimmed());
            query.bindValue(":a", parts[1].trimmed());
            query.bindValue(":h", parts[2].trimmed());
        } else if (type == ExerciseType::Grammar && parts.size() == 7) {
            bool ok;
            int answerIdx = parts[5].trimmed().toInt(&ok);
            if (!ok || answerIdx < 0 || answerIdx > 3) {
                qWarning() << "Skipping malformed grammar line (answer index):" << line << "in file" << filePath;
                all_lines_ok = false;
                continue;
            }
            query.prepare("INSERT INTO grammar_questions (difficulty_id, question_text, option1, option2, option3, option4, correct_option_index, hint_text) "
                          "VALUES (:did, :q, :o1, :o2, :o3, :o4, :ans, :h)");
            query.bindValue(":did", difficultyId);
            query.bindValue(":q", parts[0].trimmed());
            query.bindValue(":o1", parts[1].trimmed());
            query.bindValue(":o2", parts[2].trimmed());
            query.bindValue(":o3", parts[3].trimmed());
            query.bindValue(":o4", parts[4].trimmed());
            query.bindValue(":ans", answerIdx);
            query.bindValue(":h", parts[6].trimmed());
        } else {
            qWarning() << "Skipping malformed line in file" << filePath << ":" << line;
            all_lines_ok = false;
            continue;
        }

        if (!query.exec()) {
            qWarning() << "Failed to insert question from" << filePath << ":" << line << "Error:" << query.lastError().text();
            all_lines_ok = false;
        } else {
            count++;
        }
    }
    file.close();

    if (!appDb.commit()) {
        qCritical() << "Database transaction commit failed after importing" << filePath << appDb.lastError().text();
        appDb.rollback();
        return false;
    }

    qInfo() << "Imported" << count << "questions from" << filePath;
    return all_lines_ok && count > 0;
}


void MainWindow::setupUI()
{
    QWidget *centralWidget = new QWidget(this);
    QVBoxLayout *mainLayout = new QVBoxLayout(centralWidget);

    QHBoxLayout *topBarLayout = new QHBoxLayout();
    difficultyLabel = new QLabel(tr("Difficulty: %1").arg(currentDifficulty));
    startTranslationBtn = new QPushButton(tr("Start Translation Exercise"));
    startGrammarBtn = new QPushButton(tr("Start Grammar Exercise"));
    topBarLayout->addWidget(difficultyLabel);
    topBarLayout->addStretch();
    topBarLayout->addWidget(startTranslationBtn);
    topBarLayout->addWidget(startGrammarBtn);
    mainLayout->addLayout(topBarLayout);

    connect(startTranslationBtn, &QPushButton::clicked, this, &MainWindow::startTranslationExercise);
    connect(startGrammarBtn, &QPushButton::clicked, this, &MainWindow::startGrammarExercise);

    stackedWidget = new QStackedWidget(this);
    createWelcomePage();
    createTranslationPage();
    createGrammarPage();
    stackedWidget->addWidget(welcomePage);
    stackedWidget->addWidget(translationExercisePage);
    stackedWidget->addWidget(grammarExercisePage);
    mainLayout->addWidget(stackedWidget, 1);

    QHBoxLayout *statusLayout = new QHBoxLayout();
    scoreLabel = new QLabel(tr("Score: 0"));
    progressBar = new QProgressBar();
    progressBar->setRange(0, TOTAL_QUESTIONS_PER_SESSION);
    progressBar->setValue(0);
    progressBar->setTextVisible(true);
    timerLabel = new QLabel(tr("Time: 00:00"));

    statusLayout->addWidget(scoreLabel);
    statusLayout->addStretch();
    statusLayout->addWidget(new QLabel(tr("Progress:")));
    statusLayout->addWidget(progressBar,1);
    statusLayout->addStretch();
    statusLayout->addWidget(timerLabel);
    mainLayout->addLayout(statusLayout);

    setCentralWidget(centralWidget);
}

void MainWindow::setupMenu()
{
    fileMenu = menuBar()->addMenu(tr("&File"));
    difficultyMenu = menuBar()->addMenu(tr("&Options"));

    setDifficultyAction = new QAction(tr("&Set Difficulty..."), this);
    connect(setDifficultyAction, &QAction::triggered, this, &MainWindow::selectDifficulty);
    difficultyMenu->addAction(setDifficultyAction);

    exitAction = new QAction(tr("E&xit"), this);
    connect(exitAction, &QAction::triggered, qApp, &QApplication::quit);
    fileMenu->addAction(exitAction);
}

void MainWindow::createWelcomePage()
{
    welcomePage = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(welcomePage);
    layout->setAlignment(Qt::AlignCenter);
    welcomeLabel = new QLabel(tr("Welcome to the English Language Tutor!\n\n"
                                  "Select an exercise type above to begin.\n"
                                  "Change difficulty from the 'Options' menu.\n"
                                  "Press 'H' during an exercise for a hint."), this);
    welcomeLabel->setAlignment(Qt::AlignCenter);
    welcomeLabel->setWordWrap(true);
    QFont font = welcomeLabel->font();
    font.setPointSize(14);
    welcomeLabel->setFont(font);
    layout->addWidget(welcomeLabel);
}

void MainWindow::createTranslationPage()
{
    translationExercisePage = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(translationExercisePage);
    translationQuestionLabel = new QLabel(tr("Translate this sentence:"));
    translationQuestionLabel->setWordWrap(true);
    translationAnswerEdit = new QLineEdit();
    submitTranslationButton = new QPushButton(tr("Submit Answer"));

    layout->addWidget(translationQuestionLabel);
    layout->addWidget(translationAnswerEdit);
    layout->addWidget(submitTranslationButton, 0, Qt::AlignRight);
    layout->addStretch();

    connect(submitTranslationButton, &QPushButton::clicked, this, &MainWindow::submitTranslationAnswer);
    connect(translationAnswerEdit, &QLineEdit::returnPressed, this, &MainWindow::submitTranslationAnswer);
}

void MainWindow::createGrammarPage()
{
    grammarExercisePage = new QWidget();
    QVBoxLayout *layout = new QVBoxLayout(grammarExercisePage);
    grammarQuestionLabel = new QLabel(tr("Complete this sentence:"));
    grammarQuestionLabel->setWordWrap(true);
    layout->addWidget(grammarQuestionLabel);

    grammarOptionGroup = new QButtonGroup(this);
    QVBoxLayout *optionsLayout = new QVBoxLayout();
    for (int i = 0; i < 4; ++i) {
        grammarOptions[i] = new QRadioButton(tr("Option %1").arg(i + 1));
        optionsLayout->addWidget(grammarOptions[i]);
        grammarOptionGroup->addButton(grammarOptions[i], i);
    }
    layout->addLayout(optionsLayout);

    submitGrammarButton = new QPushButton(tr("Submit Answer"));
    layout->addWidget(submitGrammarButton, 0, Qt::AlignRight);
    layout->addStretch();

    connect(submitGrammarButton, &QPushButton::clicked, this, &MainWindow::submitGrammarAnswer);
}

void MainWindow::selectDifficulty()
{
    if (difficultyDialog) {
        difficultyDialog->deleteLater();
    }
    difficultyDialog = new DifficultyDialog(currentDifficulty, this);
    if (difficultyDialog->exec() == QDialog::Accepted) {
        onDifficultySelected(difficultyDialog->selectedDifficulty());
    }
    difficultyDialog->deleteLater();
    difficultyDialog = nullptr;
}

void MainWindow::onDifficultySelected(const QString& difficulty)
{
    if (currentExerciseType != ExerciseType::None) {
        QMessageBox::warning(this, tr("Exercise in Progress"), tr("Cannot change difficulty during an exercise. Please finish or reset."));
        return;
    }
    currentDifficulty = difficulty;
    difficultyLabel->setText(tr("Difficulty: %1").arg(currentDifficulty));
    qDebug() << "Difficulty set to:" << currentDifficulty;
}

void MainWindow::loadQuestionsFromDb(ExerciseType type, const QString& difficulty)
{
    currentTranslationSet.clear();
    currentGrammarSet.clear();

    if (!appDb.isOpen()) {
        QMessageBox::critical(this, tr("Database Error"), tr("Database is not open. Cannot load questions."));
        return;
    }

    QSqlQuery query(appDb);
    query.prepare("SELECT id FROM difficulties WHERE name = :name");
    query.bindValue(":name", difficulty);
    if (!query.exec() || !query.next()) {
        QMessageBox::critical(this, tr("Database Error"), tr("Could not find difficulty '%1' in database.").arg(difficulty));
        return;
    }
    int difficultyId = query.value(0).toInt();

    QString queryString;
    if (type == ExerciseType::Translation) {
        queryString = "SELECT question_text, answer_text, hint_text FROM translation_questions "
                      "WHERE difficulty_id = :did ORDER BY RANDOM() LIMIT :limit";
    } else if (type == ExerciseType::Grammar) {
        queryString = "SELECT question_text, option1, option2, option3, option4, correct_option_index, hint_text "
                      "FROM grammar_questions WHERE difficulty_id = :did ORDER BY RANDOM() LIMIT :limit";
    } else {
        return;
    }

    query.prepare(queryString);
    query.bindValue(":did", difficultyId);
    query.bindValue(":limit", TOTAL_QUESTIONS_PER_SESSION);

    if (!query.exec()) {
        QMessageBox::critical(this, tr("Database Error"), tr("Failed to load questions: %1").arg(query.lastError().text()));
        return;
    }

    while (query.next()) {
        if (type == ExerciseType::Translation) {
            currentTranslationSet.append({
                query.value("question_text").toString(),
                query.value("answer_text").toString(),
                query.value("hint_text").toString()
            });
        } else if (type == ExerciseType::Grammar) {
            currentGrammarSet.append({
                query.value("question_text").toString(),
                {query.value("option1").toString(), query.value("option2").toString(),
                 query.value("option3").toString(), query.value("option4").toString()},
                query.value("correct_option_index").toInt(),
                query.value("hint_text").toString()
            });
        }
    }

    int questionsLoaded = (type == ExerciseType::Translation) ? currentTranslationSet.size() : currentGrammarSet.size();
    qDebug() << "Loaded" << questionsLoaded << "questions from DB for" << difficulty << (type == ExerciseType::Translation ? "Translation" : "Grammar");

    if (questionsLoaded == 0) {
         QMessageBox::warning(this, tr("No Questions"),
                              tr("No questions found in the database for %1 %2 difficulty.\n"
                                 "Ensure data was imported correctly or add questions to the source files and restart if DB was empty.")
                              .arg(difficulty)
                              .arg(type == ExerciseType::Translation ? "translation" : "grammar"));
    } else if (questionsLoaded < TOTAL_QUESTIONS_PER_SESSION) {
         QMessageBox::information(this, tr("Fewer Questions"),
                                 tr("Loaded %1 questions, which is less than the desired %2 for a full session.")
                                 .arg(questionsLoaded).arg(TOTAL_QUESTIONS_PER_SESSION));
         progressBar->setMaximum(questionsLoaded);
    } else {
        progressBar->setMaximum(TOTAL_QUESTIONS_PER_SESSION);
    }
}


void MainWindow::startTranslationExercise()
{
    if (currentExerciseType != ExerciseType::None) {
        QMessageBox::information(this, tr("Exercise Ongoing"), tr("An exercise is already in progress."));
        return;
    }
    loadQuestionsFromDb(ExerciseType::Translation, currentDifficulty);
    if (currentTranslationSet.isEmpty()) {
        resetExerciseState();
        return;
    }
    startExercise(ExerciseType::Translation);
}

void MainWindow::startGrammarExercise()
{
    if (currentExerciseType != ExerciseType::None) {
        QMessageBox::information(this, tr("Exercise Ongoing"), tr("An exercise is already in progress."));
        return;
    }
    loadQuestionsFromDb(ExerciseType::Grammar, currentDifficulty);
    if (currentGrammarSet.isEmpty()) {
        resetExerciseState();
        return;
    }
    startExercise(ExerciseType::Grammar);
}


void MainWindow::startExercise(ExerciseType type)
{
    currentExerciseType = type;
    currentQuestionIndex = 0;
    incorrectAttempts = 0;
    timeLeftSeconds = EXERCISE_DURATION_SECONDS;

    int numQuestionsInSet = (type == ExerciseType::Translation) ? currentTranslationSet.size() : currentGrammarSet.size();
    progressBar->setValue(0);
    progressBar->setMaximum(numQuestionsInSet > 0 ? numQuestionsInSet : 1);

    updateTimer();
    exerciseTimer->start(1000);

    startTranslationBtn->setEnabled(false);
    startGrammarBtn->setEnabled(false);
    setDifficultyAction->setEnabled(false);

    displayNextQuestion();
}

void MainWindow::displayNextQuestion()
{
    if (currentExerciseType == ExerciseType::Translation) {
        if (currentQuestionIndex < currentTranslationSet.size()) {
            displayTranslationQuestion();
            stackedWidget->setCurrentWidget(translationExercisePage);
        } else {
            endExercise(true, tr("Translation exercise completed successfully!"));
        }
    } else if (currentExerciseType == ExerciseType::Grammar) {
        if (currentQuestionIndex < currentGrammarSet.size()) {
            displayGrammarQuestion();
            stackedWidget->setCurrentWidget(grammarExercisePage);
        } else {
            endExercise(true, tr("Grammar exercise completed successfully!"));
        }
    }
    updateProgressBar();
}

void MainWindow::displayTranslationQuestion()
{
    const auto& q = currentTranslationSet.at(currentQuestionIndex);
    translationQuestionLabel->setText(tr("Translate: \"%1\"").arg(q.question));
    translationAnswerEdit->clear();
    translationAnswerEdit->setFocus();
}

void MainWindow::displayGrammarQuestion()
{
    const auto& q = currentGrammarSet.at(currentQuestionIndex);
    grammarQuestionLabel->setText(q.question);
    grammarOptionGroup->setExclusive(false);
    for (int i = 0; i < 4; ++i) {
        grammarOptions[i]->setText(q.options.at(i));
        grammarOptions[i]->setAutoExclusive(false);
        grammarOptions[i]->setChecked(false);
        grammarOptions[i]->setAutoExclusive(true);
    }
    grammarOptionGroup->setExclusive(true);
    if(grammarOptionGroup->checkedButton()) {
        grammarOptionGroup->checkedButton()->setChecked(false);
    }
}

void MainWindow::submitTranslationAnswer()
{
    if (currentExerciseType != ExerciseType::Translation || currentTranslationSet.isEmpty()) return;

    QString userAnswer = translationAnswerEdit->text().trimmed();
    const auto& currentQ = currentTranslationSet.at(currentQuestionIndex);

    if (userAnswer.compare(currentQ.correctAnswer, Qt::CaseInsensitive) == 0) {
        QMessageBox::information(this, tr("Correct!"), tr("Your answer is correct."));
        currentQuestionIndex++;
        displayNextQuestion();
    } else {
        incorrectAttempts++;
        QMessageBox::warning(this, tr("Incorrect"), tr("Your answer is incorrect. Try again.\nHint: %1\nAttempts left: %2")
                             .arg(currentQ.hint)
                             .arg(MAX_INCORRECT_ATTEMPTS - incorrectAttempts));
        if (incorrectAttempts >= MAX_INCORRECT_ATTEMPTS) {
            endExercise(false, tr("Too many incorrect attempts. Exercise failed."));
        }
    }
}

void MainWindow::submitGrammarAnswer()
{
    if (currentExerciseType != ExerciseType::Grammar || currentGrammarSet.isEmpty()) return;

    QRadioButton* selectedButton = qobject_cast<QRadioButton*>(grammarOptionGroup->checkedButton());
    if (!selectedButton) {
        QMessageBox::warning(this, tr("No Selection"), tr("Please select an answer."));
        return;
    }

    int selectedOptionIndex = grammarOptionGroup->id(selectedButton);
    const auto& currentQ = currentGrammarSet.at(currentQuestionIndex);

    if (selectedOptionIndex == currentQ.correctAnswerIndex) {
        QMessageBox::information(this, tr("Correct!"), tr("Your selection is correct."));
        currentQuestionIndex++;
        displayNextQuestion();
    } else {
        incorrectAttempts++;
        QMessageBox::warning(this, tr("Incorrect"), tr("Your selection is incorrect. Try again.\nHint: %1\nAttempts left: %2")
                             .arg(currentQ.hint)
                             .arg(MAX_INCORRECT_ATTEMPTS - incorrectAttempts));
        if (incorrectAttempts >= MAX_INCORRECT_ATTEMPTS) {
            endExercise(false, tr("Too many incorrect attempts. Exercise failed."));
        }
    }
}


void MainWindow::updateTimer()
{
    if (timeLeftSeconds > 0) {
        timeLeftSeconds--;
        int minutes = timeLeftSeconds / 60;
        int seconds = timeLeftSeconds % 60;
        timerLabel->setText(tr("Time: %1:%2").arg(minutes, 2, 10, QChar('0')).arg(seconds, 2, 10, QChar('0')));
    } else {
        handleTimeout();
    }
}

void MainWindow::handleTimeout()
{
    exerciseTimer->stop();
    endExercise(false, tr("Time's up! Exercise failed."));
}

void MainWindow::endExercise(bool success, const QString& message)
{
    exerciseTimer->stop();
    int questionsInThisSet = (currentExerciseType == ExerciseType::Translation) ?
                             currentTranslationSet.size() : currentGrammarSet.size();

    if (success && incorrectAttempts == 0 && questionsInThisSet > 0) {
        score += questionsInThisSet;
        scoreLabel->setText(tr("Score: %1").arg(score));
        QMessageBox::information(this, tr("Exercise Finished"), message + tr("\nCongratulations! You earned %1 points.").arg(questionsInThisSet));
    } else if (success && incorrectAttempts > 0) {
         QMessageBox::information(this, tr("Exercise Finished"), message + tr("\nYou completed the exercise, but made some mistakes. No points awarded for this round."));
    }
    else {
        QMessageBox::critical(this, tr("Exercise Failed"), message);
    }
    resetExerciseState();
}

void MainWindow::resetExerciseState()
{
    currentExerciseType = ExerciseType::None;
    currentQuestionIndex = 0;
    incorrectAttempts = 0;

    timeLeftSeconds = EXERCISE_DURATION_SECONDS;
    timerLabel->setText(tr("Time: %1:00").arg(EXERCISE_DURATION_SECONDS / 60, 2, 10, QChar('0')));
    if(exerciseTimer->isActive()) exerciseTimer->stop();

    progressBar->setValue(0);
    progressBar->setMaximum(TOTAL_QUESTIONS_PER_SESSION);

    currentTranslationSet.clear();
    currentGrammarSet.clear();

    stackedWidget->setCurrentWidget(welcomePage);

    startTranslationBtn->setEnabled(true);
    startGrammarBtn->setEnabled(true);
    setDifficultyAction->setEnabled(true);

    if(translationAnswerEdit) translationAnswerEdit->clear();
    if(grammarOptionGroup && grammarOptionGroup->checkedButton()) {
      grammarOptionGroup->setExclusive(false);
      grammarOptionGroup->checkedButton()->setChecked(false);
      grammarOptionGroup->setExclusive(true);
    }
}

void MainWindow::updateProgressBar()
{
    int totalQuestionsInSet = 0;
    if (currentExerciseType == ExerciseType::Translation && !currentTranslationSet.isEmpty()) {
        totalQuestionsInSet = currentTranslationSet.size();
    } else if (currentExerciseType == ExerciseType::Grammar && !currentGrammarSet.isEmpty()) {
        totalQuestionsInSet = currentGrammarSet.size();
    }

    if (totalQuestionsInSet > 0) {
        progressBar->setMaximum(totalQuestionsInSet);
        progressBar->setValue(currentQuestionIndex);
    } else {
         progressBar->setValue(0);
    }
}


void MainWindow::showHelp()
{
    if (currentExerciseType == ExerciseType::None || currentQuestionIndex < 0) {
        QMessageBox::information(this, tr("Help"), tr("No exercise is currently active or no question displayed. Select an exercise to begin."));
        return;
    }

    QString hintText = tr("No specific hint available for this general stage.");
    if (currentExerciseType == ExerciseType::Translation) {
        if (currentQuestionIndex < currentTranslationSet.size()) {
            hintText = currentTranslationSet.at(currentQuestionIndex).hint;
        }
    } else if (currentExerciseType == ExerciseType::Grammar) {
        if (currentQuestionIndex < currentGrammarSet.size()) {
            hintText = currentGrammarSet.at(currentQuestionIndex).hint;
        }
    }
    QMessageBox::information(this, tr("Hint"), hintText.isEmpty() ? tr("No hint provided for this question.") : hintText);
}

void MainWindow::keyPressEvent(QKeyEvent *event)
{
    if (event->key() == Qt::Key_H) {
        showHelp();
    } else {
        QMainWindow::keyPressEvent(event);
    }
}