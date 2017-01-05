
#include <fstream>
#include <streambuf>
#include <iostream>

#include "file_util.h"

using namespace std;

FileUtil* g_pFileUtil = NULL;

FileUtil::FileUtil()
{

}

FileUtil::~FileUtil()
{

}

FileUtil* FileUtil::GetInstance()
{
	static FileUtil fu;
	return &fu;
}

bool FileUtil::ReadFile(const char* file_name, string& str)
{
	bool is_ok = false;

	ifstream is(file_name);

	if (is)
	{
		is.seekg(0, is.end);
		str.reserve(is.tellg());
		is.seekg(0, is.beg);

		str.assign((istreambuf_iterator<char>(is)),
			istreambuf_iterator<char>());

		is_ok = true;
	}

	return is_ok;
}