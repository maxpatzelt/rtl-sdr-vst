#define JUCE_UNIT_TESTS 1

#include <juce_core/juce_core.h>

/**
 * Test Runner Main
 *
 * This file provides the entry point for running JUCE unit tests.
 * All test classes registered with JUCE's UnitTest system will be
 * discovered and executed.
 *
 * Tests are defined in separate files:
 * - test_Oscillator.cpp
 * - test_Envelope.cpp
 */

int main(int argc, char* argv[])
{
    juce::ignoreUnused(argc, argv);

    // Create a UnitTestRunner
    juce::UnitTestRunner runner;

    // Run all registered tests
    runner.runAllTests();

    // Get results
    int numTests = runner.getNumResults();
    int numPasses = 0;
    int numFailures = 0;

    for (int i = 0; i < numTests; ++i)
    {
        auto* result = runner.getResult(i);
        numPasses += result->passes;
        numFailures += result->failures;
    }

    // Print summary
    std::cout << "\n===========================================\n";
    std::cout << "Test Summary:\n";
    std::cout << "  Total Tests: " << numTests << "\n";
    std::cout << "  Passes: " << numPasses << "\n";
    std::cout << "  Failures: " << numFailures << "\n";
    std::cout << "===========================================\n";

    // Return 0 if all tests passed, 1 otherwise
    return (numFailures == 0) ? 0 : 1;
}
