# ArabicTranscoding
# Mixed Text Bidirectional Processing

A C++ implementation for handling mixed-direction text (Arabic RTL with LTR languages) using FriBidi library.

## Features
- 🕋 Arabic text segmentation & bidirectional processing
- 🌐 Supports mixed scripts:
    - Arabic (RTL)
    - English (LTR)
    - Numbers (LTR)
    - Chinese/Japanese (LTR)
- 🔄 Automatic text segmentation by script type
- 🛠 UTF-16 surrogate pair handling
- 📜 Bidirectional algorithm implementation

## Requirements
- C++11 compiler
- FriBidi library (v1.0.10+)
- CMake (for building)

## Build & Run
```bash
# Install dependencies
sudo apt-get install libfribidi-dev cmake

# Build
mkdir build && cd build
cmake ..
make

# Run with test cases
./FribidiTest
```

API Usage
Core Functions
```[C++]
// Process mixed-direction text
std::vector&lt;uint16_t&gt; process_mixed_text(const std::vector&lt;uint16_t&gt;& input);

// Arabic segment processor
std::vector&lt;uint16_t&gt; process_arabic_segment(const std::vector&lt;uint16_t&gt;& segment);

// Script detection
bool is_arabic_char(uint16_t c);
```

Input Format
UTF-16 encoded text segments:

```[C++]
std::vector&lt;uint16_t&gt; input = {
    0x0048, 0x0065, 0x006C, 0x006C, 0x006F,  // Hello
    0x0645, 0x0631, 0x062D, 0x0628, 0x0627,  // مرحبا
    0x0031, 0x0032, 0x0033,                 // 123
    0x4E16, 0x754C                           // 世界
};
```
Output Handling
```[C++]
// Convert processed UTF-16 to UTF-8
std::string utf8_output = unicode_to_utf8(output);
std::cout &lt;&lt; "Processed text: " &lt;&lt; utf8_output &lt;&lt; std::endl;
```

Test Cases
Case 1: "Helloمرحبا123世界"
Input Segments:
- LTR (Hello)
- RTL (مرحبا)
- LTR (123)
- LTR (世界)

Processing Flow:
1. Keep English "Hello" in original order
2. Apply FriBidi RTL processing to "مرحبا"
3. Maintain numeric "123" sequence
4. Preserve Chinese "世界" order

Output: "Helloابحرم123世界"
Case 2: "مرحبا بالعالم Hello"
Input Segments:
- RTL (مرحبا)
- RTL (بالعالم)
- LTR (Hello)

Processing Result:
1. Process Arabic segments with RTL rules
2. Maintain English order

Output: "Helloابحرم لامجلاوب"