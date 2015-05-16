#ifndef MAINWINDOW_H_84ca4d9e_0126_4e0f_a6dd_95df30d53dba
#define MAINWINDOW_H_84ca4d9e_0126_4e0f_a6dd_95df30d53dba

#include <QMainWindow>

namespace Ui {
class MainWindow;
}

class MainWindow : public QMainWindow
{
    Q_OBJECT

public:
    explicit MainWindow(QWidget *parent = 0);
    ~MainWindow();

private slots:

private:
    Ui::MainWindow *ui;
};

#endif // MAINWINDOW_H
