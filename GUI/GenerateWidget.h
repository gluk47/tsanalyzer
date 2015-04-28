#ifndef GENERATEWIDGET_H
#define GENERATEWIDGET_H

#include <QWidget>

namespace Ui {
class GenerateWidget;
}

class GenerateWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GenerateWidget(QWidget *parent = 0);
    ~GenerateWidget();

private:
    Ui::GenerateWidget *ui;
};

#endif // GENERATEWIDGET_H
