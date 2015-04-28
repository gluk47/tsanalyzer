#include "GenerateWidget.h"
#include "ui_GenerateWidget.h"

GenerateWidget::GenerateWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::GenerateWidget)
{
    ui->setupUi(this);
}

GenerateWidget::~GenerateWidget()
{
    delete ui;
}
