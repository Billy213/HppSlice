#include "HPPSlice.h"
#include <FindFile.h>
#include <vector>
using namespace std;
// 命令行参数：
// -b {dir/filename }|dir/* 恢复指定文件或指定文件夹下的文件，可以使用通配符
// -s {dir/filename }|dir/* 分离指定文件或指定文件夹下的文件，可以使用通配符
int main( int argc, char** argv )
{
	vector<string> files;
	string com;
	if( argc < 2 )
	{
		int n;
		cout << "请输入操作：\n";
		cout << "-b n{dir/filename }|dir/* 恢复指定文件,可以使用通配符,  n 表示参数个数\n";
		cout << "-s n{dir/filename }|dir/* 分离指定文件,可以使用通配符,  n 表示参数个数\n";
		cin >> com;
		cin >> n; 
		while(n--)
		{
			string s;
			cin >> s;
			files.push_back(s);
		}
	}
	else 
	{
		com = argv[1];
		for( int i = 2; i < argc; i++ )
			files.push_back( argv[i] );
	}
	vector<string>files_tmp;
	for( int i = 0; i < files.size(); i++ )
	{
		FindFile ff(files[i].c_str());
		for( string name = ff.GetNextFile(); !name.empty(); name = ff.GetNextFile() )
		{
			files_tmp.push_back(name);
		}
	}
	files = files_tmp;

	if( "-b" == com )
	{
		for( int i = 0; i < files.size(); i++ )
		{
			HppSlice hs( files[i] );
			hs.Recover();
		}
	}
	else if( "-s" == com )
	{
		for( int i = 0; i < files.size(); i++ )
		{
			HppSlice hs( files[i] );
			hs.Slice();
		}
	}
	else 
	{
		cerr << "指令不存在!\n";
		exit(-1);
	}
	cout << "successed!\n";

}

