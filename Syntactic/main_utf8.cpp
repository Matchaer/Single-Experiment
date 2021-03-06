#include"SLR.h"
//利用main进行命令行参数配置
// 第一个参数为语法生成式文件名
// 第二个参数为输入文件名
// 第三个参数为输出结果文件名
int main(int argc, char* argv[]) {
	try{
		if (argc < 3)
			throw std::runtime_error("Too few arguments");

		std::ifstream grammar(argv[1]);
		std::ifstream input(argv[2]);
		std::ofstream result(argv[3]);

		if (! grammar.is_open())
			throw std::runtime_error(std::string("Cannot open: ") + argv[1]);
		if (! input.is_open())
			throw std::runtime_error(std::string("Cannot open: ") + argv[2]);
		if (! result.is_open())
			throw std::runtime_error(std::string("Cannot open: ") + argv[3]);

		Conduct process_(grammar);
		process_.SyntaxParser(input, result);
	}
	catch (const std::exception & e){
		std::cout << e.what() << std::endl;
		return 1;
	}

	return 0;
}