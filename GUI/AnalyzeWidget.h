#ifndef ANALYZEWIDGET_H
#define ANALYZEWIDGET_H

#include <QWidget>

class QLabel;
namespace Ui {
class AnalyzeWidget;
}

class AnalyzeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AnalyzeWidget(QWidget *parent = 0);
    ~AnalyzeWidget();


private slots:
    void on_browseLzmaPath_clicked();
    void on_browseSetPath_clicked();
    void processAllSeries ();
    void FindCorrelations ();
    void status (const QString& message);
    void on_go_clicked();
    void resizeCorrelations();
    void checkPaths();
    void on_setPath_textChanged(const QString &arg1);
    void on_lzmaPath_textChanged(const QString &arg1);

private:
    /// Get the directory with the result of the program
    QString destPath ();
    QLabel* correlations_label = nullptr;
    Ui::AnalyzeWidget *ui;
};

#endif // ANALYZEWIDGET_H
