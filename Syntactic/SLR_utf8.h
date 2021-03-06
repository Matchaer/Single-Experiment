#include<iostream>
#include<fstream>
#include<string>
#include<unordered_map>
#include<unordered_set>
#include<vector>
#include<stdexcept>
#include<iomanip>
#include<stack>
#include<set>
#include<stack>
#include<iterator>
#include<queue>
#include<algorithm>

struct _state {
	//当前状态序号
	int cur_state;
	//当前项目集中的文法
	std::unordered_map<std::string, std::vector<std::vector<std::string>>> production;
	//当前状态集中的FIRST元素
	std::unordered_set<std::string> SFIRST;
	//判断两个集合是否相同
	bool operator ==(const _state& k) const {
		if (production == k.production)
			return true;
		else {
			return false;
		}
	}
};
class Pre_treatment {
	friend class Conduct;
private:
	//FIRST集  ： 一个非终结符 对应一个集合
	std::unordered_map<std::string, std::unordered_set<std::string>> FIRST;
	//FOLLOW集 ： 一个非终结符 对应一个集合
	std::unordered_map<std::string, std::unordered_set<std::string>> FOLLOW;
	//LR(0)项目集： 即和grammar相同的存储文法 但是是个结构体集合
	std::vector<_state> state;
	//状态转移行为： 构表用
	std::unordered_map<int, std::unordered_map<std::string, std::vector<std::string>>> changes;
	//录入的文法
	//对文法做出规定 ： 1. 任意两个符号之间用空格来分开  2.多个产生式后要么为空要么是 | 
	std::unordered_map<std::string, std::vector<std::vector<std::string>>> grammar;
	//非终结符集合
	std::unordered_set <std::string> nonterminal_;
	//终结符集合
	std::unordered_set<std::string> terminal_ = { std::string{"$"} };
	//起始符
	std::string start_;

	//利用文法书写规则分割符号与符号 和 产生式与产生式
	//参数： arg[0]是读到的内容， arg[1]是分隔符 
	std::vector<std::string> Division(std::string, const std::string);
	//去除字符串首尾空格
	std::string Trim_space(const std::string);
	//判断是不是终结符【不是非终结符就是终结符】
	inline bool ifTerminal(std::string);
	//找 first
	//first集需要提供字符，按照  -> 找
	//构造follow集不需要提供字符，直接按照产生式找 
	std::unordered_set<std::string> Search_First(std::string);
	//查看某符号某种集合里是否有空
	bool ifNULL(std::unordered_set<std::string>);
	//查出每个项目集的第一个字符串
	std::unordered_set<std::string> SFIRST(const _state);
	//求项目集的闭包
	void closure(_state&);
	//构造FIRST FOLLOW表
	void Complete_FIRSTable();
	void Complete_FOLLOWTable();
	//构造SLR(1)表
	void Complete_SLRTable();
	
	//输出各个表
	//输出 分好类的 符号表 (起始符、终结符、非终结符、产生式)
	void printSymbols(std::ofstream&);
	//输出 SLR(1)分析表
	void printSLRTable(std::ofstream&);
public:
	//录入文件 ：录入文法 (并处理)
	Pre_treatment(std::ifstream&);
};

class Conduct {
private:
	Pre_treatment machine;
public:
	// 预处理 文法： 形成 LL(1) 分析表
	Conduct(std::ifstream& input): machine(input){}
	// 输入需要处理的文本内容 并 输出分析文件
	void SyntaxParser(std::ifstream&, std::ofstream&);
};