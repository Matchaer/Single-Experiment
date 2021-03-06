#include"beckoned.h"

bool LexicalAnalysis::transfer_go() {
	//声明变量以及初始化句柄
	char varcc;
	std::string sstring = "";

	//读文件前的测试
	//读到最后
	if (EOF == GetC()) {
		rows = 0;
		//返回true： 已完成
		return true;

	}
	//若当前文件指针不在开始位置
	//返回开始位置
	else {
		file.clear();
		file.seekg(0, std::ios::beg);
	}

	//开始读
	while ((varcc = GetC()) != EOF) {
		switch (cur_state)
		{
		case STATE::PRE_TREATMENT:
			if ('\n' == varcc)
				cur_state = STATE::NORMAL;
			break;
		case STATE::NORMAL:
			//利用try-catch结构可以捕捉到 error type
			try {
				sstring.clear();
				READ(varcc, sstring);
				sstring.clear();
			}
			catch(const std::exception& err){
				errors.emplace_back(err.what());
			}
			break;
		case STATE::Line_COMMENT:
			//换行符为结尾
			if ('\n' == varcc) {
				comments.emplace_back(sstring);
				sstring.clear();
				cur_state = STATE::NORMAL;
				break;
			}
			sstring += varcc;
			break;
		case STATE::Block_COMMENT:
			//  出现下一个* 即结尾
			if ('*' == varcc){
				if ('/' == GetC()) {
					comments.emplace_back(sstring);
					sstring.clear();
					cur_state = STATE::NORMAL;
					break;
				}
				else {
					Rollback();
				}
			}
			sstring += varcc;
			break;
		default:
			break;
		}
	}
}

void LexicalAnalysis::READ(char& varcc, std::string& tokens) {

	//如何读

	//预处理指令
	if ('#' == varcc) {
		cur_state = STATE::PRE_TREATMENT;
	}
	//有理数(会涉及到这些符号)
	else if (isdigit(varcc) || '+' == varcc || '-' == varcc || '.' == varcc) {

		tokens += varcc;
		DF_Size state = 0;
		DF_Size trigger = 0;

		//判断是单独的 + - 号 \ 组合算符(+= -= ++ --) <直接退出> 还是和数字组合的 类似 +.99 或 -.99 这类的
		if ('+' == varcc || '-' == varcc) {
			char temp = GetC();
			//一定是单独 + - 号 
			if (isalpha(temp)) {
				TokenTable.emplace_back(std::make_pair(tokens, "-"));
				return;
			}
			else if (varcc == temp || temp == '=') {
				TokenTable.emplace_back(std::make_pair(tokens + temp, "-"));
				return;
			}
			else if ('-' == varcc && '>' == temp) {
				TokenTable.emplace_back(std::make_pair(tokens + temp, "-"));
				return;
			}
			else {
				Rollback();
			}

		}

		state = (varcc == '.') ? 2 : 1;

		//有理数 <根据书上的自动机>
		while (!trigger) {

			varcc = GetC();

			switch (state)
			{
				//数字
			case 1:
				if (isdigit(varcc))
					break;
				//有可能是变参宏 或 小数
				else if ('.' == varcc) {
					state = 2;
				}
				else if ('e' == varcc || 'E' == varcc) {
					state = 4;
				}
				else if(isalpha(varcc)){
					throw std::runtime_error("Row " + std::to_string(rows) + " ( " + tokens + varcc + " ) " +
						":unexpected numeric constant");
				}
				else {
					trigger = 1;
				}
				break;

			case 2:
				if (isdigit(varcc)) {
					state = 3;
				}
				else if ('.' == varcc) {
					state = 7;
				}
				else if(isalpha(varcc)){
					throw std::runtime_error("Row " + std::to_string(rows) + " ( " + tokens + varcc + " ) " +
						": unexpected numeric constant");
				}
				else {
					trigger = 1;
				}
				break;

			case 3:
				if (isdigit(varcc))
					break;
				else if ('E' == varcc || 'e' == varcc)
					state = 4;
				else if (isalpha(varcc)) {
					throw std::runtime_error("Row " + std::to_string(rows) + " ( " + tokens + varcc + " ) " +
						": unexpected numeric constant");
				}
				else {
					trigger = 1;
				}
				break;

			case 4:
				if (isdigit(varcc))
					state = 6;
				else if ('+' == varcc || '-' == varcc)
					state = 5;
				else {
					throw std::runtime_error("Row " + std::to_string(rows) + " ( " + tokens + varcc  + " ) " +
						": unexpected numeric constant");
				}
				break;

			case 5:
				if (isdigit(varcc))
					state = 6;
				else {
					throw std::runtime_error("Row " + std::to_string(rows) + " ( " + tokens + varcc  + " ) "+ 
						": Bad Exponential");
				}
				break;

			case 6:
				if (isdigit(varcc))
					break;
				//变参宏
				else if (isalpha(varcc)) {
					throw std::runtime_error("Row " + std::to_string(rows) +" ( "+ tokens + varcc + " ) "+
						": Bad Exponential");
				}
				else
					trigger = 1;
				break;

			case 7:
				//变参宏
				if ('.' == varcc) {
					state = 0;
				}
					
				else
					throw std::runtime_error("Row " + std::to_string(rows) +
						": Syntax error " +" ( " +tokens + varcc + " ) ");
				break;

			default:
				trigger = 1;
				break;
			}
			if (trigger)
				Rollback();
			else tokens += varcc;
		}

		if (0 == state)
			TokenTable.emplace_back(std::make_pair(tokens, std::string("-")));
		else TokenTable.emplace_back(std::make_pair(tokens, std::string("num")));

	}
	//识别字符串
	else if (varcc == '\"') {
		while ((varcc = GetC()) != '\"') {

			if ('\n' == varcc) {
				throw std::runtime_error("Row " + std::to_string(rows - 1) +
					": missing closing quotation mark \" character");
			}
			//防止 错误 识别 末尾右引号
			if ('\\' == varcc) {
				char temp = GetC();
				if (temp == '"' || temp == '\'') {
					tokens += temp;
					continue;
				}
				else if (isalpha(temp)) {
					TokenTable.emplace_back(std::make_pair(std::string(1, varcc) + temp, std::string("-")));
					continue;
				}
				else {
					Rollback();
				}

			}

			tokens += varcc;
		}
			TokenTable.emplace_back(std::make_pair(tokens, std::string("string")));
	}
	//跳过空格、换行、制表符
	else if ('\n' == varcc || '\t' == varcc || ' ' == varcc) {}
	//处理标识符
	else if ('_' == varcc || isalpha(varcc)) {
		tokens += varcc;
		varcc = GetC();
		while (isalpha(varcc) || isdigit(varcc) || '_' == varcc)
		{
			tokens += varcc;
			varcc = GetC();
		}
		//注意上面循环结尾后 仍然挪动了指针
		Rollback();
		if (Reserved_words.find(tokens) != Reserved_words.end()) {
			TokenTable.emplace_back(std::make_pair(tokens, std::string( "-")));
		}
		else {
			IdTable.insert(tokens);
			TokenTable.emplace_back(std::make_pair(tokens, std::string("id")));
		}
	}
	//识别字符
	else if ('\'' == varcc) {
		varcc = GetC();
		//有问题的
		if ('\'' == varcc) {
			throw std::runtime_error("Row " + std::to_string(rows) +
				": Too few arguments for char const");
		}
		//转义字符
		else if ('\\' == varcc) {
			char temp = GetC();
			tokens = "\\" + temp;
		}
		else if ('\n' == varcc) {
			throw std::runtime_error("Row " + std::to_string(rows - 1) + ": missing terminating ' character");
		}
		else  tokens += varcc;

		if (GetC() == '\'') {
			TokenTable.emplace_back(std::make_pair(tokens, std::string("-")));
		}
		else {
			Rollback();
			throw std::runtime_error("Row " + std::to_string(rows) + ": missing closing ' character");
		}
	}
	//识别关系运算符
	else if ('<' == varcc) {
		char temp = GetC();
		switch (temp)
		{
		case '=':
			TokenTable.emplace_back(std::make_pair(std::string("relop"), std::string("LE")));
			break;
		case '<':
			TokenTable.emplace_back(std::make_pair(std::string("<<"), std::string("-")));
			break;
		default:
			TokenTable.emplace_back(std::make_pair(std::string("relop"), std::string("LT")));
			Rollback();
			break;
		}
	}
	else if ('>' == varcc) {
		char temp = GetC();
		switch (temp)
		{
		case '=':
			TokenTable.emplace_back(std::make_pair(std::string("relop"), std::string("GE")));
			break;
		case '>':
			TokenTable.emplace_back(std::make_pair(std::string(">>"), std::string("-")));
			break;
		default:
			TokenTable.emplace_back(std::make_pair(std::string("relop"), std::string("GT")));
			Rollback();
			break;
		}
	}
	else if ('=' == varcc) {
		if ('=' == GetC()) {
			TokenTable.emplace_back(std::make_pair(std::string("relop"), std::string("EQ")));
		}
		else {
			TokenTable.emplace_back(std::make_pair(std::string("assign-op"), std::string("-")));
			Rollback();
		}
	}
	else if ('!' == varcc) {
		if ('=' == GetC()) {
			TokenTable.emplace_back(std::make_pair(std::string("relop"), std::string("NE")));
		}
		else {
			Rollback();
			TokenTable.emplace_back(std::make_pair(std::string("!"), std::string("-")));
		}
	}
	//识别注释 / 除号
	else if ('/' == varcc) {
		char temp = GetC();
		switch (temp) {
		case '/':
			cur_state = STATE::Line_COMMENT;
			break;
		case '*':
			cur_state = STATE::Block_COMMENT;
			break;
		default:
			Rollback();
			TokenTable.emplace_back(std::make_pair(std::string("/"), std::string("-")));
		}
	}
	//识别OPs
	else if (OPs.find(std::string(1, varcc)) != OPs.end()) {
		char temp = GetC();
		if (('&' == varcc || '|' == varcc) && temp == varcc)
			TokenTable.emplace_back(std::make_pair(std::string(2, varcc), std::string("-")));
		else {
			Rollback();
			TokenTable.emplace_back(std::make_pair(std::string(1, varcc), std::string("-")));
		}
	}
	else
		throw std::runtime_error("Row " + std::to_string(rows) + ": The character " +
	std::string(1, varcc) + "(" + std::to_string(varcc) + ")" + " is illegal");

}

//获取下一个字符
//处理行数和字符数
char LexicalAnalysis::GetC() {
	char varcc = file.get();

	if ('\n' == varcc)
		rows ++;
	if (varcc != ' ' && varcc != '\n' && varcc != '\t')
		C_count++;

	return varcc;
}

//回退到上一个字符
//处理行数和字符数
void LexicalAnalysis::Rollback() {
	file.seekg(-1, std::ios::cur);
	//预先读下一个字符，但不改变指针指向
	char varcc = file.peek();

	if (varcc != ' ' && varcc != '\n' && varcc != '\t')
		C_count --;
	if ('\n' == varcc)
		rows --;
}

//打印需求
DF_Size LexicalAnalysis::GetR() const{
	return rows;
}

DF_Size LexicalAnalysis::GetCC() const {
	return C_count;
}

const std::set<std::string> LexicalAnalysis::GetIdT() const {
	return IdTable;
}

const std::vector<std::pair<std::string, std::string>> LexicalAnalysis::GetTT()const {
	return TokenTable;
}

const std::vector<std::string> LexicalAnalysis::GetCM() const{
	return comments;
}

const std::vector<std::string> LexicalAnalysis::GetER() const {
	return errors;
}

