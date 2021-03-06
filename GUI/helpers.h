#ifndef HELPERS_H_8726b63b_e90f_4a0b_a7c2_2280a7a5bb03
#define HELPERS_H_8726b63b_e90f_4a0b_a7c2_2280a7a5bb03

// for size_t
#include <cstddef>
#include <functional>
class QWidget;
#include <QObject>
#include <QString>

struct coordinates_t {
    static constexpr size_t nValues = 10;
    union {
        double values [nValues];
        struct {
            double harmonicComplexity,
                   tendencyHarmonicComplexity,
                   fractalDimensionality,
                   tendencyFractalDimensionality,
                   symbolicDiversityWindow,
                   tendencySymbolicDiversityWindow,
                   symbolicDiversityDiff,
                   tendencySymbolicDiversityDiff,
                   KolmogorovComplexity,
                   tendencyKolmogorovComplexity;
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

struct scope_exit_t {
    scope_exit_t (std::function<void ()>&& f) : f (move(f)) {}
    ~scope_exit_t () { f(); }
private:
    std::function <void ()> f;
};

#define scope_exit(x) scope_exit_t scope_exit_object_ ## __LINE__ (x)

QString plural (const char* base,
                const char* one,
                const char* some,
                const char* many,
                int value,
                bool prepend_number = false);

#endif // HELPERS_H
