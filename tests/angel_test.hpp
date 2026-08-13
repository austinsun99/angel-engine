struct TestResult {
    bool success;
    int line         = 0;
    const char *name = "";
};

using PFN_test = TestResult (*)();

namespace Pastel::Tests {
void run_tests();
void add_test(const char *name, PFN_test test);
}  // namespace Pastel::Tests

#define TEST_ASSERT(expr)                                                            \
    do {                                                                             \
        if (!(expr)) {                                                               \
            return TestResult{.success = false, .line = __LINE__, .name = __FILE__}; \
        }                                                                            \
    } while (false);

#define TEST_FUNC(name)                    \
    TestResult test_##name();              \
    namespace Pastel::Tests {              \
    struct Register_##name {               \
        Register_##name() {                \
            add_test(#name, &test_##name); \
        }                                  \
    } name##_registrator;                  \
    }                                      \
    TestResult test_##name()
