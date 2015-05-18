#ifndef HELPERS_H_8726b63b_e90f_4a0b_a7c2_2280a7a5bb03
#define HELPERS_H_8726b63b_e90f_4a0b_a7c2_2280a7a5bb03

// for size_t
#include <cstddef>
#include <functional>
class QWidget;
class QString;
#include <QObject>

struct coordinates_t {
    static constexpr size_t nValues = 8;
    union {
        double values [nValues];
        struct {
            double harmonicComplexity,
                   tendencyHarmonicComplexity,
                   KolmogorovComplexity,
                   tendencyKolmogorovComplexity,
                   fractalDimensionality,
                   tendencyFractalDimensionality,
                   characterVariability,
                   tendencycharacterVariability;
        } by_name;
    };
    /// User-readable coordinate name (in Russian)
    static const char* coordinateName (size_t index);
};


/**
 * @brief set css of the given widget to the reddish or greenish background.
 * @warning this is a quick-and-dirty function: the old css is erased.
 * @param w widget to modify
 * @param is_error whether red (true) or green (false) background to set
 */
void setErrorBackground (QWidget* w, bool is_error,
                         const QString& noerrorToolTip,
                         const QString& errorToolTip = QObject::tr("Файл не найден"));

struct scope_exit {
    scope_exit (std::function<void ()>&& f) : f (move(f)) {}
    ~scope_exit () { f(); }
private:
    std::function <void ()> f;
};

#endif // HELPERS_H
