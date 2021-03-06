#include"beckoned.h"
#include<map>
//资料
const std::set<std::string> LexicalAnalysis::Reserved_words =
{ "auto", "break", "case", "char", "const", "continue", "default",
		"do", "double", "else", "main", "enum", "extern", "float", "for",
		"goto", "if", "int", "long", "register", "return", "short", "signed",
		"sizeof", "static", "struct", "switch", "typedef", "union", "unsigned",
		"void", "volatile", "while"
};

const std::set<std::string> LexicalAnalysis::OPs = {
	"+", "-", "*", "/", "^", "|", "&", "(", ")",
	";", "{", "}", "[", "]", ",",
	":", "?", ".", "%", "~"
};
int main() {
	//输入文件
	std::string filename = "";
	std::cout << "Please input filename: ";
	std::cin >> filename;
	std::ifstream file(filename, std::ios::in);
	//统计各类单词个数
	std::map<std::string, int> sizes;

	if (file.fail())
		std::cout << "The file not exist" << std::endl;
	else
	{
		LexicalAnalysis analysis(file);
		std::ofstream outfile("result.txt");

		if (! analysis.transfer_go())
		{
			outfile << "[FATEL ERROR] Something wrong with the program" << std::endl;
		}
		//输出统计信息
		outfile << "Total Lines: " << analysis.GetR()<< std::endl;
		outfile << "Total Words: " << analysis.GetTT().size() << std::endl;
		outfile << "Total Chars: " << analysis.GetCC() << std::endl;
		outfile << std::endl;
		//输出记号表
		outfile << "Token List:" << std::endl;
		auto table = analysis.GetTT();
		for (const auto& pair : table)
		{
			outfile << "< " << pair.first << " , " << pair.second << " >" << std::endl;
			sizes[pair.second] ++;

		}
		outfile << "The TokensTable has " << table.size() << " items in total. " << std::endl;
		//输出各类单词数
		for (const auto& pair : sizes) {
			outfile << "\" " << pair.first << " \"" << "\tin total : " << pair.second << std::endl;
		}

		outfile << std::endl;
		//输出注释
		auto comments = analysis.GetCM();
		outfile << "Comment List: " << comments.size() << " in total" << std::endl;
		for (const auto& comment : comments)
		{
			outfile << "[Comment] : " << comment << std::endl;
		}
		//输出错误
		auto errors = analysis.GetER();
		if (errors.size() != 0)
		{
			outfile << "Error List:" << std::endl;
			for (const auto& str : errors)
			{
				outfile << "[Error] " << str << std::endl;
			}
			outfile << std::endl;
		}

		//关闭文件
		outfile.close();
	}
	file.close();
	return 0;

}
