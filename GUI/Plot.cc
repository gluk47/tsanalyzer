#include "Plot.h"
#include "ui_Plot.h"
#include "helpers.h"

#include <QDebug>
#include <QDir>
#include <QFileDialog>
#include <QTextStream>

#include "helpers.h"
#include "qcustomplot.h"

Plot::Plot(QWidget *parent) :
    QWidget(parent),
    ui(new Ui::Plot) {
    ui->setupUi(this);

    connect (ui->setPath, SIGNAL(textChanged(QString)),
             this, SLOT(validatePath()));
    connect (ui->refreshPlot, SIGNAL(clicked()),
             this, SLOT(refreshPlot()));

    ui->progressBar->hide();


    for (size_t i = 0; i < coordinates_t::nValues; ++i) {
        ui->axisX->addItem(coordinates_t::coordinateName(i));
        ui->axisY->addItem(coordinates_t::coordinateName(i));
    }

    validatePath();
}

Plot::~Plot() {
    delete ui;
}

void Plot::validatePath() {
    bool exists = QDir(ui->setPath->text()).exists();
    setErrorBackground(ui->setPath, not exists,
                       tr("Папка с выборкой временных рядов"));
    ui->refreshPlot->setEnabled(exists);
}

void Plot::on_browseSetPath_clicked() {
    auto ans = QFileDialog::getExistingDirectory(this,
                                 tr("Укажите папку с выборкой рядов"),
                                 ui->setPath->text());
    if (not ans.isNull())
        ui->setPath->setText(ans);
}

void Plot::refreshPlot() {
    enableControls(false);
    ui->progressBar->show();
    scope_exit restore_controls ([this]{
        ui->progressBar->hide();
        enableControls(true);
    });

    QApplication::processEvents();

    QDir setDir (ui->setPath->text());
    setDir.cd("processed"); //if exists
    ui->progressBar->setValue(0);
    QStringList lst = setDir.entryList(QStringList() << "*.coords",
                                       QDir::Readable | QDir::Files);
    if (lst.isEmpty())
        return;

    ui->progressBar->setMaximum(lst.size());
    QApplication::processEvents();
//    scene.clear();

    ui->plotWidget->addGraph(ui->plotWidget->yAxis, ui->plotWidget->xAxis);
    ui->plotWidget->graph(0)->setPen(QColor(50, 50, 50, 255));
    ui->plotWidget->graph(0)->setLineStyle(QCPGraph::lsNone);
    ui->plotWidget->graph(0)->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 4));

    QVector <double> x, y;

    x.reserve(lst.size());
    y.reserve(lst.size());

    double xmin = 0, xmax = 5, ymin = 0, ymax = 5;

    auto read_file = [setDir, this](const QString& fname, double& x, double& y){
        QFile file (setDir.filePath(fname));
        file.open(QIODevice::ReadOnly);
        QTextStream coordinates (&file);
        coordinates_t point;
        for (size_t j = 0; j < coordinates_t::nValues; ++j)
            coordinates >> point.values[j];
        x = point.values[ui->axisX->currentIndex()];
        y = point.values[ui->axisY->currentIndex()];
        return std::isfinite(x) and std::isfinite(y);
    };

    double x1, y1;
    for (const QString& fname : lst) {
        if (not read_file (fname, x1, y1))
            continue;
        xmin = xmax = x1;
        ymin = ymax = y1;
        break;
    }

    int progress = 0;
    for (QString fname : lst) {
        if (not read_file (fname, x1, y1))
            continue;
        x.append (x1);
        xmin = std::min (xmin, x1);
        xmax = std::max (xmax, x1);
        y.append (y1);
        ymin = std::min (ymin, y1);
        ymax = std::max (ymax, y1);
        ui->progressBar->setValue(++progress);
        if (progress % 20 == 0)
            QApplication::processEvents();
    }

    ui->plotWidget->graph(0)->setData(x, y);
    qDebug () << "Ranges:" << xmin << ".." << xmax << "×"
              << ymin << ".." << ymax;
    ui->plotWidget->xAxis->setRange(xmin, xmax);
    ui->plotWidget->yAxis->setRange(ymin, ymax);
}

void Plot::enableControls(bool _) {
    ui->refreshPlot->setEnabled(_);
    ui->axisX->setEnabled(_);
    ui->axisY->setEnabled(_);
    ui->setPath->setEnabled(_);
    ui->browseSetPath->setEnabled(_);
}
