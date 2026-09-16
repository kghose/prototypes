/*
 * Let N be a number with and even count of digits (10, 1000 etc)
 * Let top(N) be the upper-half digits eg top(1000) = 10
 * Let bot(N) be the lower-half digits eg bot(1000) = 00
 * I define N to be a squishy square if sqrt(N) = top(N) + bot(N)
 *
 *  clang++ -std=c+=20 -O3 brute.cpp -o brute
 */
#include <array>
#include <cmath>
#include <cstdint>
#include <format>
#include <iostream>

inline uint64_t pow10(int exponent) {
  static constexpr std::array<uint64_t, 20> table = // Powers of 10 table ..
      {1ULL,
       10ULL,
       100ULL,
       1000ULL,
       10000ULL,
       100000ULL,
       1000000ULL,
       10000000ULL,
       100000000ULL,
       1000000000ULL,
       10000000000ULL,
       100000000000ULL,
       1000000000000ULL,
       10000000000000ULL,
       100000000000000ULL,
       1000000000000000ULL,
       10000000000000000ULL,
       100000000000000000ULL,
       1000000000000000000ULL,
       10000000000000000000ULL};

  // Eschewing range checks here ...

  return table[exponent];
}

bool is_squishy(uint64_t n, uint64_t n2, uint64_t midpoint_divisor) {
  uint64_t top = n2 / midpoint_divisor;
  uint64_t bot = n2 % midpoint_divisor;
  return n == top + bot;
}

int main(int argc, char *argv[]) {
  if (argc != 2) {
    std::cout << "Usage: ./brute <N> where the maximum squishy square to be "
                 "computed "
                 "has 2N digits";
    exit(1);
  }

  uint64_t max_root_digits = atoi(argv[1]);
  if (max_root_digits > 9) {
    std::cout << "Sorry, this program will only work for N <= 9" << std::endl;
    exit(1);
  }

  for (uint64_t root_digits = 1; root_digits <= max_root_digits;
       root_digits++) {
    uint64_t midpoint_divisor = pow10(root_digits);
    uint64_t min_root_to_test = ceil(sqrt(pow10(2 * root_digits - 1)));
    uint64_t max_root_to_test = midpoint_divisor - 1;

    for (uint64_t n = min_root_to_test; n <= max_root_to_test; n++) {
      uint64_t n2 = n * n;
      if (is_squishy(n, n2, midpoint_divisor)) {
        std::cout << std::format("{:<10} -> {:<20}\n", n, n2);
      }
    }
  }

  return 0;
}
