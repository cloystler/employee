#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <stdlib.h>
#include <string.h>
#include <sqlite3.h>
#define ERR_MSG(msg) do{\
	fprintf(stderr,"%d",__LINE__);\
	perror(msg);\
}while(0)
#define PORT 8888
#define IP "192.168.2.72"
typedef void (*sighandler_t)(int sig);
typedef struct msg11
{
	int sign;
	int num;
	char passwd[15];
	char rights;
	int umsg;
}msg1;
typedef struct msg22
{
	int num;
	char name[128];
	char sex;
	int age; 
	char id[20];
	char telephone[20];
	char department[128];
	int salary;
}msg2;
typedef struct msg00
{
	int newsfd;
	struct sockaddr_in cin;
	msg1 m1;
	msg2 m2;
}msg0;
//信号函数
void handler(int sig)
{
	while(waitpid(-1,NULL,WNOHANG)>0);
}
void do_status(msg0* m0,sqlite3* db,char s);
//清理字符串和标志位
void do_clean(msg0* m0)
{
	m0->m1.sign=0;
	m0->m1.rights=0;
	memset(m0->m1.passwd,0,sizeof(m0->m1.passwd));
	memset(m0->m2.name,0,sizeof(m0->m2.name));
	memset(m0->m2.department,0,sizeof(m0->m2.department));
}
void do_send(msg0* m0)
{
	if(send(m0->newsfd,&m0->m1,sizeof(msg1),0)<0)
	{
		ERR_MSG("send");
		exit(0);
	}
	//查询和修改时额外发送完整版员工信息
	if((m0->m1.sign==5)||(m0->m1.sign==6)||(m0->m1.sign==9))
	{
		if(send(m0->newsfd,&m0->m2,sizeof(msg2),0)<0)
		{
			ERR_MSG("send");
			exit(0);
		}
	}

}
void do_recv(msg0* m0,sqlite3* db)
{
	int n=m0->m1.num;
	do_clean(m0);
	ssize_t res;
	res=recv(m0->newsfd,&m0->m1,sizeof(msg1),0);
	if(res<0)
	{
		ERR_MSG("send");
		exit(0);
	}else if(res==0)
	{
		printf("客户端异常退出\n");
		m0->m1.num=n;
		do_status(m0,db,'0');
		exit(0);
	}
	//新增和修改时额外接收完整版员工信息
	if((m0->m1.sign==3)||(m0->m1.sign==5))
	{
		res=recv(m0->newsfd,&m0->m2,sizeof(msg2),0);
		if(res<0)
		{
			ERR_MSG("send");
			exit(0);
		}else if(res==0)
		{
			printf("客户端异常退出\n");
			m0->m1.num=n;
			do_status(m0,db,'0');
			exit(0);
		}
	}

}
//初始化数据库
sqlite3* do_sql()
{
	sqlite3* db=NULL;
	char* errmsg=NULL;
	if(sqlite3_open("./employee.db",&db)!=SQLITE_OK)
	{
		ERR_MSG("sqlite3_open");
		exit(0);
	}
	char buf[256]="create table if not exists user(num int primary key,passwd char,status char,rights char)";
	if(sqlite3_exec(db,buf,NULL,NULL,&errmsg)!=0)
	{
		printf("%d-%s\n",__LINE__,errmsg);
		exit(0);
	}
	strcpy(buf,"create table if not exists umsg(num int primary key,name char,sex char,age int,id char,telephone char,department char,salary int)");
	if(sqlite3_exec(db,buf,NULL,NULL,&errmsg)!=0)
	{
		fprintf(stderr,"%d-%s\n",__LINE__,errmsg);
		exit(0);
	}
	printf("数据库打开成功\n");
	return db;
}
//判断工号是否在数据库中
int do_num(msg0* m0,sqlite3* db)
{
	char* errmsg=NULL;
	char buf[256];
	char** pres=NULL;
	int row,column;
	sprintf(buf,"select * from user where num=%d",m0->m1.num);
	if(sqlite3_get_table(db,buf,&pres,&row,&column,&errmsg)!=SQLITE_OK)
	{
		fprintf(stderr,"%d-%s\n",__LINE__,errmsg);
		exit(0);
	}
	sqlite3_free_table(pres);
	if(row<1)
		return 0;
	return 1;
}
//判断修改的工号是否在数据库中
int do_num2(msg0* m0,sqlite3* db)
{
	char* errmsg=NULL;
	char buf[256];
	char** pres=NULL;
	int row,column;
	sprintf(buf,"select * from umsg where num=%d",m0->m2.num);
	if(sqlite3_get_table(db,buf,&pres,&row,&column,&errmsg)!=SQLITE_OK)
	{
		fprintf(stderr,"%d-%s\n",__LINE__,errmsg);
		exit(0);
	}
	sqlite3_free_table(pres);
	if(row<1)
		return 0;
	return 1;
}
//判断密码是否正确
int do_passwd(msg0* m0,sqlite3* db)
{
	char* errmsg=NULL;
	char buf[256];
	char** pres=NULL;
	int row,column;
	sprintf(buf,"select * from user where num=%d and passwd=\"%s\"",m0->m1.num,m0->m1.passwd);
	if(sqlite3_get_table(db,buf,&pres,&row,&column,&errmsg)!=SQLITE_OK)
	{
		fprintf(stderr,"%d-%s\n",__LINE__,errmsg);
		exit(0);
	}
	if(row==0)
	{
		return -1;
	}
	if(*(pres[6])=='1')
	{
		return -2;
	}
	if(*(pres[7])=='1')
	{
		return 1;
	}else
	{
		return 0;
	}
	sqlite3_free_table(pres);
}
//修改登录状态
void do_status(msg0* m0,sqlite3* db,char s)
{
		char* errmsg=NULL;
		char buf[256];
		sprintf(buf,"update user set status=\"%c\" where num=%d",s,m0->m1.num);
		if(sqlite3_exec(db,buf,NULL,NULL,&errmsg)!=0)
		{
			fprintf(stderr,"%d-%s\n",__LINE__,errmsg);
			exit(0);
		}
}
//注册
void do_register(msg0* m0,sqlite3* db)
{
	if(!do_num(m0,db))
	{
		char* errmsg=NULL;
		char buf[256];
		sprintf(buf,"insert into user values('%d','%s','0','%c')",m0->m1.num,m0->m1.passwd,m0->m1.rights);
		if(sqlite3_exec(db,buf,NULL,NULL,&errmsg)!=0)
		{
			fprintf(stderr,"%d-%s\n",__LINE__,errmsg);
			exit(0);
		}
	}else
	{
		m0->m1.sign=100;//100=账户已存在
		m0->m1.num=0;//防止重复注册后通过强行关闭来重置status
	}
	do_send(m0);
}
//登录
void do_login(msg0* m0,sqlite3* db)
{
	int n;
	if(!do_num(m0,db))
	{
		m0->m1.sign=102;//102=账户不存在
		m0->m1.num=0;//防止登录失败后通过强行关闭来重置status
	}
	n=do_passwd(m0,db);
	if(n==-1)
	{
		m0->m1.sign=103;//103=密码错误
		m0->m1.num=0;
	}else if(n==-2)
	{
		m0->m1.sign=104;//104=重复登录
		m0->m1.num=0;
	}else if(n==0)
	{
		m0->m1.sign=100;//100=普通员工
		do_status(m0,db,'1');
		printf("员工%d已登录\n",m0->m1.num);
	}else if(n==1)
	{
		m0->m1.sign=101;//101=管理员
		do_status(m0,db,'1');
		printf("管理员%d已登录\n",m0->m1.num);
	}
	do_send(m0);
}
void do_back(msg0* m0,sqlite3* db)
{
	do_status(m0,db,'0');
	printf("%d退出登录\n",m0->m1.num);
	return;
}
//新增
void do_add(msg0* m0,sqlite3* db)
{
	char* errmsg=NULL;
	char buf[512];
	sprintf(buf,"insert into umsg values(%d,\"%s\",\"%c\",%d,\"%s\",\"%s\",\"%s\",%d)",m0->m2.num,m0->m2.name,m0->m2.sex,m0->m2.age,m0->m2.id,m0->m2.telephone,m0->m2.department,m0->m2.salary);
	//puts(buf);
	if(sqlite3_exec(db,buf,NULL,NULL,&errmsg)!=0)
	{
		fprintf(stderr,"%d-%s\n",__LINE__,errmsg);
		do_send(m0);
		exit(0);
	}
	m0->m1.sign=100;
	do_send(m0);
	
}
//删除
void do_del(msg0* m0,sqlite3* db)
{
	char* errmsg=NULL;
	char buf[128];
	sprintf(buf,"delete from umsg where num=%d",m0->m1.umsg);
	if(sqlite3_exec(db,buf,NULL,NULL,&errmsg)!=0)
	{
		fprintf(stderr,"%d-%s\n",__LINE__,errmsg);
		do_send(m0);
		exit(0);
	}
	m0->m1.sign=100;
	do_send(m0);
}
//修改
void do_update(msg0* m0,sqlite3* db)
{
	char* errmsg=NULL;
	char buf[256];
	switch(m0->m1.rights)
	{
		case '1':if(do_num2(m0,db))
			 {
				 m0->m1.sign=101;//101=num重复
				 do_send(m0);
				 return;
			 }
			sprintf(buf,"update umsg set num=%d where num=%d",m0->m2.num,m0->m1.umsg);
			break;
		case '2':sprintf(buf,"update umsg set name=\"%s\" where num=%d",m0->m2.name,m0->m1.umsg);
			break;
		case '3':sprintf(buf,"update umsg set sex=\"%c\" where num=%d",m0->m2.sex,m0->m1.umsg);
			break;
		case '4':sprintf(buf,"update umsg set age=%d where num=%d",m0->m2.age,m0->m1.umsg);
			break;
		case '5':sprintf(buf,"update umsg set id=\"%s\" where num=%d",m0->m2.id,m0->m1.umsg);
			break;
		case '6':sprintf(buf,"update umsg set telephone=\"%s\" where num=%d",m0->m2.telephone,m0->m1.umsg);
			break;
		case '7':sprintf(buf,"update umsg set department=\"%s\" where num=%d",m0->m2.department,m0->m1.umsg);
			break;
		case '8':sprintf(buf,"update umsg set salary=%d where num=%d",m0->m2.salary,m0->m1.umsg);
			break;
	}
	//puts(buf);
	if(sqlite3_exec(db,buf,NULL,NULL,&errmsg)!=0)
	{
		fprintf(stderr,"%d-%s\n",__LINE__,errmsg);
		do_send(m0);
		exit(0);
	}
	m0->m1.sign=100;//100=修改成功
	do_send(m0);
	
}
//查询
void do_search(msg0* m0,sqlite3* db)
{
	int i=0;
	msg2* p=&(m0->m2);
	char* errmsg=NULL;
	char buf[256];
	char** pres=NULL;
	int row,column;
	if(m0->m1.sign==6)
	{
		sprintf(buf,"select * from umsg where num=%d",m0->m1.umsg);
	}else if(m0->m1.sign==9)
	{
		sprintf(buf,"select num,name,sex,age,telephone,department from umsg where num=%d",m0->m1.umsg);
	}
	//puts(buf);
	if(sqlite3_get_table(db,buf,&pres,&row,&column,&errmsg)!=SQLITE_OK)
	{
		fprintf(stderr,"%d-%s\n",__LINE__,errmsg);
		exit(0);
	}
	if(row==0)
	{
		m0->m1.sign=100;
		do_send(m0);
		return;
	}
	if(m0->m1.sign==6)
	{	
		m0->m2.num=atoi(pres[8]);
		strcpy(m0->m2.name,pres[9]);
		m0->m2.sex=*pres[10];
		m0->m2.age=atoi(pres[11]);
		strcpy(m0->m2.id,pres[12]);
		strcpy(m0->m2.telephone,pres[13]);
		strcpy(m0->m2.department,pres[14]);
		m0->m2.salary=atoi(pres[15]);
	}else if(m0->m1.sign==9)
	{
		m0->m2.num=atoi(pres[6]);
		strcpy(m0->m2.name,pres[7]);
		m0->m2.sex=*pres[8];
		m0->m2.age=atoi(pres[9]);
		strcpy(m0->m2.telephone,pres[10]);
		strcpy(m0->m2.department,pres[11]);
	}
	do_send(m0);
	sqlite3_free_table(pres);
}
//管理员菜单
void do_menu1(msg0* m0,sqlite3* db)
{
	while(1)
	{
		do_recv(m0,db);
		switch(m0->m1.sign)
		{
			case 3:printf("增加\n");
			       do_add(m0,db);
			       break;
			case 4:printf("删除\n");
			       do_del(m0,db);
			       break;
			case 5:printf("修改\n");
			       do_update(m0,db);
			       break;
			case 6:printf("查询\n");
			       do_search(m0,db);
			       break;
			case 7:printf("返回\n");
			       do_back(m0,db);
			       return;
			default:printf("输入有误\n");
		}
	}
}
//普通员工菜单
void do_menu0(msg0* m0,sqlite3* db)
{
	while(1)
	{
		do_recv(m0,db);
		switch(m0->m1.sign)
		{
			case 9:printf("查询\n");
			       do_search(m0,db);
			       break;
			case 7:printf("返回\n");
			       do_back(m0,db);
			       return;
		}
	}
}
int main(int argc, const char *argv[])
{
	msg0 m0;
	sqlite3* db=do_sql();
	sighandler_t s=signal(17,handler);
	if(s==SIG_ERR)
	{
		ERR_MSG("signal");
		return -1;
	}
	int sfd=socket(AF_INET,SOCK_STREAM,0);
	if(sfd<0)
	{
		ERR_MSG("socket");
		return -1;
	}
	int reuse = 1;
	if(setsockopt(sfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
	{
		ERR_MSG("setsockopt");
		return -1;
	}
	struct sockaddr_in sin;
	sin.sin_family=AF_INET;
	sin.sin_port=htons(PORT);
	sin.sin_addr.s_addr=inet_addr(IP);
	if(bind(sfd,(struct sockaddr*)&sin,sizeof(sin))<0)
	{
		ERR_MSG("bind");
		return -1;
	}
	if(listen(sfd,10)<0)
	{
		ERR_MSG("listen");
		return -1;
	}
	socklen_t len=sizeof(m0.cin);
	pid_t pid;
	while(1)
	{
		m0.newsfd=accept(sfd,(struct sockaddr*)&m0.cin,&len);
		if(m0.newsfd<0)
		{
			ERR_MSG("accept");
			return -1;
		}
		pid=fork();
		if(pid==0)
		{
			close(sfd);
			while(1)
			{
				do_recv(&m0,db);
				switch(m0.m1.sign)
				{
					case 1:printf("注册\n");
					       do_register(&m0,db);
					       break;
					case 2:printf("登录\n");
					       do_login(&m0,db);
						if(m0.m1.sign==100)
						{
							do_menu0(&m0,db);
						}else if(m0.m1.sign==101)
						{
							do_menu1(&m0,db);
						}
					       break;
					case 8:printf("%d退出连接\n",m0.newsfd);
					       exit(0);
					default:printf("输入有误\n");
				}
			}
			close(m0.newsfd);
			exit(0);
		}
		close(m0.newsfd);
	}
	close(sfd);
	return 0;
}
