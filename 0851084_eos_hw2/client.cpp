#include <iostream>
#include <stdlib.h>
#include <pthread.h>
#include <vector>
#include <string>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/types.h>
#include <sys/sem.h>
#include <time.h> 
#include <fstream>
#include <sstream>

#include "sockop.h"

#define command_num 6
#define semkey 6666
#define BUF_SIZE 1024

using namespace std;

char *server,*port;
int thread_num;
int work_num;
vector<string>log_id;

string id[30] = {"A117188556", "B174959258", "C124442496", "D127578896", "E165814285",
                 "F144839863", "G132796381", "H288868017", "J161621706", "K164296902",
                 "L157449751", "M129448656", "N177233546", "P149589598", "Q119678958",
                 "R167761572", "S142039712", "T173532954", "U171830029", "V155914441",
                 "W186714857", "X184327514", "Y191108548", "Z196589652", "O176202085",
                 "I169747354", "X184327515", "Y191108546", "Z196589657", "O176202088",};

struct city{
    int id;
    string name;
    int mask[3];
};

struct store{
    int num;
    string name;
    vector<city> citys;
};

struct file{
    int num;
    int day;
    int alcohol;
    vector<store> stores;
}mydata;

int string2int(string input){
    stringstream ss;
    int output;
    ss << input;
    ss >> output;
    return output;
}

void read_file(){
    fstream read_file;
    read_file.open("inventory.txt", ios::in);
    if (!read_file){
        cout << "can't open input file\n";
        exit(1);
    }
    string data = "";
    //stringstream ss;
    read_file >> data;
    mydata.num = string2int(data);
    for (int i = 0; i < mydata.num; i++){
        store temp_store;
        read_file >> data;
        temp_store.name = data;
        read_file >> data;
        temp_store.num = string2int(data);
        for (int j = 0; j < temp_store.num; j++){
            city temp_city;
            read_file >> data;
            temp_city.id = string2int(data);
            read_file >> data;
            temp_city.name = data;
            for (int k = 0; k < 3; k++){
                read_file >> data;
                temp_city.mask[k] = string2int(data);
            }
            temp_store.citys.push_back(temp_city);
        }
        mydata.stores.push_back(temp_store);
    }
}

int P(int sem){
    struct sembuf sop;
    sop.sem_num = 0;
    sop.sem_op = -1;
    sop.sem_flg = 0;

    if (semop(sem, &sop, 1) < 0){
        printf("enter fail\n");
        return -1;
    }
    return 0;
}

int V(int sem){
    struct sembuf sop;
    sop.sem_num = 0;
    sop.sem_op = 1;
    sop.sem_flg = 0;

    if (semop(sem, &sop, 1) < 0){
        printf("exit fail\n");
        return -1;
    }
    return 0;
}

int create_sem(){
    int sem, sem_n = 1;
    sem = semget(semkey, 1, IPC_CREAT | IPC_EXCL | 0666);
    if (sem < 0)
        printf("fail to create semaphore\n");
    if (semctl(sem, 0, SETVAL, sem_n) < 0)
        printf("fail to initialize semaphore\n");
    return sem;
}

void show_data(vector<string> read_from_server, int flag){
    if(flag == 0){
        int count = 0;
        int num = mydata.num;
        //cout<<read_from_server[0]<<endl;
        for(int i=0 ; i<num ; i++){
            cout<<read_from_server[count]<<endl;
            count++;
            int num2 = mydata.stores[i].num;
            //cout<<read_from_server[count]<<endl;
            //count++;
            for(int j=0 ; j<num2 ; j++){
                for(int k=0 ; k<5 ; k++){
                    cout<<read_from_server[count]<<" ";
                    count++;
                }
                cout<<endl;
            }
        }
    }
    else{
        for(int i=0 ; i<read_from_server.size() ; i++)
            cout<<read_from_server[i]<<" ";
        cout<<endl;
    }
}

vector<string>string2vector(char buf[BUF_SIZE]){
    string s;
    stringstream ss;
    vector<string>read_from_server;
    ss<<buf;
    while(!ss.eof()){
        ss>>s;
        read_from_server.push_back(s);
    }
    return read_from_server;
}

void show(int connfd){
    char buf[BUF_SIZE];
    string s = "show";
    write(connfd, s.c_str(), strlen(s.c_str()));
    //write(connfd, s.c_str(), 50);
    memset(buf, 0, 1024);
    read(connfd, buf, 1024);
    cout<<buf;
}

void login(int connfd){
    char buf[BUF_SIZE];
    int i = rand()%30;
    string s = "login " + id[i];
    vector<string>read_from_server;
    write(connfd, s.c_str(), strlen(s.c_str()));
    //write(connfd, s.c_str(), 50);
    memset(buf, 0, 1024);
    read(connfd, buf, 1024);
    read_from_server = string2vector(buf);
    show_data(read_from_server, 1);
    read_from_server.clear();
}

void logout(int connfd){
    char buf[BUF_SIZE];
    string s = "logout";
    vector<string>read_from_server;
    write(connfd, s.c_str(), strlen(s.c_str()));
    //write(connfd, s.c_str(), 50);
    memset(buf, 0, 1024);
    read(connfd, buf, 1024);
    read_from_server = string2vector(buf);
    show_data(read_from_server, 1);
    read_from_server.clear();
}

string int2string(int x){
    stringstream ss;
    string s;
    ss<<x;
    ss>>s;
    return s;
}

void buy(int connfd){
    char buf[BUF_SIZE];
    string s = "buy", temp;
    vector<string>read_from_server;
    int store_name = rand()%(mydata.num+1);
    if(store_name<mydata.num)
        temp = mydata.stores[store_name].name;
    else
        temp = "XXX";
    s = s + " "+ temp;
    if(temp == "XXX")
        store_name= rand()%mydata.num;

    int city_id = rand()%(mydata.stores[store_name].num+1);
    if(city_id<mydata.stores[store_name].num)
        temp = int2string(city_id+1);
    else
        temp = int2string(mydata.stores[store_name].num+1);
    s = s + " "+ temp;
    int x = rand()%11;
    if(x==0)//300
        temp =  "3 0 0";
    else if(x==1)//210
        temp =  "2 1 0";
    
    else if(x==2)//201
        temp =  "2 0 1";
    
    else if(x==3)//120
        temp =  "1 2 0";
    
    else if(x==4)//102
        temp =  "1 0 2";
    
    else if(x==5)//111
        temp =  "1 1 1";
    
    else if(x==6)//021
        temp =  "0 2 1";
    
    else if(x==7)//012
        temp =  "0 1 2";
    
    else if(x==8)//030
        temp =  "0 3 0";
    
    else if(x==9)//003
        temp =  "0 0 3";
    
    else
        temp = int2string(rand()%4)+" "+int2string(rand()%4)+" "+int2string(rand()%4);
    s = s + " "+ temp;
    cout<<"snd msg: "<<s<<endl;
    write(connfd, s.c_str(), strlen(s.c_str()));
    //write(connfd, s.c_str(), 50);
    memset(buf, 0, 1024);
    read(connfd, buf, 32);
    read_from_server = string2vector(buf);
    show_data(read_from_server, 1);
    if(strcmp(buf,"Disinfecting hands with alcohol.")==0){
        memset(buf, 0, 1024);
        read(connfd, buf, 28);
        read_from_server = string2vector(buf);
        show_data(read_from_server, 1);
        memset(buf, 0, 1024);
        read(connfd, buf, 28);
        read_from_server = string2vector(buf);
        show_data(read_from_server, 1);
    }
    read_from_server.clear();
}

void  connfd_statics(int connfd){
    char buf[BUF_SIZE];
    string s = "statics";
    vector<string>read_from_server;
    write(connfd, s.c_str(), strlen(s.c_str()));
    //write(connfd, s.c_str(), 50);
    memset(buf, 0, 1024);
    read(connfd, buf, 1024);
    read_from_server = string2vector(buf);
    show_data(read_from_server, 1);
    read_from_server.clear();
}

void connfd_exit(int connfd){
    char buf[BUF_SIZE];
    string s = "exit";
    vector<string>read_from_server;
    write(connfd, s.c_str(), strlen(s.c_str()));
    //write(connfd, s.c_str(), 50);
    memset(buf, 0, 1024);
    read(connfd, buf, 1024);
    read_from_server = string2vector(buf);
    show_data(read_from_server, 1);
    read_from_server.clear();
}

void handler(int signum) {
    exit(signum); 
}

void *myclient(void *c){
    string id;
    int connfd, command, times = work_num;
    connfd = connectsock(server, port, "tcp");
    for(int i=0 ; i<times ; i++){
        command = rand()%command_num;
        if(command == 0){//show
            show(connfd);
        }
        else if(command == 1){//login
            login(connfd);
        }
        else if(command == 2){//logout
            logout(connfd);
        }
        else if(command == 3){//buy
            buy(connfd);
        }
        else if(command == 4){//statics
            connfd_statics(connfd);
        }
        else if(command == 5){//exit
            connfd_exit(connfd);
        }
    }
    close(connfd);
    return 0;
}

int main(int argc, char *argv[]){
    srand( time(NULL) );
    signal(SIGINT, handler);
    if(argc<5){
        cout<<"please enter correct input\n";
        exit(1);
    }
    server = argv[1];
    port = argv[2];
    read_file();
    thread_num = atoi(argv[3]);
    work_num = atoi(argv[4]);
    vector<pthread_t> threads;
    string s, msg;
    int connfd;
    for(int i=0 ; i<thread_num ; i++){
        pthread_t new_th;
        threads.push_back(new_th);
        pthread_create(&threads.back(), NULL, myclient, (void *)(-1));
    }
    cin>>s;
    return 0;
}
