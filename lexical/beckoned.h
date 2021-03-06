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
	//������ȡ
	NORMAL = 1,
	//ע���� ( // )
	Line_COMMENT,
	//ע�Ϳ� (/* */)
	Block_COMMENT,
	//Ԥ����ָ��
	PRE_TREATMENT
};

class LexicalAnalysis {

private:
	//�����ļ�
	std::ifstream& file;
	//�ļ�����
	DF_Size rows;
	//�ļ����ַ���
	DF_Size C_count;
	//ʶ��ǰ״̬
	STATE cur_state;

	//��ʶ����(ȥ��)
	std::set<std::string> IdTable;

	//�Ǻű�<���ţ� ����>
	//Ϊ��ʹ��vector�� emplace_back  (��ֵ)
	std::vector<std::pair<std::string, std::string>> TokenTable;
	//ע��
	std::vector<std::string> comments;
	//����
	std::vector<std::string> errors;

	//�����ּ���
	//set���Ա�֤ ��ѯЧ��
	static const std::set<std::string> Reserved_words;
	//�������ż���
	static const std::set<std::string> OPs;


	//��װ�������ܵĺ���
	//ֻ��public��ʹ��

	//��ȡ�ַ��� ָ�����
	char GetC();
	//���ˣ� ָ��ǰ��
	void Rollback();
	//��ȡ
	void READ(char&, std::string&);





public:
	//��ʼ��
	LexicalAnalysis(std::ifstream& infile) : file(infile), rows(1), C_count(0), cur_state(STATE::NORMAL) {}
	//����
	~LexicalAnalysis() {}

	//��Ҫ����
	//���ݶ�ȡ���ַ����� STATE ת��
	//�ж��Ƿ���� / ����
	bool transfer_go();

	//��ӡ����
	DF_Size GetR()const;
	DF_Size GetCC()const;
	const std::set<std::string> GetIdT()const;
	const std::vector<std::pair<std::string, std::string>> GetTT()const;
	const std::vector<std::string> GetCM()const;
	const std::vector<std::string> GetER()const;

};






#pragma once
