#include "AnalyzeWidget.h"
#include "ui_AnalyzeWidget.h"

AnalyzeWidget::AnalyzeWidget(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::AnalyzeWidget)
{
    ui->setupUi(this);
}

AnalyzeWidget::~AnalyzeWidget()
{
    delete ui;
}
