#include<iostream>
#include<vector>

class StringFilter
{
	public:
		void addKeyword(std::string newKeyword);
		void filter(std::string &strToBeReplace);

	private:
		std::vector< std::string > keywords;
};

void  StringFilter::addKeyword(std::string newKeyword)
{
	this->keywords.push_back(newKeyword);
}

void StringFilter::filter(std::string &strToBeReplace)
{
	std::cout << strToBeReplace << std::endl;
	for (auto const& key : keywords)
	{
		size_t pos;
		size_t keyLen = key.length();
		while (std::string::npos != (pos = strToBeReplace.find(key)))
		{
			strToBeReplace = strToBeReplace.replace(pos, keyLen, keyLen, '*');
		}
	}
	std::cout << strToBeReplace << std::endl;
}


int main()
{
	StringFilter obj;
	std::string input = "ABCDEFGHIJKLMNOPQRSTUVWXYZ";

	obj.addKeyword("ABC");
	obj.addKeyword("IJK");
	obj.addKeyword("XYZ");

	obj.filter(input);
	return 0;
}
