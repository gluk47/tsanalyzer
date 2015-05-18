#ifndef GENERATEWIDGET_H
#define GENERATEWIDGET_H

#include <QWidget>
#include <functional>

namespace Ui {
class GenerateWidget;
}

class CoefftWidget;

class GenerateWidget : public QWidget
{
    Q_OBJECT

public:
    explicit GenerateWidget(QWidget *parent = 0);
    ~GenerateWidget();

private slots:
    void generate();
    void countSetSize ();

private:
    /**
     * @brief Call a function @f for all combinations of coefficients
     */
    void forAllCoefficients (std::function <void (const QVector <double>&)>&& f);
    /**
     * @brief iterate over the coefficients
     * @param k current values of all the coefficients
     * @param f function to call for each coefficients combination
     * @param iterateBy index of the coefficient being changed by this recursion level.
     *  if it is k.size() then f is actually called by this function,
     *  otherwise this function is recursively called for iterateBy = iterateBy + 1.
     */
    void iterateK (QVector<double>& k,
                   std::function<void (const QVector<double>&)>& f,
                   unsigned iterateBy);

    Ui::GenerateWidget *ui;
    unsigned setSize;
    QVector <CoefftWidget*> kWidgets;
};

#endif // GENERATEWIDGET_H
