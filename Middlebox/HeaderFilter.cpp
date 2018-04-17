#include "HeaderFilter.h"
#include <fstream>
#include "config.h"


HeaderFilter::HeaderFilter()
{
	int blockSize = ORE_BLK_SIZE;
	engine.Init(ORE_KEY, blockSize);
}

int HeaderFilter::CheckHeader(const string& token, bool fastReturn)
{
	for (auto it = vecHeaderRules.begin(); it != vecHeaderRules.end(); it++)
	{
		if (it->filter(&engine, token))
		{
			return it->getid();
		}
		if (fastReturn)
			return 0;
	}
	return -1;
}

void HeaderFilter::Build(string path, int size)
{
	const int maxLineSize = 64;
	char buffer[maxLineSize];
	ifstream fin(path);

	if (fin.good())
	{
		vecHeaderRules.clear();
		for (int i = 0; fin.good()&&i<size; i++)
		{
			fin.getline(buffer, maxLineSize);
			HeaderRule head(buffer,i);
			head.init(&engine);
			vecHeaderRules.push_back(std::move(head));
		}
		fin.close();
		cout << "Load "<<vecHeaderRules.size()<< " rules." << endl;
	}
	else
	{
		cout << "Open file "<< path <<" Failed." << endl;
	}
}
