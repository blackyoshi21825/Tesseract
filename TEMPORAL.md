# Temporal Programming in Tesseract

Tesseract features built-in temporal programming capabilities, allowing variables to maintain history and enabling time-aware computations.

## Temporal Variables

### Declaration
```tesseract
let$ var := <temp@N>  # N is the history size
```

### Assignment and History
```tesseract
let$ x := <temp@3>
let$ x := 10    # x@0 = 10
let$ x := 20    # x@0 = 20, x@1 = 10
let$ x := 30    # x@0 = 30, x@1 = 20, x@2 = 10
```

### Accessing History
```tesseract
x@0  # current value (30)
x@1  # previous value (20)
x@2  # value before that (10)
```

## Temporal Loops

Iterate through a variable's history:

```tesseract
temporal$ t in x {
    ::print "Historical value: @s" (t)
}
# Output:
# Historical value: 30
# Historical value: 20
# Historical value: 10
```

## Built-in Temporal Functions

### Temporal Aggregation

`::temporal_aggregate(variable_name, operation, window_size)`

Performs aggregation operations over a sliding window of temporal history:

**Parameters:**
- `variable_name`: String name of the temporal variable
- `operation`: "sum", "avg", "min", "max"
- `window_size`: Number of historical values to include

**Examples:**
```tesseract
let$sensor := <temp@10>
let$sensor := 100
let$sensor := 105
let$sensor := 110

# Calculate average of last 3 values
::print ::temporal_aggregate("sensor", "avg", 3)  # prints 105

# Sum of last 2 values
::print ::temporal_aggregate("sensor", "sum", 2)  # prints 215

# Min/Max operations
::print ::temporal_aggregate("sensor", "min", 3)  # prints 100
::print ::temporal_aggregate("sensor", "max", 3)  # prints 110
```

### Temporal Pattern Detection

`::temporal_pattern(variable_name, pattern_type, threshold)`

Detects patterns in temporal data:

**Parameters:**
- `variable_name`: String name of the temporal variable
- `pattern_type`: "trend", "cycle", "anomaly"
- `threshold`: Sensitivity threshold for pattern detection

**Pattern Types:**
- `"trend"`: Detects upward (1), downward (-1), or stable (0) trends
- `"cycle"`: Detects cyclical patterns in the data
- `"anomaly"`: Detects anomalous values using z-score analysis

**Examples:**
```tesseract
let$sensor := <temp@10>
let$sensor := 100
let$sensor := 105
let$sensor := 110

# Detect upward trend (threshold 3%)
::print ::temporal_pattern("sensor", "trend", 3.0)  # prints 1 (upward)

# Detect anomalies (z-score threshold 2.0)
::print ::temporal_pattern("sensor", "anomaly", 2.0)  # prints 0 (no anomaly)

# Add anomalous value and test again
let$sensor := 200
::print ::temporal_pattern("sensor", "anomaly", 1.5)  # prints 1 (anomaly detected)
```

## Use Cases

### Moving Averages
```tesseract
let$ price := <temp@5>
let$ price := 100
let$ price := 105
let$ price := 98

func$ moving_average() => {
    let$ sum := 0
    let$ count := 0
    temporal$ p in price {
        let$ sum := sum + p
        let$ count := count + 1
    }
    sum / count
}
```

### State Tracking
```tesseract
let$ state := <temp@10>
let$ state := "idle"
let$ state := "processing"
let$ state := "complete"

# Check if state changed
if$ state@0 != state@1 {
    ::print "State changed from @s to @s" (state@1, state@0)
}
```

### Time Series Analysis
```tesseract
let$ sensor := <temp@100>

func$ detect_trend() => {
    let$ increasing := 0
    let$ decreasing := 0
    
    loop$i := 0 => 4 {
        if$ sensor@i > sensor@(i+1) {
            let$ increasing := increasing + 1
        } else {
            let$ decreasing := decreasing + 1
        }
    }
    
    increasing > decreasing ? "upward" : "downward"
}
```

## Memory Management

- History is automatically managed
- Oldest values are discarded when history limit is reached
- Memory usage is O(N) where N is the history size

## Best Practices

1. **Choose appropriate history size**: Balance memory usage with needed history depth
2. **Initialize before use**: Always assign initial values before accessing history
3. **Check bounds**: Accessing `x@N` where N exceeds history returns undefined behavior
4. **Use temporal loops**: More efficient than manual history iteration

## Advanced Patterns

### Conditional History Access
```tesseract
let$ value := <temp@5>
# ... assignments ...

func$ get_last_valid() => {
    temporal$ v in value {
        if$ v > 0 {
            v  # return first positive historical value
        }
    }
    0  # default if none found
}
```

### History Comparison
```tesseract
func$ is_stable() => {
    let$ stable := true
    loop$i := 0 => 2 {
        if$ value@i != value@(i+1) {
            let$ stable := false
        }
    }
    stable
}
```

Temporal programming in Tesseract enables powerful time-aware applications with minimal syntax overhead.