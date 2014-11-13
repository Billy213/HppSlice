#include"HPPSlice.h"
HppSlice::HppSlice( std::string hpp_path )
	:_hpp_name(hpp_path)
{

	_tmp_name = hpp_path+".tmp";
	int index = hpp_path.find_last_of(".");
	if( index != string::npos )
		_cpp_name = hpp_path.substr(0,index) + ".cpp" ;
	else 
		_cpp_name = hpp_path+".cpp";
}
void HppSlice::SliceInit()
{
	std::ifstream hpp(_hpp_name);
	if( !hpp )
	{
		std::cerr << "找不到文件！\n";
		exit(-1);
	}

	// 把源文件写出到缓存文件中，然后再一次读取缓存文件，把内容写入到 _out_cpp 及 _out_h 中
	
	std::ofstream tmp( _tmp_name );
	if( !tmp )
	{
		cerr << "无法输出缓存，文件 " << _tmp_name << " 打开失败\n";
		exit(-1);
	}
	tmp << hpp.rdbuf();
	tmp.close();
	hpp.close();

	_in.open( _tmp_name );
	_out_h.open(_hpp_name);
	
	if(!_in || !_out_h )
	{
		std::cerr << "文件打开失败\n";
		std::cerr << "file:" << __FILE__ << "line:" << __LINE__ << 

			std::endl;
		exit(-1);
	}
	string file_name;
	int index = _hpp_name.find_last_of("\\/");
	file_name = _hpp_name.substr(index+1);
	
	ifstream in(_cpp_name);
	// 如果源文件已经存在则不写入：#include 
	if( in )
	{
		in.close();
		_out_cpp.open(_cpp_name, ios::app );
	}
	else
	{
		_out_cpp.open(_cpp_name, ios::app );
		_out_cpp << "#include\"" << file_name << "\"" << endl;
	}
}
void  HppSlice::Slice()
{
	SliceInit();
	string line;
	while( std::getline( _in, line ) )
	{
		int index = line.find("///@");
		if( index != string::npos )
		{
			trimString(line);
			if( line == "///@slice" )
			{
				int c;
				_out_h << setClsName(c);
				if( '/' == c ) // 肯定为 注释
				{
					_out_h << skipNote(c) << (char)c;
				}
				else 
					_out_h << (char)c;
				continue;
			}
			else if("///@sl" == line )
			{
				handleFun();
				continue;
			}
			else if( "///@~" == line )
			{
				_cls_names.pop_back();
				continue;
			}
		}
		_out_h << line << endl;
	}
}
void HppSlice::Recover()
{
	_in.close();
	_out_cpp.close();
	_out_h.close();
	remove(_cpp_name.c_str());

	_in.open( _tmp_name );
	_out_h.open(_hpp_name);
	_out_h << _in.rdbuf();
	_in.close();
	_out_h.close();
}
void  HppSlice::FunSlice()
{
	_ret.clear();
	_decl.clear();
	_impl.clear();

	int c;// = _in.get();
	skipSpace(c);
	string name = skipSpaceNote(c);	// 把注释看成是 name 的一部分
	while(c!=EOF)
	{
		// 标示符结束标志
		if( isSpace(c) || '/' == c || '(' == c )
		{
			name.append(skipSpaceNote(c));
			break;
		}
		else 
		{
			name.push_back(c);
			c = _in.get();
		}
	}
	// 普通函数,name == ret
	if( c!= '(' )
	{
		_ret = name;
		name.clear();
		while(c!=EOF)
		{
			// 标示符结束标志
			if( isSpace(c) || '/' == c || '(' == c )
			{
				name.append(skipSpaceNote(c));
				break;
			}
			else
			{
				name.push_back(c);
				c = _in.get();
			}
		}
	}
	// name == funname
	assert('(' == c);
	_decl = name;
	_decl.append(skipParam(c));
	setImpl();
}
string  HppSlice::skipNote( int& c )
{
	string note;
	note.push_back('/');
	c = _in.get();
	note.push_back(_in.get());

	string s;

	if( "//" == note )
	{
		getline( _in, s );
		s += "\n";
		c = _in.get();
	}
	else if("/*" == note )
	{
		while((c=_in.get())!=EOF)
		{
			s.push_back(c);
			if( '*' == c )
			{
				c = _in.get();
				s.push_back(c);
				if( '/' == c )	
					break;
			}
		}
		c = _in.get();
	}
	else 
		return "";

	return note + s;
}
string  HppSlice::skipSpace( int& c )
{
	string s;
	c = _in.get();
	while( isSpace(c) )
	{
		s.push_back(c);
		c = _in.get();
	}
	return s;
}
string  HppSlice::skipQtMark( int type )
{
	assert( '\"' == type || '\'' == type );
	string s;
	int c;
	s.push_back(type);
	while((c=_in.get())!=EOF)
	{
		s.push_back(c);
		if( '\\' == c )
			s.push_back(_in.get());
		else if( type == c )
			break;
	}
	return s;
}
string  HppSlice::skipParam( int& c )
{
	assert('(' == c );
	string pa;
	pa.push_back(c);
	while((c = _in.get())!=EOF)
	{
		if( '\'' == c || '\"' == c )
			pa.append(skipQtMark(c));
		else if( '/' == c )
		{
			string s = skipNote(c);
			if( s.empty() )	// 当前 / 号不是注释，而是 除号
			{
				pa.push_back('/');
				pa.push_back(c);
			}
			else
			{
				pa.append(s);
				pa.push_back(c);
			}
		}
		else if( '{' == c )
			break;
		else
			pa.push_back(c);
	}
	while(isSpace(pa.back()))
		pa.pop_back();
	return pa;
}
string  HppSlice::skipSpaceNote( int& c )
{
	string s;
	string node,space;
	if( isSpace(c) )
	{
		s.push_back(c);
		s.append(skipSpace(c));
	}
	while( '/' == c )
	{
		s.append(skipNote(c));
		if(isSpace(c))
		{
			s.push_back(c);
			s.append(skipSpace(c));
		}
		else
			break;
	}
	return s;
}
void  HppSlice::setImpl()
{
	_impl.clear();
	int c;
	stack<char> bracket;
	_impl.push_back('{');	// 只有当 { 被处理后才会来到函数 setImpl  
	bracket.push('{');

	while( (c = _in.get()) != EOF )
	{
		if( '\'' == c || '\"' == c ){
			_impl.append( skipQtMark(c));
		}
		else if( '/' == c )
		{
			string s = skipNote(c);
			if(s.empty())	// 非注释
			{
				_impl.push_back('/');
				_impl.push_back(c);
			}
			else
			{
				_impl.append(s);
				_impl.push_back(c);
			}

		}
		else{
			_impl.push_back(c);
			if( '{' == c )
				bracket.push(c);
			else if( '}' == c )
			{
				assert( bracket.size() );
				bracket.pop();
				if(bracket.empty())
					break;
			}
		}
	}
	assert('}' == c );
}
void  HppSlice::handleFun()
{
	FunSlice();
	// 格式化输出
	for( int i = 0; i < _cls_names.size(); i++ )
	{
		_out_h<< "\t";
	}
	if(	!_ret.empty()){
		_out_h << _ret << " " << _decl << ";"<< endl ;
		_out_cpp << _ret << " ";
	}
	else{
		_out_h << _decl << ";" << endl ;
	}
	for( int i = 0; i < _cls_names.size(); i++ )
	{
		_out_cpp << _cls_names[i] << "::";	
	}
	_out_cpp << _decl << endl << _impl << endl;
}
void  HppSlice::trimString( std::string&s )
{
	int first = s.find_first_not_of(" \t\n");
	int last  = s.find_last_not_of(" \t\n");
	s = s.substr(first, last-first+1);
}
string  HppSlice::setClsName( int& c )
{
	string name;
	string s;
	c = _in.get();
	s = skipSpaceNote(c);
	assert('c' == c);
	s.push_back(c);	// 
	// 'c' 字符已经被吃掉了
	for( int i = 0; i < strlen("lass"); i++ )
		s.push_back(_in.get());

	c = _in.get();
	s.append(skipSpaceNote(c));
	s.push_back(c);
	name.push_back(c);
	while( (c=_in.get()) != EOF )
	{
		if( c == '{' || c == ':' || isSpace(c) || c == '/' )
			break;
		else
		{
			name.push_back(c);
			s.push_back(c);
		}
	}
	_cls_names.push_back(name);
	return s;
}
bool  HppSlice::isSpace( char c )
{
	if( c == ' ' || c == '\t' || c == '\n' )
		return true;
	return false;
}
