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
    ui->progressMsg->hide();

    for (size_t i = 0; i < coordinates_t::nValues; ++i) {
        ui->axisX->addItem(coordinates_t::coordinateName(i));
        ui->axisY->addItem(coordinates_t::coordinateName(i));
    }

    ui->plotWidget->setInteraction(QCP::iRangeDrag, true);
    ui->plotWidget->setInteraction(QCP::iRangeZoom, true);

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

// avoiding symlink loops (I don't know who and why may create them but anyway)
// yeah, that's a silly single-threaded solution
// since we're in the GUI thread, that's no problem.
static QSet <QString> excludedPaths;

QList <Plot::colouredFiles> Plot::getAllFiles (const QDir& dir, int& nFiles) {
    excludedPaths.insert(dir.canonicalPath());

    qDebug () << "+++" << dir.canonicalPath();

    const QColor colour = [this, dir]{
        qDebug () << "Looking for file" << colourFname;
        QFile colourFile (dir.filePath(colourFname));
        if (not colourFile.exists()) {
            qDebug () << "... not found";
            return defaultColor;
        }
        colourFile.open(QIODevice::ReadOnly);
        if (not colourFile.isOpen()) {
            qDebug () << "... failed to open";
            return defaultColor;
        }
        int r = -1, g = -1, b = -1;
        QTextStream filestream (&colourFile);
        filestream >> r >> g >> b;
        qDebug () << "... read colors: " << r << g << b;
        if (r == -1 or g == -1 or b == -1)
            return defaultColor;
        return QColor::fromRgb(r, g, b);
    }();

    QStringList lst = dir.entryList(QStringList() << "*.coords",
                                    QDir::Readable | QDir::Files);
    QList <Plot::colouredFiles> ans;
    nFiles = lst.size();
    if (nFiles != 0)
        ans.append(colouredFiles {lst, dir, colour});

    lst = dir.entryList(QDir::Readable | QDir::Dirs | QDir::NoDotAndDotDot);
    qDebug () << "Examining subfolders of" << dir.absolutePath() << "("
              << lst.size() << "entries)...";
    for (const auto& d : lst) {
        QDir nextDir (dir.filePath(d));
        if (excludedPaths.contains(nextDir.canonicalPath())) {
            qDebug () << "Skipping" << nextDir.absolutePath() << "(already analyzed)";
            continue;
        }
        excludedPaths.insert(nextDir.canonicalPath());
        int nMoreFiles;
        ans += getAllFiles(nextDir, nMoreFiles);
        nFiles += nMoreFiles;
    }

    qDebug () << "---" << dir.absolutePath();

    return ans;
}

void Plot::refreshPlot() {
    enableControls(false);
    ui->progressBar->show();
    scope_exit ([this]{
        ui->progressBar->hide();
        enableControls(true);
    });

    QApplication::processEvents();

    QDir setDir (ui->setPath->text());
    setDir.cd("processed"); //if exists
    ui->progressMsg->setText(tr("Чтение списка файлов..."));
    excludedPaths.clear();
    int nFiles;
    auto lst = getAllFiles(setDir, nFiles);
    if (lst.isEmpty() or nFiles == 0) {
        qDebug () << "";
        return;
    }

    ui->progressBar->setValue(0);
    ui->progressBar->setMaximum(nFiles);
    QApplication::processEvents();
//    scene.clear();
    ui->plotWidget->xAxis->setLabel(coordinates_t::coordinateName(ui->axisX->currentIndex()));
    ui->plotWidget->yAxis->setLabel(coordinates_t::coordinateName(ui->axisY->currentIndex()));

    double xmin = 0, xmax = 5, ymin = 0, ymax = 5;

    auto read_file = [setDir, this](const QString& fname, double& x, double& y){
        QFile file (setDir.filePath(fname));
        file.open(QIODevice::ReadOnly);
        if (file.size() < 2)
            // some invalid empty file
            return false;
        QTextStream coordinates (&file);
        coordinates_t point;
        for (size_t j = 0; j < coordinates_t::nValues; ++j)
            coordinates >> point.values[j];
        x = point.values[ui->axisX->currentIndex()];
        y = point.values[ui->axisY->currentIndex()];
        return std::isfinite(x) and std::isfinite(y);
    };

    double x1, y1;
    for (auto& cset : lst)
        for (const QString& fname : cset.fnames) {
            // read the first file and set starting values for x and y ranges
            if (not read_file (fname, x1, y1))
                continue;
            xmin = xmax = x1;
            ymin = ymax = y1;
            goto got_ranges;
        }
got_ranges:

    int progress = 0;
    int graph_no = 0;
    for (const colouredFiles& files : lst) {
        const QColor colour = files.colour;
        const QDir& dir = files.dir;
        ui->plotWidget->addGraph(ui->plotWidget->yAxis, ui->plotWidget->xAxis);
        auto graph = ui->plotWidget->graph(graph_no++);
        graph->setLineStyle(QCPGraph::lsNone);
        graph->setScatterStyle(QCPScatterStyle(QCPScatterStyle::ssCircle, 4));
        graph->setPen(colour);

        QVector <double> x, y;

        x.reserve(lst.size());
        y.reserve(lst.size());

        for (const auto& fname : files.fnames) {
            ui->progressBar->setValue(progress++);
            if (progress % 20 == 0)
                QApplication::processEvents();
            if (not read_file (dir.filePath(fname), x1, y1))
                continue;
            if (std::isnan(x1) or std::isnan(y1) or std::isinf(x1) or std::isinf(y1))
                // that means that the coordinate is not applicable to the series, just ignore this point
                continue;
            x.append (x1);
            xmin = std::min (xmin, x1);
            xmax = std::max (xmax, x1);
            y.append (y1);
            ymin = std::min (ymin, y1);
            ymax = std::max (ymax, y1);
        }

        graph->setData(y, x);
    }

    qDebug () << "Range:" << xmin << ".." << xmax << "×"
              << ymin << ".." << ymax;

    auto adjust_range = [](auto& rmin, auto& rmax) {
        if (rmin == rmax) {
            if (rmin == 0) {
                rmin = -.03;
                rmax = +.03;
            } else {
                rmin *=  .97;
                rmax *= 1.03;
            }
        } else {
            auto delta = (rmax - rmin) * .03;
            rmin -= delta;
            rmax += delta;
        }
    };

    adjust_range (xmin, xmax);
    adjust_range (ymin, ymax);

    qDebug () << "Adjusted range:" << xmin << ".." << xmax << "×"
              << ymin << ".." << ymax;

    ui->plotWidget->xAxis->setRange(xmin, xmax);
    ui->plotWidget->yAxis->setRange(ymin, ymax);
    ui->plotWidget->replot();
}

void Plot::enableControls(bool _) {
    ui->refreshPlot->setEnabled(_);
    ui->axisX->setEnabled(_);
    ui->axisY->setEnabled(_);
    ui->setPath->setEnabled(_);
    ui->browseSetPath->setEnabled(_);
}

void Plot::on_savePlot_clicked() {
    auto fname = QFileDialog::getSaveFileName(this,
                                              tr("Укажите папку с выборкой рядов"),
                                              ui->setPath->text(),
                                              tr("PDF (*.pdf);;"
                                                 "Png image (*.png);;"
                                                 "Jpeg image(*.jpg)"));
    if (fname.isNull())
        return;

    QFileInfo file (fname);

    if (file.suffix().toLower() == "pdf")
        ui->plotWidget->savePdf(fname);
    else if (file.suffix().toLower() == "png")
        ui->plotWidget->savePng(fname);
    else if (file.suffix().toLower() == "jpg")
        ui->plotWidget->saveJpg(fname);
}

void Plot::on_saveAll_clicked() {
    const auto x = ui->axisX->currentIndex(), y = ui->axisY->currentIndex();
    scope_exit (([this, x, y]{
        ui->axisX->setCurrentIndex(x);
        ui->axisY->setCurrentIndex(y);
        refreshPlot();
    }));

    QDir target (ui->setPath->text());
    target.mkdir("plots");
    target.cd("plots");

    QString cmdline;
    for (unsigned i = 0; i < coordinates_t::nValues - 1; ++i)
        for (unsigned j = i + 1; j < coordinates_t::nValues; ++j) {
            ui->axisX->setCurrentIndex(i);
            ui->axisY->setCurrentIndex(j);
            refreshPlot();
            auto fname = target.absoluteFilePath(QString("%1×%2.pdf")
                                                 .arg(coordinates_t::coordinateName(i))
                                                 .arg(coordinates_t::coordinateName(j)));
            cmdline += QString ("'%1' ").arg(fname);
            ui->plotWidget->savePdf(fname);
        }

    system (ui->pdftk->text()
            .arg(cmdline)
            .arg(target.absoluteFilePath("plots.pdf"))
            .toLocal8Bit());
}
