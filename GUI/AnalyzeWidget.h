#ifndef ANALYZEWIDGET_H
#define ANALYZEWIDGET_H

#include <QWidget>

namespace Ui {
class AnalyzeWidget;
}

class AnalyzeWidget : public QWidget
{
    Q_OBJECT

public:
    explicit AnalyzeWidget(QWidget *parent = 0);
    ~AnalyzeWidget();

private:
    Ui::AnalyzeWidget *ui;
};

#endif // ANALYZEWIDGET_H
