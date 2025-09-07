#!/bin/bash

# Performance profiling script for Tesseract
set -e

echo "📊 Profiling Tesseract performance..."

# Build with profiling flags
echo "Building with profiling enabled..."
make clean
CFLAGS="-pg -O2" make -j$(nproc 2>/dev/null || sysctl -n hw.ncpu 2>/dev/null || echo 4)

# Create a test file for profiling
cat > profile_test.tesseract << 'EOF'
# Performance test script
func$fibonacci(n) => {
    if$ n <= 1 {
        n
    } else {
        fibonacci(n - 1) + fibonacci(n - 2)
    }
}

func$factorial(n) => {
    if$ n <= 1 {
        1
    } else {
        n * factorial(n - 1)
    }
}

# Run some computations
let$fib_result := fibonacci(25);
let$fact_result := factorial(10);

::print "Fibonacci(25): @s" (fib_result);
::print "Factorial(10): @s" (fact_result);

# Test data structures
let$list := [1, 2, 3, 4, 5];
loop$i := 1 => 1000 {
    ::append(list, i);
}

let$stack := <stack>;
loop$i := 1 => 1000 {
    ::push(stack, i);
}

loop$i := 1 => 1000 {
    ::pop(stack);
}
EOF

# Run the profiler
echo "Running profiler..."
./tesser profile_test.tesseract

# Generate profile report if gprof is available
if command -v gprof &> /dev/null; then
    echo "Generating profile report..."
    gprof ./tesser gmon.out > profile_report.txt
    echo "Profile report saved to profile_report.txt"
    
    # Show top functions
    echo "Top time-consuming functions:"
    head -20 profile_report.txt
else
    echo "gprof not available. Install it for detailed profiling reports."
fi

# Cleanup
rm -f profile_test.tesseract gmon.out

echo "✅ Profiling completed!"