#include<iostream>
#include<vector>
#include<unordered_map>

class DicTreeNode
{
	public:
		DicTreeNode(char val = '\0', unsigned int len = 0);
		~DicTreeNode();
		void addChildNode(char, class DicTreeNode *);
		DicTreeNode* findLeaf(char *, off_t &);
		void setEnd(bool);
		bool getEnd();
		off_t isMatchKeyword(char *, off_t);

	private:
		bool isEnd;
		char value;
		unsigned int length;
		std::unordered_map < char, class DicTreeNode * > map;
		std::vector< class DicTreeNode * > childNodes; // for deconstructor
};

DicTreeNode::DicTreeNode(char val, unsigned int len)
{
	value = val;
	length = len;
}

DicTreeNode::~DicTreeNode()
{
	for (auto const& child : childNodes) {
		delete child;
	}
}

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

off_t DicTreeNode::isMatchKeyword(char *str, off_t maxLen)
{
	DicTreeNode *result = this;
	off_t ind = 0;
	std::unordered_map< char, class DicTreeNode *>::iterator it;
	
	while ( ((it = result->map.find(str[ind])) !=  result->map.end()) && ind < maxLen) {
		//std::cout << str[ind] << std::endl;
		result = it->second;
		ind++;
	}
	if (result->getEnd()) {
		return ind;
	}
	return 0;
}

class StringFilter
{
	public:
		void addKeyword(std::string newKeyword);
		void filter(std::string &strToBeReplace);

	private:
		std::vector< std::string > keywords;
		class DicTreeNode root;
};

void  StringFilter::addKeyword(std::string newKeyword)
{
	char *szBuf = new char[newKeyword.size() + 1];
	off_t pos, len;
	DicTreeNode *node, *child;

	this->keywords.push_back(newKeyword);

	snprintf(szBuf, newKeyword.size() + 1, "%s", newKeyword.c_str());
	len = pos = newKeyword.length();
	node = root.findLeaf(szBuf, pos);

	//std::cout << "pos = " << pos << std::endl;
	//std::cout << "len = " << len << std::endl;
	while (pos < len) {
		//std::cout << szBuf[pos] << std::endl;
		child = new DicTreeNode(szBuf[pos], pos);
		node->addChildNode(szBuf[pos], child);
		node = child;
		pos++;
	}
	
	node->setEnd(true);
}

void StringFilter::filter(std::string &strToBeReplace)
{
	char *target = new char[strToBeReplace.size() + 1];
	char *ptr = target;
	off_t ind = 0;
	off_t maxlen = strToBeReplace.size();
	off_t remain = maxlen;

	snprintf(target, strToBeReplace.size() + 1, "%s", strToBeReplace.c_str());
	//std::cout << target << std::endl;

	while(ind < maxlen) {
		//std::cout << "ind: "  << ind << std::endl;
		//std::cout << "remain: "  << remain << std::endl;
		off_t len = root.isMatchKeyword(ptr, remain);
		//std::cout << "len: "  << len << std::endl;
		for (off_t i = ind; i < ind + len; i++) {
			target[i] = '*';
		}
		//std::cout << target << std::endl;
		ind = ind + len + 1;
		ptr = ptr + len + 1;
		remain = maxlen - ind;
	}
	strToBeReplace = target;
	delete [] target;
}

int main()
{
	StringFilter obj;
	std::string input = "ABCDEFGHIJKLMNOPQRSTUVWXYZ, ABCDEFGHIJKLMNOPQRSTUVWXYZ";
	obj.addKeyword("ABC");
	obj.addKeyword("ABCD");
	obj.addKeyword("IJK");
	obj.addKeyword("XYZ");

	std::cout << input << std::endl;
	obj.filter(input);
	std::cout << input << std::endl;
	return 0;
}
