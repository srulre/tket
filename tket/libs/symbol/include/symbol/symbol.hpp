// Copyright 2019-2022 Cambridge Quantum Computing
//
// Licensed under the Apache License, Version 2.0 (the "License");
// you may not use this file except in compliance with the License.
// You may obtain a copy of the License at
//
//     http://www.apache.org/licenses/LICENSE-2.0
//
// Unless required by applicable law or agreed to in writing, software
// distributed under the License is distributed on an "AS IS" BASIS,
// WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
// See the License for the specific language governing permissions and
// limitations under the License.

#pragma once

#include <nlohmann/json.hpp>

#if !defined(_MSC_VER)
#pragma GCC diagnostic push
#if defined(__clang__)
#pragma GCC diagnostic ignored "-Wambiguous-reversed-operator"
#endif
#pragma GCC diagnostic ignored "-Wunused-parameter"
#endif

#include <symengine/expression.h>
#include <symengine/visitor.h>  // for free_symbols

#if !defined(_MSC_VER)
#pragma GCC diagnostic pop
#endif

#include <map>
#include <optional>
#include <set>
#include <vector>

namespace tket {
namespace symbol {

/** Representation of a phase as a multiple of \f$ \pi \f$ */
typedef SymEngine::Expression Expr;

/** Shared pointer to an \p Expr */
typedef SymEngine::RCP<const SymEngine::Basic> ExprPtr;

/** Shared pointer to a free symbol */
typedef SymEngine::RCP<const SymEngine::Symbol> Sym;

}  // namespace symbol
}  // namespace tket

namespace nlohmann {

template <>
struct adl_serializer<tket::symbol::Expr> {
  static void to_json(json& j, const tket::symbol::Expr& exp) {
    tket::symbol::ExprPtr e_ = exp;
    j = e_->__str__();
  }

  static void from_json(const json& j, tket::symbol::Expr& exp) {
    exp = j.get<std::string>();
  }
};

template <>
struct adl_serializer<tket::symbol::Sym> {
  static void to_json(json& j, const tket::symbol::Sym& exp) {
    tket::symbol::ExprPtr e_ = exp;
    j = e_->__str__();
  }

  static void from_json(const json& j, tket::symbol::Sym& exp) {
    exp = SymEngine::symbol(j.get<std::string>());
  }
};

}  // namespace nlohmann

namespace tket {
namespace symbol {

/** Default tolerance for floating-point comparisons */
constexpr double SYMEPS = 1e-11;

struct SymCompareLess {
  bool operator()(const Sym& a, const Sym& b) const {
    return a->compare(*b) < 0;
  }
};

typedef std::set<Sym, SymCompareLess> SymSet;

/** Map from symbols to expressions */
typedef std::map<Sym, Expr, SymEngine::RCPBasicKeyLess> symbol_map_t;

/** Map from symbols to values */
typedef std::map<Sym, double, SymEngine::RCPBasicKeyLess> symbol_vals_t;

/** Set of all free symbols contained in the expression */
SymSet expr_free_symbols(const Expr& e);

/** Set of all free symbols contained in the expressions in the vector */
SymSet expr_free_symbols(const std::vector<Expr>& es);

std::optional<double> eval_expr(const Expr& e);

std::optional<std::complex<double>> eval_expr_c(const Expr& e);

/**
 * Evaluate an expression modulo n
 *
 * The result will be in the half-interval [0,n). If it is within SYMEPS of a
 * multiple of 0.25 the result is clamped to an exact multiple.
 *
 * @param e expression to evaluate
 * @param n modulus
 * @return value of expression modulo n, iff it has no free symbols
 */
std::optional<double> eval_expr_mod(const Expr& e, unsigned n = 2);

/**
 * Return cos(e*pi/2)
 *
 * If e is within @p SYMEPS of an integer then it is rounded so that the result
 * can be evaluated.
 */
Expr cos_halfpi_times(const Expr& e);

/**
 * Return sin(e*pi/2)
 *
 * If e is within @p SYMEPS of an integer then it is rounded so that the result
 * can be evaluated.
 */
Expr sin_halfpi_times(const Expr& e);

/**
 * Test if an expression is approximately zero
 *
 * @param e expression
 * @param tol tolerance
 *
 * @return whether \p e is within \p tol of zero
 */
bool approx_0(const Expr& e, double tol = SYMEPS);

/**
 * Evaluate modulo n in the range [0,n)
 *
 * @param x value to be reduced
 * @param n modulus
 *
 * @return y s.t. y == x (mod n) and 0 <= y < n
 */
double fmodn(double x, unsigned n);

/**
 * Test approximate equality of two values modulo n
 *
 * @param x first value
 * @param y second value
 * @param mod modulus
 * @param tol tolerance
 *
 * @return whether \p x is within \p tol of \p y modulo \p mod
 */
bool approx_eq(double x, double y, unsigned mod = 2, double tol = SYMEPS);

/**
 * Test approximate equality of expressions modulo n
 *
 * @param e0 expression
 * @param e1 expression
 * @param n modulus
 * @param tol tolerance
 *
 * @retval true \p e0 and \p e1 are within \p tol of each other modulo n
 * @retval false expressions are not within tolerance or could not ve evaluated
 */
bool equiv_expr(
    const Expr& e0, const Expr& e1, unsigned n = 2, double tol = SYMEPS);

/**
 * Test approximate value of an expression modulo n
 *
 * @param e expression
 * @param x test value
 * @param n modulus
 * @param tol tolerance
 *
 * @retval true \p e is within \p tol of \p x modulo n
 * @retval false expression is not within tolerance or could not be evaluated
 */
bool equiv_val(const Expr& e, double x, unsigned n = 2, double tol = SYMEPS);

/**
 * Test whether a expression is approximately 0 modulo n
 *
 * @param e expression
 * @param n modulus
 * @param tol tolerance
 *
 * @retval true \p e is within \p tol of 0 modulo n
 * @retval false expression is not within tolerance or could not be evaluated
 */
bool equiv_0(const Expr& e, unsigned n = 2, double tol = SYMEPS);

/**
 * Test whether an expression is approximately a Clifford angle (some multiple
 * of 0.5 modulo n)
 *
 * @param e expression
 * @param n modulus
 * @param tol tolerance
 *
 * @retval nullopt expression is not within tolerance or could not be evaluated
 * @retval u \f$ e \approx \frac12 u \pmod n \f$ where \f$ 0 \leq u < 2n \} \f$.
 */
std::optional<unsigned> equiv_Clifford(
    const Expr& e, unsigned n = 2, double tol = SYMEPS);

}  // namespace symbol
}  // namespace tket
