#include <iostream>
#include <vector>
#include <unordered_map>
#include <fstream>

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

//To find match keyword
class DicTreeNode *DicTreeNode::isMatchKeyword(char *str, off_t maxLen, char * &out)
{
	DicTreeNode *result = this;
	off_t ind = 0;
	class DicTreeNode *retval = NULL;
	std::unordered_map< char, class DicTreeNode *>::iterator it;
	
	out = str;
	search_count++;
	while ( ((it = result->map.find(str[ind])) !=  result->map.end()) && ind < maxLen) {
		//std::cout << str[ind] << std::endl;
		result = it->second;
		ind++;
		search_count++;
		if (result->getEnd()) {
			retval = result;
			out = str + ind;
			//break;
		} else {
			if (retval == NULL && result->toFirstMatchNode != NULL) {
				retval = result->toFirstMatchNode;
				out = str + ind;
			}
		}
	}
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

	snprintf(szBuf, newKeyword.size() + 1, "%s", newKeyword.c_str());
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
}

void StringFilter::filter(std::string &strToBeReplace)
{
	char *target = new char[strToBeReplace.size() + 1];
	char *ptr = target;
	char *out = NULL;
	off_t ind = 0;
	off_t maxlen = strToBeReplace.size();
	off_t remain = maxlen;
	class DicTreeNode *cur = root;

	snprintf(target, strToBeReplace.size() + 1, "%s", strToBeReplace.c_str());
	//std::cout << target << std::endl;

	while(ind < maxlen) {
		class DicTreeNode *next = cur->isMatchKeyword(ptr, remain, out);
		//std::cout << "ind: "  << ind << std::endl;
		//std::cout << "remain: "  << remain << std::endl;
		//std::cout << "XXXX: target = " << target << std::endl;
		//std::cout << "XXXX: str = " << ptr << std::endl;
		//std::cout << "XXXX: out = " << out << std::endl;
		//if (next == NULL) {
		//	std::cout << "XXXX: NOT FOUND!!!" << std::endl;
		//} else {
		//	std::cout << "XXXX: next->value = " << next->value << std::endl;
		//}
		ind = out - target + 1;
		ptr = out;
		ptr++;
		if (next != NULL && next->getEnd()) {
			//std::cout << "XXXX: found" << std::endl;
			for (off_t i = 0; i <= next->length; i++) {
				*(out - i - 1) = '*';
			}
			cur = root; // reset
		} else if (next == NULL) {
			//std::cout << "XXXX: IGNORE all char and start with next char" << std::endl;
			cur = root;
		} else { // keep searching
			//std::cout << "XXXX: keep search" << std::endl;
			ind = ind + next->length - cur->length;
			cur = next;
		}
		remain = maxlen - ind;
	}
	strToBeReplace = target;
	delete [] target;
}

//#define UT
#ifdef UT
#define UT_CASE_1
#define UT_CASE_2
#endif

int main()
{
#ifdef UT
#ifdef UT_CASE_1
	{
		StringFilter obj;
		std::string input = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";
		std::string result = "****EFGH***LMNOPQRSTUVW***";
		obj.addKeyword("ABCD");
		obj.addKeyword("IJK");
		obj.addKeyword("XYZ");
		obj.buildKMP();

		std::cout << "UT case 1" << std::endl;
		std::cout << input << std::endl;
		obj.filter(input);
		std::cout << input << std::endl;
		std::cout << result << std::endl;
		if (input == result) {
			std::cout << "Success" << std::endl;
		} else {
			std::cout << "Failed" << std::endl;
		}
		std::cout << "input length: " << input.size() << "; search count: " << DicTreeNode::search_count << std::endl;
	}
#endif //UT_CASE_1
#ifdef UT_CASE_2
	{
		StringFilter obj;
		std::string input = "ABCDEF";
		std::string result = "ABCDE*";
		obj.addKeyword("ABCDEG");
		obj.addKeyword("BCDEG");
		obj.addKeyword("CDEG");
		obj.addKeyword("DEG");
		obj.addKeyword("EG");
		obj.addKeyword("F");
		obj.buildKMP();

		std::cout << "UT case 2" << std::endl;
		std::cout << input << std::endl;
		obj.filter(input);
		std::cout << input << std::endl;
		std::cout << result << std::endl;
		if (input == result) {
			std::cout << "Success" << std::endl;
		} else {
			std::cout << "Failed" << std::endl;
		}
		std::cout << "input length: " << input.size() << "; search count: " << DicTreeNode::search_count << std::endl;
	}
#endif //UT_CASE_2
#endif //UT

	StringFilter obj;
	std::string input;
	std::ifstream keyword_file("./test/keywords");
	std::ifstream filter_file("./test/11-0.txt");

	std::ofstream result;
	std::ofstream stat;

	result.open("./test/result.txt");
	stat.open("./test/stat.txt");

	stat << "input_length	search_count"<< std::endl;

	while (std::getline(keyword_file, input))
	{
		//std::cout << input << std::endl;
		obj.addKeyword(input);
	}

	while (std::getline(filter_file, input))
	{
		if (input.size() == 0) {
			result << input << std::endl;
			continue;
		}
		DicTreeNode::search_count = 0;
		std::cout << input << std::endl;
		obj.filter(input);
		std::cout << input << std::endl;
		result << input << std::endl;
		stat << input.size() << "	" << DicTreeNode::search_count << std::endl;
	}


	return 0;
}
