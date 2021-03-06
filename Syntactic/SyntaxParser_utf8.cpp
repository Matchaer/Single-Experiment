#include"SLR.h"

const std::string empty = "empty";

Pre_treatment::Pre_treatment(std::ifstream& in_grammar){
	std::string file_in = "";
	int row = 0;
	while (getline(in_grammar, file_in)) {
		row ++;
		//分出非终结符和产生式：两部分
		auto Parts = Division(file_in, "->");
		//如果是上下文有关文法： 报错
		//如果左侧无符号： 报错
		if (Parts[0].find(" ") != std::string::npos) {
			throw std::runtime_error("Too much symbols in the left In the line " + std::to_string(row));
		}
		if (Parts.size() < 2) {
			throw std::runtime_error("Illegal grammar In the line " + std::to_string(row));
		}
		//默认第一行 -> 左侧是起始符
		if (1 == row) {
			start_ = Parts[0];
			//拓广文法
			std::vector<std::string> temps;
			temps.emplace_back(".");
			temps.emplace_back(start_);
			grammar["S"].emplace_back(std::move(temps));
			nonterminal_.emplace("S");
			start_ = "S";
			FOLLOW[start_].emplace(std::string{ "$" });
		}
		//把非终结符加到表里
		nonterminal_.emplace(Parts[0]);
		//处理剩下的产生式
		//把每个符号都拆分存储：便于生成FIRST和FOLLOW集
		auto production = Division(Parts[1], "|");
		std::vector<std::vector<std::string>> cur_pd;
		for (auto iter : production) {
			std::vector<std::string> temp, ttemp;
			temp.emplace_back(".");
			ttemp = Division(iter, " ");
			temp.insert(temp.end(), ttemp.begin(), ttemp.end());
			cur_pd.emplace_back(std::move(temp));
		}
		//加入到grammar里
		grammar[Parts[0]] = cur_pd;
	}

	Complete_SLRTable();
}

bool Pre_treatment::ifNULL(std::unordered_set<std::string> xxstring) {
	for (auto temp : xxstring) {
		if (temp == empty)
			return true;
	}
	return false;
}

void Pre_treatment::Complete_FIRSTable() {
	//为非终结符构造first集
	for (auto temp : nonterminal_) {
		auto _FIRST = Search_First(temp);
		auto find = FIRST.find(temp);
		if (find == FIRST.end())
			FIRST[temp] = _FIRST;
		else FIRST[temp].insert(_FIRST.begin(), _FIRST.end());
	}
	//添加终结符
	for (auto temp : terminal_) {
		FIRST[temp] = Search_First(temp);
	}
}

void Pre_treatment::Complete_FOLLOWTable() {
	//lamda函数
	auto addFollow = [&](std::string state, std::unordered_set<std::string>& FollowSet)
	{
		auto pos = FOLLOW.find(state);
		//找到该非终结符
		if (pos != FOLLOW.end())
			FOLLOW[state].insert(FollowSet.begin(), FollowSet.end());
		//没找到该非终结符
		else
			FOLLOW[state] = std::move(FollowSet);
	};

	for (const auto& none_ : nonterminal_)
	{
		for (const auto& rule : grammar.find(none_)->second)
		{
			//第一个是 对于分析文法无意义的   .
			for (auto iter = rule.begin() + 1; iter != rule.end(); iter++)
			{
				auto symbol = *iter;

				if (! ifTerminal(symbol))
				{   
					//如果该非终结符符号后面还有 符号
					if ((iter + 1) != rule.end())
					{
						if (ifTerminal(*(iter + 1))) {
							if (FIRST.find(*(iter + 1)) == FIRST.end()) {
								FIRST[*(iter + 1)].emplace(*(iter + 1));
							}
						}
						auto first = FIRST.find(*(iter + 1))->second;
						//且后面紧跟的字符可以为空
						if (ifNULL(first))
						{
							//左侧非终结符的follow 集 要加到 该非终结符的follow中
							auto follow = FOLLOW.find(none_)->second; 
							addFollow(symbol, follow);
						}
						//除去first中的空
						first.erase(empty);  
						//加入到follow集里
						addFollow(symbol, first);  
					}
					//如果已经到这个生成式的最后 且 是非终结符 则把 左侧非终结符的follow集 加到 该非终结符的 follow集
					else
					{
						auto follow = FOLLOW.find(none_)->second;
						addFollow(symbol, follow);
					}
				}
			}
		}
	}
}

inline bool Pre_treatment::ifTerminal(std::string xstring) {
	if (nonterminal_.find(xstring) == nonterminal_.end() || xstring == empty) {
		terminal_.emplace(xstring);
		return true;
	}
	return false;
}

void Pre_treatment::printSLRTable(std::ofstream& os) {
	os << "SLR(1) Table: " << std::endl;
	os << std::string(138, '-') << std::endl;
	os << "| " << std::setw(14) << "States:";
	for (const auto terminals : terminal_) {
		if(terminals != empty)
		os << std::setw(10) << terminals;
	}
	for (const auto terminals : nonterminal_) {
		os << std::setw(10) << terminals;
	}
		
	os << " |" << std::endl;
	for(int i = 0; i < changes.size(); i++){
		os << "| " << std::setw(14) << i;
		for (const auto terminals : terminal_)
		{
			if (terminals != empty) {
				auto pos = changes[i].find(terminals);

				if (pos != changes[i].end())
				{
					std::string str;
					for (const auto vec : pos->second) {
						str += vec;
					}

					os << std::setw(10) << str;
				}
				else
					os << std::setw(10) << " ";
			}
		}
		for (const auto ts : nonterminal_)
		{
			if (ts != empty) {
				auto pos = changes[i].find(ts);

				if (pos != changes[i].end())
				{
					std::string str;
					for (const auto vec : pos->second) {
						str += vec;
					}

					os << std::setw(10) << str;
				}
				else
					os << std::setw(10) << " ";
			}
		}
		os << " |" << std::endl;
	}

	os << std::string(138, '-') << std::endl;
}

void Pre_treatment::printSymbols(std::ofstream& os) {
	os << "Start character :" <<std::endl;
	os << '\t' << start_ << std::endl;
	os << "Non-Terminal Symbols:" << std::endl << '\t';
	for (const auto& symbol : nonterminal_)
		os << symbol << ' ';
	os << std::endl << "Terminal Symbols:" << std::endl << '\t';
	for (const auto& symbol : terminal_)
		os << symbol << ' ';
	os << std::endl << "Grammars:" << std::endl;

	for (const auto pair : grammar)
	{
		os << '\t' << pair.first << " -> ";
		for (const auto rule : pair.second)
		{
			for (const auto symbol : rule) {
				if(symbol != ".") 
					os << symbol << ' ';
			}	
			if(rule != *pair.second.rbegin())
					os << "| ";
		}
		os << std::endl;
	}
}

std::string Pre_treatment::Trim_space(const std::string xstring){
	auto begins = xstring.find_first_not_of(' ');
	auto ends = xstring.find_last_not_of(' ');
	return xstring.substr(begins, ends - begins + 1);
}

std::vector<std::string> Pre_treatment::Division(std::string xstring, const std::string depart) {
	
	std::vector<std::string> temp;
	std::string newstring = xstring + depart;
	size_t position = 0;
	while ((position = newstring.find(depart, 0)) != std::string::npos) {
		//先除空格
		temp.emplace_back(Trim_space(newstring.substr(0, position)));
		//pos移动
		position += depart.size();
		//清除空格
		while (newstring[position] == ' ') {
			position++;
		}
		//清除xstring每个分隔符（以及空格）前的字符串
		newstring.erase(0, position);
	}
	return std::move(temp);
}

std::unordered_set<std::string> Pre_treatment::Search_First(std::string xstring) {
	decltype(FIRST.begin()->second) first;
	//如果是终结符 返回它本身
	if (ifTerminal(xstring)) {
		return std::unordered_set<std::string>{ xstring };
	}
	//非终结符
	else {
		//找产生式 递归
		auto prod = grammar[xstring]; //返回产生式 vector<vector<string>>
		for (auto iter = prod.begin(); iter < prod.end(); iter++) {
			auto null_mark = false;
			//跳过第一个  .
			for (auto iiter = iter->begin() + 1; iiter < iter->end(); iiter ++) {
				//首符是终结符直接加 直接退循环
				if (ifTerminal(*iiter)) {
					null_mark = true;
					first.emplace(*iiter);
					break;
				}
				//首符是非终结符 递归找
				else {
					if (xstring != *iiter) {
						auto _first = Search_First(*iiter);
						//如果可以为空 需要剔除空符 继续找
						if (ifNULL(_first)) {
							_first.erase(empty);
							first.insert(_first.begin(), _first.end());
						}
						//如果不能为空 则直接加并退循环
						else {
							null_mark = true;
							first.insert(_first.begin(), _first.end());
							break;
						}
					}
					
				}
			}
			//右侧产生式可全为空
			if (! null_mark) {
				first.emplace(empty);
			}
		}
	}
	return first;
}

void Pre_treatment::Complete_SLRTable() {
	//为SLR准备FOLLOW集
	//先构造FIRST集
	decltype(FIRST) first1;
	//不断循环直到集合不再增大
	do {
		first1 = FIRST;
		Complete_FIRSTable();
	} while (first1 != FIRST);

	decltype(FOLLOW) follow1;
	//不断循环直到集合不再增大
	do {
		follow1 = FOLLOW;
		Complete_FOLLOWTable();
	} while (follow1 != FOLLOW);

	//构建LR(0)项目集
	std::queue<_state> states;
	//将S->E加入到state[0]中
	_state initial;
	int state_num = 0;
	initial.cur_state = 0;
	initial.production[grammar.find(start_)->first] = grammar.find(start_)->second;
	states.emplace(initial);

	while (! states.empty()) {
		_state cur = states.front();
		_state other;
		do {
			other = cur;
			closure(cur);
		} while (!(other == cur));
		//在这里自查 有没有重复的 ： 改一下state_num
		auto multi = find(state.begin(), state.end(), cur);
		//如果有重复的 则值修改状态表并不进行添加和扩展过程
		if (multi != state.end()) {
			//遍历 状态表
			for (auto& ii : changes) {
				auto& iss = ii.second;
				for (auto& itt : iss) {
					std::vector<std::string>& seeks = itt.second;
					if (seeks[0] == "S" && std::stoi(seeks[1]) == cur.cur_state) {
						seeks[1] = std::to_string(multi->cur_state);
					}
				}
			}
		}
		//如果是新项目集 开始进行添加和扩展
		else {
			for (auto& ii : changes) {
				auto& iss = ii.second;
				for (auto& itt : iss) {
					std::vector<std::string>& seeks = itt.second;
					if (seeks[0] == "S" && std::stoi(seeks[1]) == cur.cur_state) {
						seeks[1] = std::to_string(state.size());
					}
				}
			}
			cur.cur_state = state.size();
			state.push_back(cur);
			//根据  first对项目进行外扩：
			//每个符号对应一个新状态
			if (! cur.SFIRST.empty()) {
				for (auto xstring : cur.SFIRST) {
					//如果匹配就 swap两个元素 一下 然后存到新的状态里
					_state next;
					int summarize = 0;
					for (auto temp : cur.production) {
						auto ttemp = temp.second;
						for (auto xxstring : ttemp) {
							//查看各个产生式中  . 的位置
							auto ustring = find(xxstring.begin(), xxstring.end(), ".");
							//点不在最后一个 且 后面有符号
							if ((ustring + 1) < xxstring.end()) {
								ustring++;
								//匹配
								if (*ustring == xstring) {
									//交换 . 和 它后面字符的位置
									swap(*(ustring - 1), *(ustring));
									//将新文法加入到下一个 项目集中
									next.production[temp.first].emplace_back(xxstring);
								}
								
							}
							// . 出现在产生式最后的位置 要进行规约
							else {
								summarize++;
								if (temp.first == "S" && changes[cur.cur_state]["$"].empty()) {
									changes[cur.cur_state]["$"].emplace_back(std::string{ "acc" });
								}
								else {
									for (auto temps : FOLLOW[temp.first]) {
										if (changes[cur.cur_state][temps].empty()) {
											changes[cur.cur_state][temps].emplace_back("R");
											changes[cur.cur_state][temps].emplace_back(temp.first);
											changes[cur.cur_state][temps].emplace_back("->");
											for (auto _temp = xxstring.begin(); _temp < ustring; _temp++)
												changes[cur.cur_state][temps].emplace_back(*_temp);
										}
										
									}
								}

							}
						}
					}
					//如果产生式不为空则说明 存在文法 中 . 的位置不是最后一个 可以往后延伸
					if (!next.production.empty()) {
						state_num++;
						//加入到表中
						changes[cur.cur_state][xstring].emplace_back("S");
						changes[cur.cur_state][xstring].emplace_back(std::to_string(state_num));
						next.cur_state = state_num;
						states.push(next);
					}
					//FIRST集为空 表示当前状态下的产生式均规约
					//如果一个非终止状态下有不只一个表达式则冲突
					//如果两个非终止状态的FOLLOW集有交集则也冲突 
					
					if (summarize >= 2)
						throw std::runtime_error("Conflict! It's not SLR(1) grammar");
				}
			}
			else {
				if (cur.production.begin()->second.size() >= 2)
					throw std::runtime_error("Conflict! It's not SLR(1) grammar");
				else if (cur.production.size() >= 2) {
					std::unordered_set<std::string> ssset, newset;
					for (auto temp = cur.production.begin(); temp != cur.production.end(); temp++) {
						ssset.emplace(temp->first);
					}
					for (auto wap = ssset.begin(); wap != ssset.end(); wap++) {
						std::unordered_set<std::string> ssse;
						std::set_intersection(FOLLOW[*wap].begin(), FOLLOW[*wap].end(), FOLLOW[*(wap++)].begin(), FOLLOW[*(wap++)].end(), inserter(ssse, ssse.begin()));
						newset.insert(ssse.begin(), ssse.end());
					}
					if (!newset.empty())
						throw std::runtime_error("Conflict! It's not SLR(1) grammar");
				}
				for (auto temp : cur.production) {
					auto opps = temp.second;
					for (auto temps : FOLLOW[temp.first]) {
						changes[cur.cur_state][temps].emplace_back("R");
						changes[cur.cur_state][temps].emplace_back(temp.first);
						changes[cur.cur_state][temps].emplace_back("->");
						for (auto temo : opps) {
							for (auto ztemo : temo) {
								if (ztemo != ".") changes[cur.cur_state][temps].emplace_back(ztemo);
							}
						}
					}
				}
			}
		}
			
		states.pop();
	}
	
}

void Pre_treatment::closure(_state& one) {
	one.SFIRST = SFIRST(one);
	if (!one.SFIRST.empty()) {
		for (auto temp : one.SFIRST) {
			//如果是非终结符 则开始closure
			if (!ifTerminal(temp)) {
				//查找该字符的grammar 不需要去掉 .  直接全加
				if (one.production.find(temp) == one.production.end())
					one.production.insert(*grammar.find(temp));
				else{
					bool ifcan = false;
					for (auto ttemp : grammar[temp]) {
						if (find(one.production[temp].begin(), one.production[temp].end(), ttemp) != one.production[temp].end()) {
							ifcan = true;
							break;
						}	
					}
						if(! ifcan)
						one.production[temp].insert(one.production[temp].end(), grammar.find(temp)->second.begin(), grammar.find(temp)->second.end());
				}
					
			}
		}
	}
	
}

std::unordered_set<std::string> Pre_treatment::SFIRST(const _state one) {
	//找到文法   . 后  第一个字符
	//如果 . 在最后？？？ 指针容易越界
	std::unordered_set<std::string> sset;
	for (auto iter = one.production.begin(); iter != one.production.end(); iter ++) {
		auto vec = iter->second;
		for (auto temp : vec) {
			auto ttemp = find(temp.begin(), temp.end(), ".");
			if(ttemp + 1 < temp.end())
				sset.emplace(*(ttemp+1));
		}
	}
	return std::move(sset);
}


void Conduct::SyntaxParser(std::ifstream& instance, std::ofstream& os) {
	std::string in_symbol;
	//打印两个表
	machine.printSymbols(os);
	machine.printSLRTable(os);

	//初始化三个栈(状态、输入和文法)
	//状态栈的串
	std::string stateStr = "0";
	//状态栈
	std::stack<int> statestack;
	statestack.push(0);
	//文法栈里的串 (对比识别)（输出）
	std::string stackStr = "$";
	//文法栈(倒序)
	std::stack<std::string> stack;
	stack.push("$");

	os << "Process:" << std::endl;
	//输入的串(正序)
	std::vector<std::string> input;
	//输出使用
	std::string inputStr;
	while (instance >> in_symbol)
	{
		inputStr += in_symbol;
		input.emplace_back(in_symbol);
	}

	inputStr += '$';
	input.emplace_back(std::string{ '$' });

	//开始识别
	bool successful = true;
	for (const auto& symbol : input)
	{
		while (1) {
			os << std::setiosflags(std::ios::left) << std::setw(25) << stateStr << std::endl;
			os << std::setiosflags(std::ios::left) << std::setw(25) << stackStr;
			os << std::setiosflags(std::ios::left) << std::setw(25) << inputStr;
			//根据当前栈顶状态和字符串首字符判断 S? R?
			auto newstate = machine.changes[statestack.top()].find(symbol);
			//如果字符语法正确 有识别信息
			if (newstate != machine.changes[statestack.top()].end()) {
				//如果是转移 则将字符和新状态入栈
				//输入字符缩短
				if (newstate->second[0] == "S") {
					statestack.push(std::stoi(newstate->second[1]));
					stateStr += " ";
					stateStr += newstate->second[1];
					stack.push(symbol);
					stackStr += " ";
					stackStr += symbol;
					inputStr = inputStr.substr(symbol.size(), inputStr.size() - symbol.size());
					os << newstate->second[0] << newstate->second[1] << std::endl;
					break;
				}
				//如果是规约 则把 规约的字符和对应的状态都 弹掉 并查找+添加 栈顶状态与栈顶字符 对应的状态 
				else if (newstate->second[0] == "R") {
					os << newstate->second[1] << newstate->second[2];
					for (auto iter = find(newstate->second.begin(), newstate->second.end(), "->") + 1; iter < newstate->second.end(); iter++) {
						//记得还有个空格
						stateStr = stateStr.substr(0, stateStr.size() - std::to_string(statestack.top()).size() - 1);
						stackStr = stackStr.substr(0, stackStr.size() - stack.top().size() - 1);
						statestack.pop();
						stack.pop();
						os << *iter;
					}
					os << std::endl;
					stack.push(newstate->second[1]);
					stackStr += " ";
					stackStr += newstate->second[1];
					stateStr += " ";
					stateStr += machine.changes[statestack.top()].find(newstate->second[1])->second[1];
					statestack.push(std::stoi(machine.changes[statestack.top()].find(newstate->second[1])->second[1]));
					continue;
				}
				else if (newstate->second[0] == "acc") {
					os << "acc" << std::endl;
					break;
				}
			}
			else {
				os << "ERROR ! STOP !" << std::endl;
				successful = false;
				break;
			}
			break;
		}
		if (!successful) break;
	}
	
}