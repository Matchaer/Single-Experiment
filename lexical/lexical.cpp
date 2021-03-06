#include"beckoned.h"

bool LexicalAnalysis::transfer_go() {
	//���������Լ���ʼ�����
	char varcc;
	std::string sstring = "";

	//���ļ�ǰ�Ĳ���
	//�������
	if (EOF == GetC()) {
		rows = 0;
		//����true�� �����
		return true;

	}
	//����ǰ�ļ�ָ�벻�ڿ�ʼλ��
	//���ؿ�ʼλ��
	else {
		file.clear();
		file.seekg(0, std::ios::beg);
	}

	//��ʼ��
	while ((varcc = GetC()) != EOF) {
		switch (cur_state)
		{
		case STATE::PRE_TREATMENT:
			if ('\n' == varcc)
				cur_state = STATE::NORMAL;
			break;
		case STATE::NORMAL:
			//����try-catch�ṹ���Բ�׽�� error type
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
			//���з�Ϊ��β
			if ('\n' == varcc) {
				comments.emplace_back(sstring);
				sstring.clear();
				cur_state = STATE::NORMAL;
				break;
			}
			sstring += varcc;
			break;
		case STATE::Block_COMMENT:
			//  ������һ��* ����β
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

	//��ζ�

	//Ԥ����ָ��
	if ('#' == varcc) {
		cur_state = STATE::PRE_TREATMENT;
	}
	//������(���漰����Щ����)
	else if (isdigit(varcc) || '+' == varcc || '-' == varcc || '.' == varcc) {

		tokens += varcc;
		DF_Size state = 0;
		DF_Size trigger = 0;

		//�ж��ǵ����� + - �� \ ������(+= -= ++ --) <ֱ���˳�> ���Ǻ�������ϵ� ���� +.99 �� -.99 �����
		if ('+' == varcc || '-' == varcc) {
			char temp = GetC();
			//һ���ǵ��� + - �� 
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

		//������ <�������ϵ��Զ���>
		while (!trigger) {

			varcc = GetC();

			switch (state)
			{
				//����
			case 1:
				if (isdigit(varcc))
					break;
				//�п����Ǳ�κ� �� С��
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
				//��κ�
				else if (isalpha(varcc)) {
					throw std::runtime_error("Row " + std::to_string(rows) +" ( "+ tokens + varcc + " ) "+
						": Bad Exponential");
				}
				else
					trigger = 1;
				break;

			case 7:
				//��κ�
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
	//ʶ���ַ���
	else if (varcc == '\"') {
		while ((varcc = GetC()) != '\"') {

			if ('\n' == varcc) {
				throw std::runtime_error("Row " + std::to_string(rows - 1) +
					": missing closing quotation mark \" character");
			}
			//��ֹ ���� ʶ�� ĩβ������
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
	//�����ո񡢻��С��Ʊ��
	else if ('\n' == varcc || '\t' == varcc || ' ' == varcc) {}
	//�����ʶ��
	else if ('_' == varcc || isalpha(varcc)) {
		tokens += varcc;
		varcc = GetC();
		while (isalpha(varcc) || isdigit(varcc) || '_' == varcc)
		{
			tokens += varcc;
			varcc = GetC();
		}
		//ע������ѭ����β�� ��ȻŲ����ָ��
		Rollback();
		if (Reserved_words.find(tokens) != Reserved_words.end()) {
			TokenTable.emplace_back(std::make_pair(tokens, std::string( "-")));
		}
		else {
			IdTable.insert(tokens);
			TokenTable.emplace_back(std::make_pair(tokens, std::string("id")));
		}
	}
	//ʶ���ַ�
	else if ('\'' == varcc) {
		varcc = GetC();
		//�������
		if ('\'' == varcc) {
			throw std::runtime_error("Row " + std::to_string(rows) +
				": Too few arguments for char const");
		}
		//ת���ַ�
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
	//ʶ���ϵ�����
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
	//ʶ��ע�� / ����
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
	//ʶ��OPs
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

//��ȡ��һ���ַ�
//�����������ַ���
char LexicalAnalysis::GetC() {
	char varcc = file.get();

	if ('\n' == varcc)
		rows ++;
	if (varcc != ' ' && varcc != '\n' && varcc != '\t')
		C_count++;

	return varcc;
}

//���˵���һ���ַ�
//�����������ַ���
void LexicalAnalysis::Rollback() {
	file.seekg(-1, std::ios::cur);
	//Ԥ�ȶ���һ���ַ��������ı�ָ��ָ��
	char varcc = file.peek();

	if (varcc != ' ' && varcc != '\n' && varcc != '\t')
		C_count --;
	if ('\n' == varcc)
		rows --;
}

//��ӡ����
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

