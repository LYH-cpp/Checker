//checker_v1.2.1
//BY: LYH
#include <cstdio>//输入输出
#include <windows.h>//system函数 & Win32 API
#include <future>//多进程
#include <initializer_list>
class String{//当初脑子瓦特，觉得std::string不能向下转化为char数组，所以自己写了一个
	static constexpr int max_size=300;
	char s[max_size];
	int len;
public:
	String(const char*s_=""){//构造
		len=0;
		while(s_[len]){
			s[len]=s_[len];
			len++;
		}
		s[len]=0;
	}
	String(const String&s_){//复制构造
		len=0;
		while(len!=s_.len){
			s[len]=s_.s[len];
			len++;
		}
		s[len]=0;
	}
	operator char*(){//转化为char数组
		return s;
	}
	const String&operator=(const String&s_){//赋值
		len=0;
		while(len!=s_.len){
			s[len]=s_.s[len];
			len++;
		}
		s[len]=0;
		return *this;
	}
	const String&operator=(const char*s_){
		len=0;
		while(s_[len]){
			s[len]=s_[len];
			len++;
		}
		s[len]=0;
		return *this;
	}
	String operator+(const String&b)const{//连接字符串
		String ans=*this;
		for(int i=0;i<b.len;i++)ans.s[ans.len++]=b.s[i];
		ans.s[ans.len]=0;
		return ans;
	}
	friend String operator+(const char*a,const String&b){
		return String(a)+b;
	}
	void write()const{
		const char*x=s;
		while(*x)putchar(*x++);
	}
};
namespace Ostream{//输出命名空间
	template<typename T>
	void unsigned_write(T&&x){
		if(x>9)unsigned_write(x/10);
		putchar(x%10^48);
	}
	template<typename T>
	inline void write(T x){
		if(x<0){
			putchar(45);
			x=-x;
		}
		if(x>9)unsigned_write(x/10);
		putchar(x%10^48);
	}
	inline void write(const char&x){putchar(x);}
	inline void write(const char*x){while(*x)putchar(*x++);}
	inline void write(const String&x){x.write();}
}
std::mutex out;
template<typename... Ar>
inline void write(const Ar&...x){
	out.lock();//为防止多线程输出混乱，给输出加上互斥锁，保证一次只有一个线程在输出
	std::initializer_list<int>{(Ostream::write(x),0)...};//但g++的编译信息还是可能与线程信息同时输出
	out.unlock();
}//快写

//------------------------------
//以下信息可能需要改动
constexpr int
n=2,//需对拍的程序数量
time_limit=3000,//时间限制
compile_thread_limit=3,//编译线程限制
run_thread_limit=5;//运行进程限制，Win7及以下的系统请改为1

//下面是需填写的程序名（不要带扩展名）
//请以字符串形式填写
const String
data_maker="data_maker",//数据生成器
test[n]={"test1","test2"},//需对拍的程序
order="-std=c++14 -Wall -Wl,-stack=134217728";//编译命令,建议不要加-O2
//------------------------------

enum Exit_mode{clean_data=1};

constexpr WORD
Dark=MAKEWORD(0,0),
Default=MAKEWORD(7,0),//默认颜色
AC_col=MAKEWORD(10,0),//AC颜色
WA_col=MAKEWORD(12,0),//WA颜色
RE_col=MAKEWORD(13,0),//RE颜色
TLE_col=MAKEWORD(14,0),//TLE颜色
UKE_col=MAKEWORD(9,0),//UKE颜色
Light=MAKEWORD(15,0);//高亮颜色

String make_data=data_maker+".exe > .data_next.in",run[n],check[n];
int cnt,i,err,list_RE[n],list_TLE[n],list_UKE[n];
std::atomic<int>compile_thread(compile_thread_limit),run_thread(run_thread_limit),cnt_RE,cnt_TLE,cnt_UKE;
std::future<int>result[n],data_result;
std::future<void>Wait[n];
decltype(std::chrono::system_clock::now()) st[n];
std::chrono::milliseconds spend[n];
HANDLE hconsole=GetStdHandle(STD_OUTPUT_HANDLE);//获得标准输出设备的句柄
inline void set_color(const WORD&col){
	SetConsoleTextAttribute(hconsole,col);
}
inline int Compile(const String&x){//编译程序
	if(system("g++ "+x+".cpp -o "+x+".exe "+order)){
		write("Failed to compile ",x,".cpp \n");
		compile_thread++;
		return -1;
	}
	write("Compile ",x,".cpp successfully\n");
	compile_thread++;
	return 0;
}
inline int Run(const int&i){
	st[i]=std::chrono::system_clock::now();
	return system(run[i]);
}
inline int Make_data(){
	int&&ret=system(make_data);
	run_thread++;
	return ret;
}
inline void Judge(const int&i){
	result[i]=std::async(std::launch::async,Run,i);//运行程序
	switch(result[i].wait_for(std::chrono::milliseconds(time_limit))){//等待直到程序结束或超时
		case std::future_status::ready://程序结束
			if(result[i].get()){//进程返回值不是0
				write(test[i],": Runtime Error\n");
				list_RE[cnt_RE++]=i;
			}
			else{
				spend[i]=std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now()-st[i]);
				write(test[i],": spent ",spend[i].count()," ms.\n");
			}
			break;
		case std::future_status::timeout://超时
			write(test[i],": Time Limit Exceeded\n");
			system("taskkill /f /im "+test[i]+".exe\n");
			list_TLE[cnt_TLE++]=i;
			break;
		default://其它情况（如进程未启动）
			write(test[i],": Unknown Error\n");
			system("taskkill /f /im "+test[i]+".exe\n");
			list_UKE[cnt_UKE++]=i;
	}
	run_thread++;
}
inline void Exit(const int&mode=0){
	if(mode&clean_data){
		set_color(Dark);
		system("taskkill /f /im "+data_maker+".exe");
		system("del .data_next.in");
	}
	set_color(Default);
	HWND hWnd=GetConsoleWindow();//获取当前控制台窗口句柄
	ShowWindow(hWnd,SW_SHOWNORMAL);//显示窗口（退出最小化）
	SetWindowPos(hWnd,HWND_TOP,0,0,800,450,0);//将窗口置于顶层的指定位置
	write("Please press Enter to continue...");
	getchar();
	exit(0);
}
int main(){
	set_color(Dark);
	if(system("g++ -v")){//未添加命令行编译语句，建议去官网下一个编译器
		set_color(Light);
		write("Complie Error\nDid you download Complier?\nPlease download GCC and check your Path.\n");
		Exit();
	}
	system("del .data.in /f /q");
	system("taskkill /f /im "+data_maker+".exe");
	for(i=0;i<n;i++)system("taskkill /f /im "+test[i]+".exe");
	system("cls");
	set_color(Light);
	write("Compiling...\n");
	for(i=0;i<n;i++){
		compile_thread--;
		result[i]=async(std::launch::async,Compile,test[i]);//启动一个新的编译线程
		while(compile_thread<1)Sleep(10);//当前进程数量达到上限，等待
	}
	data_result=std::async(std::launch::async,Compile,data_maker);//编译数据生成器
	for(i=0;i<n;i++)
		if(result[i].get())
			err=1;
	if(data_result.get()||err){
		set_color(Light);
		write("Compilation failed\n");
		Exit();
	}
	set_color(Light);
	write("Compilation succeed\n");
	data_result=std::async(std::launch::async,system,make_data);//生成第一组数据
	for(i=0;i<n;i++)
		run[i]=test[i]+".exe < .data.in > "+test[i]+".out";//运行用字符串
	for(i=0;i<n-1;i++)
		check[i]="fc "+test[i]+".out "+test[i+1]+".out";//检查用字符串
	while(true){
		cnt++;
		system("cls");//清空屏幕
		set_color(Light);
		write("Test#",cnt,":\nMaking data");
		for(i=1;data_result.wait_for(std::chrono::seconds(1))!=std::future_status::ready;i=(i+1)&3)//等待数据生成
			if(i)write('.');
			else{
				i=1;
				write("\b\b");
			}
		if(data_result.get()){//数据生成器RE了
			write("\b\b\b\b\b\b\b\b\b\b\b\b\b\bMake data unsuccessfully\n");
			Exit(clean_data);
		}
		write("\b\b\b\b\b\b\b\b\b\b\b\b\b\bMake data successfully\n");
		system("ren .data_next.in .data.in");
		run_thread--;
		data_result=std::async(std::launch::async,Make_data);//生成数据
		for(i=0;i<n;i++){
			while(run_thread<1)Sleep(1);
			run_thread--;
			Wait[i]=std::async(std::launch::async,Judge,i);//运行
		}
		for(i=0;i<n;i++)
			Wait[i].wait();
		if(cnt_RE||cnt_TLE||cnt_UKE){//输出信息
			set_color(WA_col);
			write("\nUnaccepted on test#",cnt,'\n');
			if(cnt_RE){
				set_color(RE_col);
				write("Runtime Error: ");
				set_color(Light);
				for(i=0;i<cnt_RE;i++)
					write(test[list_RE[i]],' ');
				write('\n');
			}
			if(cnt_TLE){
				set_color(TLE_col);
				write("Time Limit Exceeded: ");
				set_color(Light);
				for(i=0;i<cnt_TLE;i++)
					write(test[list_TLE[i]],' ');
				write('\n');
			}
			if(cnt_UKE){
				set_color(UKE_col);
				write("Unknown Error:\n");
				set_color(Light);
				for(i=0;i<cnt_UKE;i++)
					write(test[list_UKE[i]],' ');
				write('\n');
			}
			Exit(clean_data);
		}
		set_color(Default);
		for(i=0;i<n-1;i++)
			if(system(check[i])){//检查
				set_color(WA_col);
				write("\nUnaccepted on test#",cnt,"\nWrong Answer!\n");
				Exit(clean_data);
			}
		set_color(AC_col);
		write("\nAccepted!\n");
		system("del .data.in /f /q");
		//Sleep(1000);
	}
	set_color(RE_col);
	write("Checker Runtime Error!");
	Exit();
}
