// Example: Convert Mapper_Node txt file to bitstream using libdoda_compiler
#include <iostream>
#include <doda.h>

int main(int argc, char* argv[]) {
    if (argc < 2) {
        std::cerr << "Usage: " << argv[0] << " <input.txt> [output_dir]" << std::endl;
        return 1;
    }

    std::string input_file = argv[1];
    std::string output_dir = (argc >= 3) ? argv[2] : ".";

    std::cout << "Converting " << input_file << " to bitstream..." << std::endl;

    return doda::compileTxtToBitstream(input_file, output_dir) ? 0 : 1;
}
