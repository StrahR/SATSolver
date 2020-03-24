#include <array>
#include <cstdint>
#include <iostream>
#include <limits>
#include <string>
#include <vector>

enum class clause_data : int8_t { normal, negated, unspec };

struct clause {
  std::vector<clause_data> term;

  clause(int length /*, stdin*/) : term(length, clause_data::unspec) {
    int reader;
    while ((std::cin >> reader) && reader) {
      if (reader > 0) {
        term[reader - 1] = clause_data::normal;
      } else {
        term[-reader - 1] = clause_data::negated;
      }
    }
  }
};

std::vector<clause> parse(/*stdin*/) {
  char end_of_comment;
  while ((std::cin >> end_of_comment) && end_of_comment == 'c') {
    std::cin.ignore(std::numeric_limits<std::streamsize>::max(), '\n');
  }

  // ignore hardcoded "cnf"
  std::cin.ignore();

  int number_of_variables, number_of_clauses;
  std::cin >> number_of_variables >> number_of_clauses;

  std::vector<clause> formula;
  formula.reserve(number_of_clauses);

  for (int i = 0; i < number_of_clauses; ++i) {
    formula.emplace_back(number_of_variables /*, stdin*/);
  }

  return formula;
}

int main() {

  auto formula = parse(/*stdin*/);

  return 0;
}
