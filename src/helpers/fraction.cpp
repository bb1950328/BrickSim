#include "fraction.h"
#include <numeric>
#include <ostream>
#include <stdexcept>

namespace bricksim {
    Fraction::Fraction(long a, long b) :
        a(a), b(b) {
        checkBnot0();
        simplify();
    }

    void Fraction::checkBnot0() const {
        if (0 == b) {
            throw std::invalid_argument("b must not be 0");
        }
    }

    void Fraction::simplify() {
        auto gcd = std::gcd(std::abs(a), std::abs(b));
        a /= gcd;
        b /= gcd;
    }

    Fraction Fraction::operator+(const Fraction& other) const {
        return Fraction(a * other.b + other.a * b, b * other.b);
    }

    Fraction Fraction::operator-(const Fraction& other) const {
        return Fraction(a * other.b - other.a * b, b * other.b);
    }

    Fraction Fraction::operator*(const Fraction& other) const {
        return Fraction(a * other.a, b * other.b);
    }

    Fraction Fraction::operator/(const Fraction& other) const {
        return Fraction(a * other.b, b * other.a);
    }

    Fraction Fraction::operator+=(const Fraction& other) {
        auto tmpA = a * other.b + other.a * b;
        auto tmpB = b * other.b;
        a = tmpA;
        b = tmpB;
        return Fraction(*this);
    }

    Fraction Fraction::operator-=(const Fraction& other) {
        auto tmpA = a * other.b - other.a * b;
        auto tmpB = b * other.b;
        a = tmpA;
        b = tmpB;
        simplify();
        return Fraction(*this);
    }

    Fraction Fraction::operator*=(const Fraction& other) {
        a = a * other.a;
        b = b * other.b;
        simplify();
        return Fraction(*this);
    }

    Fraction Fraction::operator/=(const Fraction& other) {
        a = a * other.b;
        b = b * other.a;
        simplify();
        return Fraction(*this);
    }

    Fraction Fraction::operator+(long other) const {
        return Fraction(a + other * b, b);
    }

    Fraction Fraction::operator-(long other) const {
        return Fraction(a - other * b, b);
    }

    Fraction Fraction::operator*(long other) const {
        return Fraction(a * other, b);
    }

    Fraction Fraction::operator/(long other) const {
        return Fraction(a, b * other);
    }

    Fraction Fraction::operator+=(long other) {
        a = a + other * b;
        simplify();
        return Fraction(*this);
    }

    Fraction Fraction::operator-=(long other) {
        a = a - other * b;
        simplify();
        return Fraction(*this);
    }

    Fraction Fraction::operator*=(long other) {
        a = a * other;
        simplify();
        return Fraction(*this);
    }

    Fraction Fraction::operator/=(long other) {
        b = b * other;
        simplify();
        return Fraction(*this);
    }

    bool Fraction::operator>(const Fraction& other) const {
        return a * other.b > other.a * b;
    }

    bool Fraction::operator<(const Fraction& other) const {
        return a * other.b < other.a * b;
    }

    bool Fraction::operator>=(const Fraction& other) const {
        return (a == other.a && b == other.b) || a * other.b > other.a * b;
    }

    bool Fraction::operator<=(const Fraction& other) const {
        return (a == other.a && b == other.b) || a * other.b < other.a * b;
    }

    std::ostream& operator<<(std::ostream& os, const Fraction& fraction) {
        os << "(" << fraction.a << " / " << fraction.b << ")";
        return os;
    }

    std::string Fraction::to_string() const {
        return std::string("(") + std::to_string(a) + " / " + std::to_string(b) + ")";
    }

    std::string Fraction::to_multiline_string() const {
        auto strA = std::to_string(a);
        auto strB = std::to_string(b);
        return strA + "\n" + std::string(std::max(strA.size(), strB.size()), '-') + "\n" + strB;
    }

    long Fraction::getA() const {
        return a;
    }

    long Fraction::getB() const {
        return b;
    }

    bool Fraction::operator==(long other) const {
        return b == 1 && a == other;
    }

    bool Fraction::operator!=(long other) const {
        return b != 1 || a != other;
    }

    bool Fraction::operator>(long other) const {
        return a > b * other;
    }

    bool Fraction::operator<(long other) const {
        return a < b * other;
    }

    bool Fraction::operator>=(long other) const {
        return a >= b * other;
    }

    bool Fraction::operator<=(long other) const {
        return a <= b * other;
    }
}
