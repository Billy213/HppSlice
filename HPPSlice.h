#include<iostream>
#include<fstream>
#include<string>
#include<vector>
#include<cassert>
#include<sstream>
#include<algorithm>
#include<stack>

using namespace std;


/*******************************************
* 标签定义：
* ///@slice : 表示将对该类进行分离，如果类上一行没有该标签则不会对该类进行处理。
* 如果一个内嵌类包含该标签，则其包含类必须拥有该标签，
* ///@sl	: 表示将对该方法进行分离，如果方法的上一行没有该标签则不会对该方法进行处理
* ///@~		: 表示类的结束
* 上述标记必须单独一行，否则失败

* 基本思路：首先把文本整个读进内存，然后找到
* ///@slice 标签，解析当前的类名，记录下来。
* 继续查找 ///@sl 标签，结合类名对该标签下对应的方法进行处理，写到输出文件中
* 如果 ///@sl 下对应的是一个静态变量，则做相应的处理。(这个没有实现)
* 输出结果：
* 1、实现文件：.cpp
* 2、去掉源文件中的相应实现代码
* 3、备份文件：源文件名字+.tmp
*
* 限制条件：
	1、///@sl 适用的函数有比较严格的限制，要求格式如下才能应用 ///@sl 指令：
		@[name]@name@(xxxxxx)@{xxxxxxx}
		@:表示空格和注释的任意组合, [] 表示可选， name 可以是返回值或者是 函数名，但是只能是一个单个的单词，如不可以是：const int, 因此在使用的时候要注意，否则可能会造成源码的损害。
		如果发现分离后代码有问题，可以通过 .tmp 文件恢复。
*******************************************/
using std::string;
class HppSlice{
public:
	HppSlice( std::string hpp_path );
	void Slice();
	void Recover();
private:
	void SliceInit();
	// 把一个函数分为三部分：返回值、声明、实现
	// 要求：调用之前函数指针指向函数返回值的开头部分，可以有前导空格
	/**********************************
	* 
	* 请注意，能力有限，所以函数的声明或实现的格式如下才能应用 ///@sl 指令：
		@[name]@name@(xxxxxx)@{xxxxxxx}
		@:表示空格和注释的任意组合, [] 表示可选， name 可以是返回值或者是 函数名，但是只能是一个单个的单词，如不可以是：const int, 因此在使用的时候要注意，否则可能会造成源码的损害。
		如果发现分离后代码有问题，可以通过 .tmp 文件恢复。
	***********************************/
	void  FunSlice();
		// 跳过注释的内容并且返回，返回的形式类似：//xxx 或 /* xxx
	// 输入：前件：函数指针刚好读完 '/' 符号
	string  skipNote(int& c);

	// 跳过空白字符，
	// 返回跳过的空白字符，
	// c 中存放空白字符的后一个字符
	string  skipSpace( int& c );

	// 跳过引号内容
	// 返回形式："xxx" 或 'xxx'
	// type == " 或 type == '
	string  skipQtMark( int type );

	// 跳过函数声明的参数部分：文件指针指向 左括号的后一个字符
	// c 中存放的是声明语句后面的第一个字符'{'，此时文件指针指向 c 后面的第一个字符
	// 返回形式为：(xx,xx)
	string  skipParam( int& c );

	// 跳过空格和注释的任意组合
	// 作为传入参数：c == space 或 c == / 或 其它，
	// 作为传出：c 表示下一个待处理的字符
	// 返回时，head 记录多读取的字符, assert(head.size()==2)
	string  skipSpaceNote( int& c );
	void  setImpl();
	void  handleFun();
	void  trimString( std::string&s );
	// 返回被读取的字符
	string  setClsName( int& c );
	// 不能用 c 库的 isspace 因为不能处理中文
	bool  isSpace( char c );

	std::ifstream _in;		// 从这里读进
	std::ofstream _out_cpp;	// 写出为实现文件
	std::ofstream _out_h;	// 写出为头文件
	std::vector<std::string> _cls_names;	// 为了处理嵌套的类名
	std::string _ret;
	std::string _decl;
	std::string _impl;

	std::string _hpp_name;
	std::string _cpp_name;
	std::string _tmp_name;

};
