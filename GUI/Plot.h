#ifndef PLOT_H
#define PLOT_H

#include <QDir>
#include <QWidget>

namespace Ui {
class Plot;
}

class QwtPlot;

class Plot : public QWidget {
    Q_OBJECT

public:
    explicit Plot(QWidget *parent = 0);
    ~Plot();

    /// Colour to use for files with no explicitly defined colour
    QColor defaultColor {QColor::fromRgb(50, 50, 50)};
    QString colourFname = "colour.txt";

private slots:
    void validatePath();
    void on_browseSetPath_clicked();
    void refreshPlot();
    void enableControls (bool);

    void on_savePlot_clicked();

    void on_saveAll_clicked();

private:
    Ui::Plot *ui;
    QwtPlot* plot;
    /// A file containing data to plot
    struct colouredFiles {
        QList <QString> fnames; ///< the name of the files with coordinates
        QDir dir;
        QColor colour; ///< the colour for point corresponding to the files
    };
    /// Enumerate files that contain coordinates to plot
    /// Call `excludedFiles.clear()` before using this function
    /// @param[in] dir where to read files from
    /// @param[out] How many files are there totally (recursively)
    QList <colouredFiles> getAllFiles (const QDir& dir, int &nFiles);
};

#endif // PLOT_H
