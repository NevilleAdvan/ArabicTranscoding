#include <iostream>
#include <vector>
#include <algorithm>
#include <fribidi/fribidi.h>

// 判断是否为阿拉伯字符（含标点）
bool is_arabic_char(uint16_t c) {
    return (c >= 0x0600 && c <= 0x06FF) ||   // 基本阿拉伯语
           (c >= 0x0750 && c <= 0x077F) ||   // 阿拉伯语补充
           (c >= 0x08A0 && c <= 0x08FF) ||   // 阿拉伯语扩展A
           (c == 0x0020) ||                  // 空格
           (c >= 0xFD50 && c <= 0xFDFF);     // 阿拉伯表现形式A
}

// 处理阿拉伯段落
std::vector<uint16_t> process_arabic_segment(const std::vector<uint16_t>& segment) {
    // 转换为Unicode码点
    std::vector<FriBidiChar> logical;
    for (size_t i = 0; i < segment.size();) {
        if (segment[i] >= 0xD800 && segment[i] <= 0xDBFF) {
            uint32_t high = segment[i++] - 0xD800;
            uint32_t low = segment[i++] - 0xDC00;
            logical.push_back(0x10000 + (high << 10) + low);
        } else {
            logical.push_back(segment[i++]);
        }
    }

    // 双向算法处理
    std::vector<FriBidiChar> visual(logical.size());
    FriBidiParType base_dir = FRIBIDI_PAR_RTL;
    fribidi_log2vis(logical.data(), logical.size(), &base_dir,
                   visual.data(), nullptr, nullptr, nullptr);

    // 转换回UTF-16并反转顺序
    std::vector<uint16_t> processed;
    for (auto cp : visual) {
        if (cp < 0x10000) {
            processed.push_back(cp);
        } else {
            cp -= 0x10000;
            processed.push_back((cp >> 10) + 0xD800);
            processed.push_back((cp & 0x3FF) + 0xDC00);
        }
    }
    std::reverse(processed.begin(), processed.end());
    return processed;
}

// 主处理函数
std::vector<uint16_t> process_mixed_text(const std::vector<uint16_t>& input) {
    std::vector<std::vector<uint16_t>> segments;
    std::vector<uint16_t> current_segment;
    bool current_is_arabic = false;

    // 分段处理
    for (size_t i = 0; i < input.size();) {
        uint16_t c = input[i];
        bool is_arabic = is_arabic_char(c);

        // 处理代理对
        if (c >= 0xD800 && c <= 0xDBFF) {
            if (i+1 < input.size()) {
                uint16_t next = input[i+1];
                is_arabic = is_arabic_char(((c - 0xD800) << 10) + (next - 0xDC00) + 0x10000);
            }
        }

        if (i == 0) {
            current_is_arabic = is_arabic;
        }

        if (is_arabic == current_is_arabic) {
            current_segment.push_back(c);
            if (c >= 0xD800 && c <= 0xDBFF && i+1 < input.size()) {
                current_segment.push_back(input[++i]);
            }
            i++;
        } else {
            segments.push_back(current_segment);
            current_segment.clear();
            current_is_arabic = !current_is_arabic;
        }
    }
    if (!current_segment.empty()) {
        segments.push_back(current_segment);
    }

    // 处理各段 - [修改非阿拉伯段落处理]
    std::vector<uint16_t> output;
    for (auto& seg : segments) {
        if (!seg.empty() && is_arabic_char(seg[0])) {
            auto processed = process_arabic_segment(seg);
            output.insert(output.end(), processed.begin(), processed.end());
        } else {
            // 移除非阿拉伯段落的反转操作
            // std::reverse(seg.begin(), seg.end()); // 移除这行
            output.insert(output.end(), seg.begin(), seg.end());
        }
    }

    return output;
}

// Unicode码点转UTF-8
std::string unicode_to_utf8(const std::vector<uint16_t>& codepoints) {
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

int main() {
    // 混合文本测试："Helloمرحبا123世界"
    std::vector<uint16_t> input = {
        /* Hello */ 0x0048, 0x0065, 0x006C, 0x006C, 0x006F,
        /* مرحبا */ 0x0645, 0x0631, 0x062D, 0x0628, 0x0627,
        /* 123 */ 0x0031, 0x0032, 0x0033,
        /* 世界 */ 0x4E16, 0x754C
    };

    auto output = process_mixed_text(input);

    // 输出验证
    std::cout << "Processed UTF-16 codes:\n";
    for (auto c : output) {
        printf("%04X ", c);
    }
    std::string utf8_output = unicode_to_utf8(output);

    // 打印结果
    std::cout << "Processed text: " << utf8_output << std::endl;

    std::vector<uint16_t> input2 = {
        0x0645, 0x0631, 0x062D, 0x0628, 0x0627, // مرحبا
        0x0020,                                 // 空格
        0x0628, 0x0627, 0x0644, 0x0639, 0x0627, 0x0644, 0x0645, // بالعالم
        0x0048, 0x0065, 0x006C, 0x006C, 0x006F,
    };
    auto output2 = process_mixed_text(input2);

    // 输出验证
    std::cout << "Processed UTF-16 codes:\n";
    for (auto c : output2) {
        printf("%04X ", c);
    }
    std::string utf8_output2 = unicode_to_utf8(output2);

    // 打印结果
    std::cout << "Processed text: " << utf8_output2 << std::endl;
    return 0;
}
