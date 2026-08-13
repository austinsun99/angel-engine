#include <unordered_map>
#include "angel_test.hpp"
#include <core/logging/logger.h>

namespace angel::tests {
static std::unordered_map<const char *, PFN_test> tests;

void add_test(const char *name, PFN_test test) {
    tests.insert({name, test});
}

void run_tests() {
    int num_tests = 0;
    int pass      = 0;
    int fail      = 0;

    for (auto const &[name, test] : ::angel::tests::tests) {
        ++num_tests;

        AL_CORE_INFO("Running test: %s", name);
        TestResult res = test();
        if (!res.success) {
            ++fail;
            AL_CORE_ERROR("[x] Test %s in %s failed at: %d", name, res.name, res.line);
        } else {
            ++pass;
            AL_CORE_INFO("[✔]\n");
        }
    }

    AL_CORE_INFO("\n\nTest Summary (%d) | Passing: %d | Failing: %d |", num_tests, pass, fail)
}
}  // namespace angel::tests

int main(void) {
    angel::tests::run_tests();
}
