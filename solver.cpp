#include <cstdint>
#include <iostream>
#include <limits>
#include <string>
#include <vector>
#include <functional>
#include <optional>
#include <map>


enum class clause_data : int8_t { normal, negated, unspec };

typedef std::pair<int, clause_data> Literal;

Literal neg(const Literal& lit)
{
    auto [literal, polarity] = lit;
    std::cerr << "as";
    switch (polarity) {
    case clause_data::normal:
        return std::make_pair(literal, clause_data::negated);
    case clause_data::negated:
        return std::make_pair(literal, clause_data::normal);
    case clause_data::unspec: ;
    }
    std::cerr << "test";
    __builtin_unreachable();
}

struct clause {
    std::vector<clause_data> term;

    clause() {}

    clause(const int length /*, stdin*/) : term(length, clause_data::unspec)
    {
        int reader;
        while ((std::cin >> reader) && reader) {
            if (reader > 0) {
                term[reader - 1] = clause_data::normal;
            } else {
                term[-reader - 1] = clause_data::negated;
            }
        }
    }


    clause(const clause& cl) : term(cl.term) {}
    /*
    clause(const Literal& lit) : term(LENGTH, clause_data::unspec)
    {
        auto [literal, polarity] = lit;
        term[literal] = polarity;
    }*/

    std::optional<clause> unit_propagate_group(clause units)
    {
        clause cl(*this); // make lazy construction?

        for (auto literal = 0ul; literal < term.size(); ++literal) {
            if (units.term[literal] == clause_data::unspec) {
                continue;
            }

            // propagate all units
            if (units.term[literal] == term[literal]) {
                return std::nullopt;
            } else { //if clause contains negation
                cl.term[literal] = clause_data::unspec;
            }
        }
        return cl;
    }

    std::optional<clause> unit_propagate_one(size_t lit, clause_data polarity) const
    {
        if (term[lit] == clause_data::unspec) {
            return *this;
        } else if (term[lit] == polarity) {
            return std::nullopt;
        } else {
            clause cl(*this); // TODO: remove expensive copy
            cl.term[lit] = clause_data::unspec;
            return cl;
        }
    }

    bool empty()
    {
        for (auto&& x : term) {
            if (x != clause_data::unspec) {
                return false;
            }
        }
        return true;
    }

    std::optional<size_t> unit()
    {
        // {} if it isn't
        // index of truth value if it is
        bool indicator = false;
        size_t index = 0;

        for (size_t i = 0; i < term.size(); ++i) {
            if (indicator && term[i] != clause_data::unspec) {
                return {};
            }

            // find first non-unspec
            if (term[i] != clause_data::unspec) {
                indicator = true;
                index = i;
            }
        }
        if (indicator) { // at least one found
            return index;
        }
        return {};
    }

};

struct Formula {
    std::vector<clause> formula;
    size_t clause_length;
    clause solution;

    void print() const
    {
        for (auto&& cl : formula) {
            std::cout << "(|";
            for (int literal = 0; literal < clause_length; literal++) {
                switch (cl.term[literal]) {
                case clause_data::normal:
                    std::cout << literal + 1 << '|';
                    break;
                case clause_data::negated:
                    std::cout << '!' << literal + 1 << '|';
                    break;
                case clause_data::unspec: ;
                }
            }
            std::cout << ')';
        }
        std::cout << std::endl;
    }

    void print_solution() const
    {
        for (size_t x = 0; x < clause_length; ++x) {
            switch (solution.term[x]) {
            case clause_data::normal:
                std::cout << x + 1 << "=1, ";
                break;
            case clause_data::negated:
                std::cout << x + 1 << "=0, ";
                break;
            case clause_data::unspec:;
            }
        }
        std::cout << std::endl;
    }

    void unit_propagate_group(clause units)
    {
        // https://en.wikipedia.org/wiki/Unit_propagation
        std::vector<clause> f; f.reserve(formula.size());
        for (auto&& e : formula) {
            if (auto cl = e.unit_propagate_group(units)) {
                f.push_back(*cl);
            }
        }
        formula = f;
    }

    void unit_propagate_single(size_t index, clause_data polarity)
    {
        std::vector<clause> f; f.reserve(formula.size());
        for (auto&& e : formula) {
            if (auto cl = e.unit_propagate_one(index, polarity)) {
                f.push_back(*cl);
            }
        }
        formula = f;
    }

    std::optional<clause> unit_clauses()
    {
        // returns a clause containing all units and their polarity
        clause units;
        units.term.resize(clause_length, clause_data::unspec);

        // note check for contradiction
        for (auto&& e : formula) {
            if (auto p = e.unit(); p.has_value()) { // has_value unnecessary?
                // e[p] is unital then
                if (units.term[p.value()] != clause_data::unspec &&
                    e.term[p.value()] != units.term[p.value()])
                    return {}; // contradiction
                units.term[p.value()] = e.term[p.value()];
            }
        }

        return units;
    }

    clause pure_literals()
    {
        // returns a clause which describes the literals
        // the elements with unspec aren't literals,
        // elements with clause::normal are normal, negated are negated literals
        clause pure;
        pure.term.resize(clause_length, clause_data::unspec);

        for (size_t i = 0; i < clause_length; ++i) {
            bool b1 = false;
            bool b2 = false;
            for (auto&& el : formula) {
                switch (el.term[i]) {
                case clause_data::normal: b1 = true;
                case clause_data::negated: b2 = true;
                case clause_data::unspec: ;
                }
            }
            if (!b1 and b2) {
                pure.term[i] = clause_data::negated;
            }
            if (b1 and !b2) {
                pure.term[i] = clause_data::normal;
            }
        }
        return pure;
    }

    bool contains_empty_clause() const
    {
        for (auto&& cl : formula) {
            if (cl.term.empty()) {
                return true;
            }
        }
        return false;
    }

    std::optional<size_t> choose_literal() const
    {
        for (auto&& cl : formula) {
            for (size_t literal = 0; literal < clause_length; ++literal) {
                if (cl.term[literal] != clause_data::unspec) {
                    return literal;
                }
            }
        }
        return std::nullopt;
    }
};


bool recursive_DPLL(Formula& expr)
{
    if (expr.formula.empty()) {
        return true;
    }
    if (expr.contains_empty_clause()) {
        return false;
    }
    // !units.has_value() || !(*units).empty();
    // {} or it has actual values
    for (auto units = expr.unit_clauses(); !units.has_value() || !units.value().empty(); units = expr.unit_clauses()) {
        if (!units.has_value()) {
            return false; // contradiction
        }

        // do we have to check?
        for (size_t i = 0; i < expr.clause_length; ++i) {
            if (expr.solution.term[i] != clause_data::unspec &&
                expr.solution.term[i] != units.value().term[i]
               ) {
                return false;
            }
            expr.solution.term[i] = units.value().term[i];
        }

        expr.unit_propagate_group(units.value());
    }

    if (auto p = expr.choose_literal()) {
        Formula ff(expr); // copy for branch
        expr.solution.term[p.value()] = clause_data::normal;
        expr.unit_propagate_single(p.value(), clause_data::normal);

        if (recursive_DPLL(expr)) {
            return true;
        } else {
            ff.solution.term[p.value()] = clause_data::negated;
            ff.unit_propagate_single(p.value(), clause_data::negated);
            return recursive_DPLL(ff);
        }
    } else {
        return recursive_DPLL(expr);
    }
}

bool DPLL(Formula& expr)
{
    auto pures = expr.pure_literals();
    expr.solution = pures;
    expr.unit_propagate_group(pures);
    return recursive_DPLL(expr);
}

Formula parse(/*stdin*/)
{
    char end_of_comment;
    while ((std::cin >> end_of_comment) && end_of_comment == 'c') {
        std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
    }

    // ignore hardcoded "cnf"
    std::cin.ignore(10, 'f');

    int number_of_variables, number_of_clauses;
    std::cin >> number_of_variables >> number_of_clauses;


    Formula f;
    f.clause_length = number_of_variables;
    f.formula.reserve(number_of_clauses);

    for (int i = 0; i < number_of_clauses; ++i) {
        f.formula.emplace_back(number_of_variables /*, stdin*/);
    }

    return f;
}

int main()
{
    auto formula = parse(/*stdin*/);

    std::cout << DPLL(formula) << '\n';

    return 0;
}
