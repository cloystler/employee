#include <stdio.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/socket.h>
#include <arpa/inet.h>
#include <netinet/in.h>
#include <unistd.h>
#include <signal.h>
#include <string.h>
#include <stdlib.h>
#define ERR_MSG(msg) do{\
	fprintf(stderr,"%d",__LINE__);\
	perror(msg);\
}while(0)
#define PORT 8888
#define IP "192.168.2.72"
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
	int sfd;
	struct sockaddr_in sin;
	msg1 m1;
	msg2 m2;
}msg0;
void do_menu0(msg0* m0);
void do_menu1(msg0* m0);
//清理字符串和标志位
void do_clean(msg0* m0)
{
	m0->m1.sign=0;
	m0->m1.rights=0;
	memset(m0->m1.passwd,0,sizeof(m0->m1.passwd));
	memset(m0->m2.name,0,sizeof(m0->m2.name));
	memset(m0->m2.department,0,sizeof(m0->m2.department));
}
//发送函数
void do_send(msg0* m0)
{
	if(send(m0->sfd,&m0->m1,sizeof(msg1),0)<0)
	{
		ERR_MSG("send");
		exit(0);
	}
	//新增和修改时额外发送完整版员工信息
	if((m0->m1.sign==3)||(m0->m1.sign==5))
	{
		if(send(m0->sfd,&m0->m2,sizeof(msg2),0)<0)
		{
			ERR_MSG("send");
			exit(0);
		}
	}
}
//接收函数
void do_recv(msg0* m0)
{
	do_clean(m0);
	ssize_t res;
	res=recv(m0->sfd,&m0->m1,sizeof(msg1),0);
	if(res<0)
	{
		ERR_MSG("send");
		exit(0);
	}else if(res==0)
	{
		printf("服务器已关闭\n");
		exit(0);
	}
	//查询和修改时额外接收完整版员工信息
	if((m0->m1.sign==6)||(m0->m1.sign==5)||(m0->m1.sign==9))
	{
		res=recv(m0->sfd,&m0->m2,sizeof(msg2),0);
		if(res<0)
		{
			ERR_MSG("send");
			exit(0);
		}else if(res==0)
		{
			printf("服务器已关闭\n");
			exit(0);
		}
	}
}
//注册
void do_register(msg0* m0)
{
	while(1)
	{
		do_clean(m0);
		printf("请输入您的的账户类型(管理员1/普通员工0/退出9)\n");
		scanf("%c",&m0->m1.rights);
		while(getchar()!='\n');
		if(m0->m1.rights=='9')
			return;
		if((m0->m1.rights!='0')&&(m0->m1.rights!='1'))
		{
			printf("账户类型输入有误，请重新输入\n");
			continue;
		}
		printf("请输入您的工号\n");
		scanf("%d",&m0->m1.num);
		while(getchar()!='\n');
		printf("请输入您的密码\n");
		fgets(m0->m1.passwd,sizeof(m0->m1.passwd),stdin);
		m0->m1.passwd[strlen(m0->m1.passwd)-1]=0;
		m0->m1.sign=1;
		do_send(m0);
		do_recv(m0);
		if(m0->m1.sign==100)
		{
			printf("账户已存在\n");
		}else
		{
			printf("账号注册成功\n");
		}
		return;
	}
}
//登录函数
void do_login(msg0* m0)
{
	while(1)
	{
		do_clean(m0);
		printf("请输入您的工号(输入9退出)\n");
		scanf("%d",&m0->m1.num);
		while(getchar()!='\n');
		if(m0->m1.num==9)
			return;
		printf("请输入您的密码\n");
		fgets(m0->m1.passwd,sizeof(m0->m1.passwd),stdin);
		m0->m1.passwd[strlen(m0->m1.passwd)-1]=0;
		m0->m1.sign=2;
		do_send(m0);
		do_recv(m0);
		if(m0->m1.sign==102)
		{
			printf("账号不存在，请重新输入");
			continue;
		}else if(m0->m1.sign==103)
		{
			printf("输入密码有误,请重新输入\n");
			continue;
		}else if(m0->m1.sign==104)
		{
			printf("重复登录\n");
			continue;
		}
		if(m0->m1.sign==100)
		{
			printf("普通员工登录成功\n");
			do_menu0(m0);
			return;
		}else if(m0->m1.sign==101)
		{
			printf("管理员登录成功\n");
			do_menu1(m0);
			return;
		}else
		{
			ERR_MSG("login");
			exit(0);
		}
	}
}
//退出函数
void do_exit(msg0* m0)
{
	m0->m1.sign=8;
	do_send(m0);
	printf("客户端退出\n");
	exit(0);
}
//新增
void do_add(msg0* m0)
{
	printf("请输入添加对象的工号\n");
	scanf("%d",&m0->m2.num);
	while(getchar()!='\n');
	printf("请输入添加对象的姓名\n");
	fgets(m0->m2.name,sizeof(m0->m2.name),stdin);
	m0->m2.name[strlen(m0->m2.name)-1]=0;
	printf("请输入添加对象的性别\n");
	scanf("%c",&m0->m2.sex);
	while(getchar()!='\n');
	printf("请输入添加对象的年龄\n");
	scanf("%d",&m0->m2.age);
	while(getchar()!='\n');
	printf("请输入添加对象的身份证号码\n");
	fgets(m0->m2.id,sizeof(m0->m2.id),stdin);
	m0->m2.id[strlen(m0->m2.id)-1]=0;
	printf("请输入添加对象的电话号码\n");
	fgets(m0->m2.telephone,sizeof(m0->m2.telephone),stdin);
	m0->m2.telephone[strlen(m0->m2.telephone)-1]=0;
	printf("请输入添加对象的所属部门\n");
	fgets(m0->m2.department,sizeof(m0->m2.department),stdin);
	m0->m2.department[strlen(m0->m2.department)-1]=0;
	printf("请输入添加对象的工资\n");
	scanf("%d",&m0->m2.salary);
	while(getchar()!='\n');
	m0->m1.sign=3;
	do_send(m0);
	do_recv(m0);
	if(m0->m1.sign==100)
	{
		printf("添加成功\n");
	}else
	{
		printf("添加失败\n");
	}
	
}
//删除
void do_del(msg0* m0)
{
	printf("请输入需要删除的工号\n");
	scanf("%d",&m0->m1.umsg);
	while(getchar()!='\n');
	m0->m1.sign=4;
	do_send(m0);
	do_recv(m0);
	if(m0->m1.sign==100)
	{
		printf("删除成功\n");
	}else
	{
		printf("删除失败\n");
	}
}
//管理员查询
void do_search1(msg0* m0,int flag)
{	
	if(flag)
	{
		printf("请输入需要查询的工号\n");
		scanf("%d",&m0->m1.umsg);
		while(getchar()!='\n');
	}
	m0->m1.sign=6;
	do_send(m0);
	do_recv(m0);
	if(m0->m1.sign!=100)
	{
		printf("工号:%d\n",m0->m2.num);
		printf("姓名:%s\n",m0->m2.name);
		printf("性别:%c\n",m0->m2.sex);
		printf("年龄:%d\n",m0->m2.age);
		printf("身份证号:%s\n",m0->m2.id);
		printf("电话:%s\n",m0->m2.telephone);
		printf("部门:%s\n",m0->m2.department);
		printf("工资:%d\n",m0->m2.salary);
	}else
	{
		printf("未找到对应工号\n");
	}
}
//员工查询
void do_search0(msg0* m0)
{	
	m0->m1.sign=9;
	do_send(m0);
	do_recv(m0);
	if(m0->m1.sign!=100)
	{
		printf("工号:%d\n",m0->m2.num);
		printf("姓名:%s\n",m0->m2.name);
		printf("性别:%c\n",m0->m2.sex);
		printf("年龄:%d\n",m0->m2.age);
		printf("电话:%s\n",m0->m2.telephone);
		printf("部门:%s\n",m0->m2.department);
	}else
	{
		printf("未找到对应工号\n");
	}
}
//修改
void do_update(msg0* m0)
{
	int flag=0;
	printf("请输入需要修改的工号\n");
	scanf("%d",&m0->m1.umsg);
	while(getchar()!='\n');
	printf("您准备修改的员工信息如下\n");
	while(1)
	{
		do_search1(m0,0);
		if(m0->m1.sign==100)
			return;
		printf("请输入需要修改的内容\n");
		printf("-------1.工号-------\n");
		printf("-------2.姓名-------\n");
		printf("-------3.性别-------\n");
		printf("-------4.年龄-------\n");
		printf("-------5.身份证号---\n");
		printf("-------6.电话-------\n");
		printf("-------7.部门-------\n");
		printf("-------8.工资-------\n");
		printf("-------9.退出-------\n");
		scanf("%c",&m0->m1.rights);
		while(getchar()!='\n');
		switch(m0->m1.rights)
		{
			case '1':printf("请输入修改后的工号\n");
				 scanf("%d",&m0->m2.num);
				 while(getchar()!='\n');
				flag=1;
				 break;
			case '2':printf("请输入修改后的姓名\n");
				 scanf("%s",m0->m2.name);
				 while(getchar()!='\n');
				 break;
			case '3':printf("请输入修改后的性别\n");
				 scanf("%c",&m0->m2.sex);
				 while(getchar()!='\n');
				 break;
			case '4':printf("请输入修改后的年龄\n");
				 scanf("%d",&m0->m2.age);
				 while(getchar()!='\n');
				 break;
			case '5':printf("请输入修改后的身份证号\n");
				 scanf("%s",m0->m2.id);
				 while(getchar()!='\n');
				 break;
			case '6':printf("请输入修改后的电话\n");
				 scanf("%s",m0->m2.telephone);
				 while(getchar()!='\n');
				 break;
			case '7':printf("请输入修改后的部门\n");
				 scanf("%s",m0->m2.department);
				 while(getchar()!='\n');
				 break;
			case '8':printf("请输入修改后的工资\n");
				 scanf("%d",&m0->m2.salary);
				 while(getchar()!='\n');
				 break;
			case '9':return;
			default:printf("输入有误请重新输入\n");
		}
		m0->m1.sign=5;
		do_send(m0);
		do_recv(m0);
		if(m0->m1.sign==100)
		{
			printf("修改成功\n");
		}else if(m0->m1.sign==101)
		{
			printf("工号重复\n");
		}else
		{
			printf("修改失败\n");
		}
		if((flag!=0)&&(m0->m1.sign==100))
		{
			m0->m1.umsg=m0->m2.num;
			flag=0;
		}
		printf("目前您修改的员工信息如下\n");
	}
}
//返回
void do_back(msg0* m0)
{
	m0->m1.sign=7;
	do_send(m0);
}
//二级菜单(普通员工)
void do_menu0(msg0* m0)
{
	char n;
	while(1)
	{
		printf("-----1.查询-----\n");
		printf("-----2.返回-----\n");
		scanf("%c",&n);
		while(getchar()!='\n');
		switch(n)
		{
			case '1':printf("请输入需要查询的工号\n");
				scanf("%d",&m0->m1.umsg);
				while(getchar()!='\n');
				do_search0(m0);
				break;
			case '2':do_back(m0);return;
			default :printf("输入有误，请重新输入\n");
		}
	}
}
//二级菜单(管理员)
void do_menu1(msg0* m0)
{
	char n;
	while(1)
	{
		printf("-----1.增加-----\n");
		printf("-----2.删除-----\n");
		printf("-----3.修改-----\n");
		printf("-----4.查询-----\n");
		printf("-----5.返回-----\n");
		scanf("%c",&n);
		while(getchar()!='\n');
		switch(n)
		{
			case '1':do_add(m0);break;
			case '2':do_del(m0);break;
			case '3':do_update(m0);break;
			case '4':do_search1(m0,1);break;
			case '5':do_back(m0);return;
			default :printf("输入有误，请重新输入\n");
		}
		
	}
}
int main(int argc, const char *argv[])
{
	msg0 m0;
	char n;
	m0.sfd=socket(AF_INET,SOCK_STREAM,0);
	if(m0.sfd<0)
	{
		ERR_MSG("socket");
		return -1;
	}
	int reuse = 1;
	if(setsockopt(m0.sfd, SOL_SOCKET, SO_REUSEADDR, &reuse, sizeof(reuse)) < 0)
	{
		ERR_MSG("setsockopt");
		return -1;
	}
	m0.sin.sin_family=AF_INET;
	m0.sin.sin_port=htons(PORT);
	m0.sin.sin_addr.s_addr=inet_addr(IP);
	if(connect(m0.sfd,(struct sockaddr*)&m0.sin,sizeof(m0.sin))<0)
	{
		ERR_MSG("connect");
		return -1;
	}
	while(1)
	{
	//客户端一级子菜单
		printf("-----1.注册-----\n");
		printf("-----2.登录-----\n");
		printf("-----3.退出-----\n");
		scanf("%c",&n);
		while(getchar()!='\n');
		switch(n)
		{
			case '1':do_register(&m0);break;
			case '2':do_login(&m0);break;
			case '3':do_exit(&m0);break;
			default :printf("输入有误，请重新输入\n");
		}
	}
	close(m0.sfd);
	return 0;
}
