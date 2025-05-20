#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>
#include <QStackedWidget>
#include <QVBoxLayout>
#include <QHBoxLayout>
#include <QPushButton>
#include <QLabel>
#include <QLineEdit>
#include <QRadioButton>
#include <QButtonGroup>
#include <QProgressBar>
#include <QMenuBar>
#include <QMenu>
#include <QAction>
#include <QTimer>
#include <QMessageBox>
#include <QFileDialog>
#include <QTextEdit>
#include <QList>
#include <QStringList>
#include <QRandomGenerator>
#include <QKeyEvent>
#include <QtSql/QSqlDatabase>
#include <QtSql/QSqlQuery>
#include <QtSql/QSqlError>

class DifficultyDialog;

struct TranslationQuestion {
    QString question;
    QString correctAnswer;
    QString hint;
};

struct GrammarQuestion {
    QString question;
    QStringList options;
    int correctAnswerIndex;
    QString hint;
};

enum class ExerciseType {
    None,
    Translation,
    Grammar
};

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    MainWindow(QWidget *parent = nullptr);
    ~MainWindow();

protected:
    void keyPressEvent(QKeyEvent *event) override;

private slots:
    void selectDifficulty();
    void startTranslationExercise();
    void startGrammarExercise();
    void submitTranslationAnswer();
    void submitGrammarAnswer();
    void updateTimer();
    void handleTimeout();
    void onDifficultySelected(const QString& difficulty);

private:
    void setupUI();
    void setupMenu();
    void createWelcomePage();
    void createTranslationPage();
    void createGrammarPage();
    bool initDatabase();
    void createTablesIfNotExist();
    bool importInitialDataIfNeeded();
    bool isDatabaseEmpty();
    bool importDifficulties();
    bool importQuestionsFromFile(const QString& filePath, ExerciseType type, int difficultyId);
    void loadQuestionsFromDb(ExerciseType type, const QString& difficulty);
    void startExercise(ExerciseType type);
    void displayNextQuestion();
    void displayTranslationQuestion();
    void displayGrammarQuestion();
    void checkAnswer();
    void endExercise(bool success, const QString& message);
    void resetExerciseState();
    void updateProgressBar();
    void showHelp();

    QStackedWidget *stackedWidget;
    QWidget *welcomePage;
    QWidget *translationExercisePage;
    QWidget *grammarExercisePage;
    QLabel *welcomeLabel;
    QLabel *translationQuestionLabel;
    QLineEdit *translationAnswerEdit;
    QPushButton *submitTranslationButton;
    QLabel *grammarQuestionLabel;
    QRadioButton *grammarOptions[4];
    QButtonGroup *grammarOptionGroup;
    QPushButton *submitGrammarButton;
    QProgressBar *progressBar;
    QLabel *scoreLabel;
    QLabel *timerLabel;
    QLabel *difficultyLabel;
    QMenu *fileMenu;
    QMenu *difficultyMenu;
    QAction *setDifficultyAction;
    QAction *exitAction;
    QPushButton *startTranslationBtn;
    QPushButton *startGrammarBtn;
    QString currentDifficulty;
    ExerciseType currentExerciseType;
    QList<TranslationQuestion> currentTranslationSet;
    QList<GrammarQuestion> currentGrammarSet;
    int currentQuestionIndex;
    int score;
    int incorrectAttempts;
    QTimer *exerciseTimer;
    int timeLeftSeconds;
    const int TOTAL_QUESTIONS_PER_SESSION = 10;
    const int MAX_INCORRECT_ATTEMPTS = 3;
    const int EXERCISE_DURATION_SECONDS = 5 * 60;
    DifficultyDialog *difficultyDialog;
    QSqlDatabase appDb;
    QString dbPath;
};

#include <QDialog>
#include <QComboBox>
#include <QDialogButtonBox>

class DifficultyDialog : public QDialog {
    Q_OBJECT
public:
    DifficultyDialog(const QString& currentDifficulty, QWidget *parent = nullptr) : QDialog(parent) {
        setWindowTitle(tr("Select Difficulty"));
        QVBoxLayout *layout = new QVBoxLayout(this);
        comboBox = new QComboBox(this);
        comboBox->addItems({"Elementary", "Intermediate", "Advanced"});
        comboBox->setCurrentText(currentDifficulty);
        layout->addWidget(comboBox);
        QDialogButtonBox *buttonBox = new QDialogButtonBox(QDialogButtonBox::Ok | QDialogButtonBox::Cancel, this);
        layout->addWidget(buttonBox);
        connect(buttonBox, &QDialogButtonBox::accepted, this, &DifficultyDialog::accept);
        connect(buttonBox, &QDialogButtonBox::rejected, this, &DifficultyDialog::reject);
    }
    QString selectedDifficulty() const { return comboBox->currentText(); }
private:
    QComboBox *comboBox;
};

#endif // MAINWINDOW_H