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

#include <catch2/catch.hpp>
#include <symbol/symbol.hpp>

#include "../testutil.hpp"

namespace tket {
namespace test_Expression {

SCENARIO("Basic symbol::Expr evaluation", "[ops]") {
  GIVEN("A constant") {
    symbol::Expr e(2.5);
    REQUIRE(test_equiv_val(e, 0.5));
  }
  GIVEN("A symbol") {
    symbol::Sym s = SymEngine::symbol("a");
    symbol::Expr e = symbol::Expr(s);
    REQUIRE(!symbol::eval_expr_mod(e));
    SymEngine::map_basic_basic smap;
    smap[s] = symbol::Expr(3.4);
    symbol::Expr ee = e.subs(smap);
    REQUIRE(test_equiv_val(ee, 1.4));
  }
  GIVEN("A non-empty sum") {
    symbol::Sym s = SymEngine::symbol("b");
    symbol::Expr e = 0.2 + symbol::Expr(s) + 0.5 + symbol::Expr(s);
    REQUIRE(!symbol::eval_expr_mod(e));
    SymEngine::map_basic_basic smap;
    smap[s] = symbol::Expr(0.3);
    symbol::Expr ee = e.subs(smap);
    REQUIRE(test_equiv_val(ee, 1.3));
  }
  GIVEN("A non-empty product") {
    symbol::Sym s = SymEngine::symbol("b");
    symbol::Expr e = 0.2 * symbol::Expr(s) * 0.5 * symbol::Expr(s);
    REQUIRE(!symbol::eval_expr_mod(e));
    SymEngine::map_basic_basic smap;
    smap[s] = symbol::Expr(3.);
    symbol::Expr ee = e.subs(smap);
    REQUIRE(test_equiv_val(ee, 0.9));
  }
  GIVEN("A more complicated expression") {
    symbol::Sym s = SymEngine::symbol("d");
    symbol::Expr e =
        -0.3 + (3.4 * symbol::Expr(SymEngine::sin(symbol::Expr(s) - 2.3)));
    SymEngine::map_basic_basic smap;
    smap[s] = symbol::Expr(2.3);
    symbol::Expr ee = e.subs(smap);
    REQUIRE(test_equiv_val(ee, 1.7));
  }
}

SCENARIO("Expression uniqueness", "[ops]") {
  GIVEN("Two equivalent constants") {
    symbol::Expr a(0.5);
    symbol::Expr b(2 * 3. / 4 - 1);
    REQUIRE(a == b);
  }
  GIVEN("Two different constants") {
    symbol::Expr a(2.);
    symbol::Expr b0(2);
    symbol::Expr b1(3.);
    REQUIRE(a != b0);
    REQUIRE(a != b1);
  }
  GIVEN("Two identical symbols") {
    symbol::Expr a("alpha");
    symbol::Expr b("alpha");
    REQUIRE(a == b);
  }
  GIVEN("Two different symbols") {
    symbol::Expr a("alpha");
    symbol::Expr b("beta");
    REQUIRE(a != b);
  }
  GIVEN("Parsed atan2") {
    symbol::Expr a("alpha");
    symbol::Expr b("beta");
    symbol::Expr at(SymEngine::atan2(a, b));
    symbol::Expr at2 = symbol::Expr("atan2(alpha, beta)");
    REQUIRE(at == at2);
  }
}

}  // namespace test_Expression
}  // namespace tket
