#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>
#include <iostream>
#include <cstdlib>
#include <string>

int main(int argc, char** argv) {
    if (argc < 4) {
        std::cout << "Usage: texture_pipeline <input.(png/jpg/tga)> <output.(dds/ktx)> <tool>\n";
        std::cout << "tool: nvtt | toktx | compressonator" << std::endl;
        return 1;
    }
    std::string input = argv[1];
    std::string output = argv[2];
    std::string tool = argv[3];

    int w, h, c;
    unsigned char* data = stbi_load(input.c_str(), &w, &h, &c, 0);
    if (!data) {
        std::cerr << "Failed to load " << input << std::endl;
        return 1;
    }
    stbi_image_free(data);

    std::string cmd;
    if (tool == "nvtt") {
        cmd = "nvcompress -bc1 " + input + " " + output;
    } else if (tool == "toktx") {
        cmd = "toktx --t2 " + output + " " + input;
    } else if (tool == "compressonator") {
        cmd = "compressonatorcli -fd BC1 " + input + " " + output;
    } else {
        std::cerr << "Unknown tool: " << tool << std::endl;
        return 1;
    }

    int res = std::system(cmd.c_str());
    if (res != 0) {
        std::cerr << "Compression tool failed" << std::endl;
        return res;
    }

    std::cout << "Converted " << input << " -> " << output << std::endl;
    return 0;
}
