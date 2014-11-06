#ifndef _FINDFILE_H
#define _FINDFILE_H

#include<iostream>
#include<string>
#include<stdio.h>
#include<io.h>
#include<stdexcept>
#include<Windows.h>
using namespace std;

// 返回目录下的文件
class FindFile{
public:
	FindFile( const char* path = "" )
		: _path(path)
		, _h_findfile(-1)
		//, _dir(path)
		, _first(true)
	{
		if( !_path.empty() )
		{
			int index = _path.find_last_of("\\");
			if( -1 == index )
			{
				index = _path.find_last_of('/');
			}
			if( index != -1 )
			{
				_dir = _path.substr( 0, index + 1 );
			}
		}
	}

	~FindFile()
	{
		_findclose( _h_findfile );
	}
	void reset()
	{
		_findclose( _h_findfile );
		_first = true;
	}
	string GetNextFile()
	{
		if( _first )
		{
			_first = false;
			return GetFirstFile();
		}
		int r;
		do{	// 跳过子目录
			r = _findnext( _h_findfile, &_find_data );
			if( -1L == r )
				return "";

		}while( _find_data.attrib & _A_SUBDIR );

		return _dir + _find_data.name;
	}

	class MyError: public std::runtime_error{
	public:
		MyError(const char* msg): runtime_error(msg){}
	};
	
private:

	// disallow this function
	FindFile(FindFile&);
	FindFile& operator=(FindFile&);

	std::string _dir;		// 存放路径: ...\ 
	std::string _path;		// 存放路径及搜索选项：...\*.exe
	intptr_t	_h_findfile;
	_finddata_t _find_data;

	bool _first;

	// 返回当前目录下的第一个找到的文件名，不包括子目录
	string GetFirstFile()
	{
		_h_findfile = _findfirst( _path.c_str(), &_find_data );

		if( -1L == _h_findfile )
			return "";

		if( _find_data.attrib & _A_SUBDIR  )
		{
			return GetNextFile();
		}
		return (string)_dir+_find_data.name;
	}
};

#endif
