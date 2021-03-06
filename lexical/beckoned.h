#include <iostream>
#include<string>
#include <vector>
#include <set>
#include <cctype>
#include <fstream>
#include <utility>
#include <stdexcept>

typedef int DF_Size;

enum class STATE {
	//正常读取
	NORMAL = 1,
	//注释行 ( // )
	Line_COMMENT,
	//注释块 (/* */)
	Block_COMMENT,
	//预处理指令
	PRE_TREATMENT
};

class LexicalAnalysis {

private:
	//输入文件
	std::ifstream& file;
	//文件行数
	DF_Size rows;
	//文件总字符数
	DF_Size C_count;
	//识别当前状态
	STATE cur_state;

	//标识符表(去重)
	std::set<std::string> IdTable;

	//记号表<符号， 属性>
	//为了使用vector的 emplace_back  (右值)
	std::vector<std::pair<std::string, std::string>> TokenTable;
	//注释
	std::vector<std::string> comments;
	//错误
	std::vector<std::string> errors;

	//保留字集合
	//set可以保证 查询效率
	static const std::set<std::string> Reserved_words;
	//保留符号集合
	static const std::set<std::string> OPs;


	//封装基础功能的函数
	//只在public里使用

	//获取字符： 指针后移
	char GetC();
	//回退： 指针前移
	void Rollback();
	//读取
	void READ(char&, std::string&);





public:
	//初始化
	LexicalAnalysis(std::ifstream& infile) : file(infile), rows(1), C_count(0), cur_state(STATE::NORMAL) {}
	//析构
	~LexicalAnalysis() {}

	//主要程序：
	//根据读取的字符进行 STATE 转移
	//判断是否结束 / 出错
	bool transfer_go();

	//打印需求
	DF_Size GetR()const;
	DF_Size GetCC()const;
	const std::set<std::string> GetIdT()const;
	const std::vector<std::pair<std::string, std::string>> GetTT()const;
	const std::vector<std::string> GetCM()const;
	const std::vector<std::string> GetER()const;

};






#pragma once
