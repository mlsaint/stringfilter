#include <iostream>
#include <vector>
#include <unordered_map>
#include <fstream>
#include <locale>
#include <algorithm>

//#define CASELESS_SEARCH

class DicTreeNode
{
	public:
		DicTreeNode(char val = '\0', off_t len = 0);
		~DicTreeNode();
		void addChildNode(char, class DicTreeNode *);
		DicTreeNode* findLeaf(char *, off_t &);
		void setEnd(bool);
		bool getEnd();
		class DicTreeNode *isMatchKeyword(char *, off_t, char * &);

		static long search_count; // compare counter

		bool isEnd; // to avoid "ABC" and "ABCD.." case
		char value;
		off_t length;
		std::unordered_map < char, class DicTreeNode * > map; // use unorder map as tree
		std::vector< class DicTreeNode * > childNodes; // for deconstructor
		class DicTreeNode *parentNode;
		class DicTreeNode *toFirstMatchNode; // KMP algo
};

long DicTreeNode::search_count = 0;


DicTreeNode::DicTreeNode(char val, off_t len)
{
	value = val;
	length = len;
	isEnd = false;
	toFirstMatchNode = NULL;
}

DicTreeNode::~DicTreeNode()
{
	map.clear();
	for (auto const& child : childNodes) {
		delete child;
	}
	childNodes.clear();
}

// To find the last common prefix node between keywords
DicTreeNode* DicTreeNode::findLeaf(char *input, off_t &pos)
{
	DicTreeNode *result = this;
	off_t ind = 0;
	for (ind = 0; ind < pos; ind++) {
		std::unordered_map< char, class DicTreeNode *>::iterator it;
		if ( (it = result->map.find(input[ind])) != result->map.end() )
		{
			//std::cout << "result char = " << it->first << std::endl;
			result = it->second;
		} else {
			break;
		}
	}

	pos = ind;
	return result;
}

void DicTreeNode::addChildNode(char c, class DicTreeNode *child)
{
	this->map.insert(std::make_pair(c, child));
	this->childNodes.push_back(child);
}

void DicTreeNode::setEnd(bool flag)
{
	isEnd = flag;
}

bool DicTreeNode::getEnd()
{
	return isEnd;
}

std::string boolToString(bool val)
{
	if (val) {
		return "TRUE";
	}
	return "FASLE";
}

//To find match keyword
class DicTreeNode *DicTreeNode::isMatchKeyword(char *str, off_t maxLen, char * &out)
{
	DicTreeNode *result = this;
	off_t ind = 0;
	class DicTreeNode *retval = NULL;
	std::unordered_map< char, class DicTreeNode *>::iterator it;
	
	search_count++;
	while ( ((it = result->map.find(str[ind])) !=  result->map.end()) && ind < maxLen) {
		//std::cout << "GET: "<< str[ind] << std::endl;
		result = it->second;
		ind++;
		search_count++;
		if (retval == NULL && result->toFirstMatchNode != NULL) {
			// keep first start node
			retval = result->toFirstMatchNode;
			out = str + ind;
		}
		if (result->getEnd()) {
			// match a substring keyword
			retval = result;
			out = str + ind;
		}
	}
	//if (retval) std::cout << "RESULT: " << retval->value <<", isEnd: " << boolToString(retval->getEnd()) << std::endl;
	return retval;
}

class StringFilter
{
	public:
		StringFilter();
		~StringFilter();
		void reset();
		void addKeyword(std::string newKeyword);
		void filter(std::string &strToBeReplace);
		void buildKMP();

	private:
		class DicTreeNode *root;
};

StringFilter::StringFilter()
{
	DicTreeNode::search_count = 0;
	root = new DicTreeNode();
}

StringFilter::~StringFilter()
{
	delete root;
}

void StringFilter::reset()
{
	DicTreeNode::search_count = 0;
	delete root;
	root = new DicTreeNode();
}

void StringFilter::buildKMP()
{
	std::vector< class DicTreeNode * > DFS_nodesTravesal;
	//intial
	for (class DicTreeNode *child : root->childNodes) {
		child->toFirstMatchNode = NULL;
		for (class DicTreeNode *node : child->childNodes) {
			DFS_nodesTravesal.push_back(node);
		}
	}
	while(DFS_nodesTravesal.size() != 0) {
		class DicTreeNode *node = DFS_nodesTravesal.back();
		DFS_nodesTravesal.pop_back();
		//std::cout << "node [" << node->value << "], is end node " << boolToString(node->getEnd())  << std::endl;

		std::unordered_map< char, class DicTreeNode *>::iterator it;
		if ( (it = root->map.find(node->value)) != root->map.end() ) {
			//std::cout << "ZZZZ: node (" << node->value << "): FOUND" << std::endl;
			node->toFirstMatchNode = it->second;
		} else {
			//std::cout << "ZZZZ: node (" << node->value << "): NOT FOUND" << std::endl;
			node->toFirstMatchNode = NULL;
		}
		for (class DicTreeNode *n : node->childNodes) {
			DFS_nodesTravesal.push_back(n);
		}
	}
}

void StringFilter::addKeyword(std::string newKeyword)
{
	char *szBuf = new char[newKeyword.size() + 1];
	off_t pos, len;
	DicTreeNode *node, *child;
	std::string temp = newKeyword;

#ifdef CASELESS_SEARCH
	transform(temp.begin(), temp.end(), temp.begin(), toupper);
#endif
	snprintf(szBuf, newKeyword.size() + 1, "%s", temp.c_str());
	len = pos = newKeyword.length();
	node = root->findLeaf(szBuf, pos);

	//std::cout << "pos = " << pos << std::endl;
	//std::cout << "len = " << len << std::endl;
	while (pos < len) {
		//std::cout << szBuf[pos] << std::endl;
		child = new DicTreeNode(szBuf[pos], pos);
		node->addChildNode(szBuf[pos], child);
		child->parentNode = node;
		node = child;
		pos++;
	}
	
	node->setEnd(true);
	//std::cout << "node is " << node->value << std::endl;
}

void StringFilter::filter(std::string &strToBeReplace)
{
	char *target = new char[strToBeReplace.size() + 1];
	char *res_buffer = new char[strToBeReplace.size() + 1];
	char *ptr = target;
	char *out = NULL;
	off_t ind = 0;
	off_t maxlen = strToBeReplace.size();
	off_t remain = maxlen;
	class DicTreeNode *cur = root;

	std::string temp = strToBeReplace;
#ifdef CASELESS_SEARCH
	transform(temp.begin(), temp.end(), temp.begin(), toupper);
#endif
	snprintf(target, temp.size() + 1, "%s", temp.c_str());
	snprintf(res_buffer, strToBeReplace.size() + 1, "%s", strToBeReplace.c_str());
	//std::cout << target << std::endl;

	while(ind < maxlen) {
		//std::cout << "-------------" << std::endl;
		//std::cout << "ind: "  << ind << std::endl;
		//std::cout << "remain: "  << remain << std::endl;
		//if (cur != root) {
		//	std::cout << "XXXX: cur = " << cur->value << std::endl;
		//}
		//std::cout << "XXXX: target = " << res_buffer << std::endl;
		//std::cout << "XXXX: str = " << ptr << std::endl;
		out = ptr;
		class DicTreeNode *next = cur->isMatchKeyword(ptr, remain, out);
		//std::cout << "XXXX: out = " << out << std::endl;
		if (next != NULL && next->getEnd()) {
			//std::cout << "XXXX: found" << std::endl;
			for (off_t i = 0; i <= next->length; i++) {
				*(res_buffer + ((out - i - 1) - target)) = '*';
			}
			cur = root; // reset
			ind = out - target;
			ptr = out;
		} else if (next == NULL) {
			//std::cout << "XXXX: IGNORE all char and start with next char" << std::endl;
			ind = out - target;
			ptr = out;
			if (cur == root) {
				ptr++;
			}
			cur = root;
		} else { // keep searching
			//std::cout << "XXXX: keep search" << std::endl;
			ind = ind + next->length - cur->length + 1;
			cur = next;
			ptr = out;
		}
		remain = maxlen - ind;
	}
	strToBeReplace = res_buffer;
	delete [] target;
	delete [] res_buffer;
}

//#define UT
#ifdef UT
#define UT_CASE_1
#define UT_CASE_2
#define UT_CASE_3
#define UT_CASE_4
#endif

int main()
{
#ifdef UT
#ifdef UT_CASE_1
	{
		StringFilter obj1;
		std::string input = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
		std::string result = "****EFGH***LMNOPQRSTUVW***";
		obj1.addKeyword("ABCD");
		obj1.addKeyword("IJK");
		obj1.addKeyword("XYZ");
		obj1.buildKMP();

		std::cout << "UT case 1" << std::endl;
		std::cout << input << std::endl;
		obj1.filter(input);
		std::cout << input << std::endl;
		std::cout << result << std::endl;
		if (input == result) {
			std::cout << "Success" << std::endl;
		} else {
			std::cout << "Failed" << std::endl;
			exit(0);
		}
		std::cout << "input length: " << input.size() << "; search count: " << DicTreeNode::search_count << std::endl;
	}
#endif //UT_CASE_1

#ifdef UT_CASE_2
	{
		StringFilter obj2;
		std::string input = "ABCDEF";
		std::string result = "ABCDE*";
		obj2.addKeyword("ABCDEG");
		obj2.addKeyword("BCDEG");
		obj2.addKeyword("CDEG");
		obj2.addKeyword("DEG");
		obj2.addKeyword("EG");
		obj2.addKeyword("F");
		obj2.buildKMP();

		std::cout << "UT case 2" << std::endl;
		std::cout << input << std::endl;
		obj2.filter(input);
		std::cout << input << std::endl;
		std::cout << result << std::endl;
		if (input == result) {
			std::cout << "Success" << std::endl;
		} else {
			std::cout << "Failed" << std::endl;
			exit(0);
		}
		std::cout << "input length: " << input.size() << "; search count: " << DicTreeNode::search_count << std::endl;
	}
#endif //UT_CASE_2

#ifdef UT_CASE_3
	{
		StringFilter obj3;
		std::string input = "ABCD";
		std::string result = "A**D";
		obj3.addKeyword("ABCZ");
		obj3.addKeyword("BC");
		obj3.buildKMP();

		std::cout << "UT case 3" << std::endl;
		std::cout << input << std::endl;
		obj3.filter(input);
		std::cout << input << std::endl;
		std::cout << result << std::endl;
		if (input == result) {
			std::cout << "Success" << std::endl;
		} else {
			std::cout << "Failed" << std::endl;
			exit(0);
		}
		std::cout << "input length: " << input.size() << "; search count: " << DicTreeNode::search_count << std::endl;
	}
#endif //UT_CASE_3

#ifdef UT_CASE_4
	{
		StringFilter obj4;
		std::string input = "ACACACACACAD";
		std::string result = "************";
		obj4.addKeyword("AC");
		obj4.addKeyword("ACAD");
		obj4.buildKMP();

		std::cout << "UT case 4" << std::endl;
		std::cout << input << std::endl;
		obj4.filter(input);
		std::cout << input << std::endl;
		std::cout << result << std::endl;
		if (input == result) {
			std::cout << "Success" << std::endl;
		} else {
			std::cout << "Failed" << std::endl;
			exit(0);
		}
		std::cout << "input length: " << input.size() << "; search count: " << DicTreeNode::search_count << std::endl;
	}
#endif //UT_CASE_4

#else // UT
	StringFilter obj;
	std::string line;
	std::string input;
	std::ifstream keyword_file("./test/keywords");
	std::ifstream filter_file("./test/11-0.txt");

	std::ofstream result;
	std::ofstream stat;

	result.open("./test/result.txt");
	stat.open("./test/stat.txt");

	stat << "input_length	search_count"<< std::endl;

	while (std::getline(keyword_file, line))
	{
		obj.addKeyword(line);
	}
	obj.buildKMP();

	input.clear();
	while (std::getline(filter_file, line))
	{
		input += line;
		if (input.size() < 100) {
			input += "\n";
			continue;
		}
		DicTreeNode::search_count = 0;
		std::cout << input << std::endl;
		obj.filter(input);
		std::cout << input << std::endl;
		result << input << std::endl;
		stat << input.size() << "	" << DicTreeNode::search_count << std::endl;
		input.clear();
	}
#endif


	return 0;
}
