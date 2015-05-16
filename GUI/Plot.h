#ifndef PLOT_H
#define PLOT_H

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

private slots:
    void validatePath();
    void on_browseSetPath_clicked();
    void refreshPlot();
    void enableControls (bool);

private:
    Ui::Plot *ui;
    QwtPlot* plot;
};

#endif // PLOT_H
