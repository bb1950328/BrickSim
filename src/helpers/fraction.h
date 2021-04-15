

#ifndef BRICKSIM_FRACTION_H
#define BRICKSIM_FRACTION_H

#include <string>

class Fraction {
    long a;
    long b;
    void checkBnot0() const;
    void simplify();
public:
    Fraction(long a, long b);
    Fraction(const Fraction& copyFrom);
    Fraction operator+(const Fraction& other) const;
    Fraction operator-(const Fraction& other) const;
    Fraction operator*(const Fraction& other) const;
    Fraction operator/(const Fraction& other) const;

    Fraction operator+=(const Fraction& other);
    Fraction operator-=(const Fraction& other);
    Fraction operator*=(const Fraction& other);
    Fraction operator/=(const Fraction& other);

    Fraction operator+(long other) const;
    Fraction operator-(long other) const;
    Fraction operator*(long other) const;
    Fraction operator/(long other) const;

    Fraction operator+=(long other);
    Fraction operator-=(long other);
    Fraction operator*=(long other);
    Fraction operator/=(long other);

    bool operator==(const Fraction& other) const;
    bool operator!=(const Fraction& other) const;
    bool operator>(const Fraction& other) const;
    bool operator<(const Fraction& other) const;
    bool operator>=(const Fraction& other) const;
    bool operator<=(const Fraction& other) const;

    [[nodiscard]] std::string to_string() const;
    [[nodiscard]] std::string to_multiline_string() const;

    [[nodiscard]] long getA() const;
    [[nodiscard]] long getB() const;

    friend std::ostream &operator<<(std::ostream &os, const Fraction &fraction);
};
#endif //BRICKSIM_FRACTION_H
