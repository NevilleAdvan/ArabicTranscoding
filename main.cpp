#include <iostream>
#include <vector>
#include <string>
#include <fribidi/fribidi.h>
#include <locale.h>
#include <algorithm> // 添加reverse函数
// 将UTF-16转换为逻辑顺序的Unicode码点
std::vector<FriBidiChar> process_arabic_bidi(const std::vector<uint16_t>& utf16_input) {
    const size_t len = utf16_input.size();
    std::vector<FriBidiChar> logical(len);

    // 转换到32-bit格式
    for (size_t i=0; i<len; ++i) {
        logical[i] = utf16_input[i];
    }

    // 双向文本处理
    std::vector<FriBidiChar> visual(len);
    FriBidiParType base_dir = FRIBIDI_PAR_RTL;
    fribidi_boolean status = fribidi_log2vis(
        logical.data(), len, &base_dir,
        visual.data(),
        nullptr, nullptr, nullptr
    );

    if (!status) {
        std::cerr << "Bidi processing failed" << std::endl;
        return {};
    }
    std::cout << "before: ";
    for (auto &iter : visual) {
        std::cout << iter << " ";
    }
    std::cout << std::endl;
    if (base_dir == FRIBIDI_PAR_RTL) {
        std::reverse(visual.begin(), visual.end());
    }
    std::cout << "end: ";
    for (auto &iter : visual) {
        std::cout << iter << " ";
    }
    std::cout << std::endl;
    return visual;
}

// Unicode码点转UTF-8
std::string unicode_to_utf8(const std::vector<FriBidiChar>& codepoints) {
    std::string utf8_str;
    for (auto cp : codepoints) {
        if (cp < 0x80) {
            utf8_str += static_cast<char>(cp);
        } else if (cp < 0x800) {
            utf8_str += 0xC0 | (cp >> 6);
            utf8_str += 0x80 | (cp & 0x3F);
        } else if (cp < 0x10000) {
            utf8_str += 0xE0 | (cp >> 12);
            utf8_str += 0x80 | ((cp >> 6) & 0x3F);
            utf8_str += 0x80 | (cp & 0x3F);
        } else {
            utf8_str += 0xF0 | (cp >> 18);
            utf8_str += 0x80 | ((cp >> 12) & 0x3F);
            utf8_str += 0x80 | ((cp >> 6) & 0x3F);
            utf8_str += 0x80 | (cp & 0x3F);
        }
    }
    return utf8_str;
}
// 添加阿拉伯语检测函数
bool is_arabic_char(uint16_t c) {
    return (c >= 0x0600 && c <= 0x06FF) ||   // 基本阿拉伯语区块
           (c >= 0x0750 && c <= 0x077F) ||   // 阿拉伯语补充
           (c >= 0x08A0 && c <= 0x08FF);     // 阿拉伯语扩展A
}
int main() {
    setlocale(LC_ALL, "en_US.utf8"); // 设置控制台为UTF-8模式

    // 阿拉伯语 "مرحبا بالعالم" (Hello World)
    std::vector<uint16_t> input = {
        0x0645, 0x0631, 0x062D, 0x0628, 0x0627, // مرحبا
        0x0020,                                 // 空格
        0x0628, 0x0627, 0x0644, 0x0639, 0x0627, 0x0644, 0x0645 // بالعالم*/
    };

    auto processed = process_arabic_bidi(input);
    std::string utf8_output = unicode_to_utf8(processed);

    // 打印结果
    std::cout << "Processed text: " << utf8_output << std::endl;
    return 0;
}


// مرحباAAABB  بالعالم
// 0x0645, 0x0631, 0x062D, 0x0628, 0x0627 , A ,A ,A ,B ,B , 0x0628, 0x0627, 0x0644, 0x0639, 0x0627, 0x0644, 0x0645

// A_STR1 0x0645, 0x0631, 0x062D, 0x0628, 0x0627
// E_STR :A ,A ,A ,B ,B
// B_str1 0x0628, 0x0627, 0x0644, 0x0639, 0x0627, 0x0644, 0x0645
// A_STR1 E_STR B_str1

// B_str1 E_STR A_STR1

