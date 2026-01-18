#ifndef MAINWINDOW_H
#define MAINWINDOW_H

#include <QMainWindow>

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
    void on_actionQuit_triggered();

    void on_buttonConvert_clicked();

    void on_buttonClear_clicked();

    void on_actionClearTextInput_triggered();

    void on_actionConvert_triggered();

    void on_actionAboutMCFCToJson_triggered();

private:
    Ui::MainWindow *ui;
};
#endif // MAINWINDOW_H
