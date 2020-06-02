#include <iostream>
#include <fstream>
#include <sstream>
#include <string>
#include <vector>
#include <map>
#include <stdlib.h>
#include <string.h>
#include <pthread.h>
#include <unistd.h>
#include <limits.h>
#include <time.h>
#include <algorithm>
#include "sockop.h"
#define BUF_SIZE 1024
using namespace std;

bool log_static[100] = {false};
string my_id[100] = {""};
vector<string> log_id;
int buy_succ = 0;

struct save_area{
    vector<int>start;
    vector<int>end;
}save_areas;

class City{
public:
    int id;
    int sum;
    string name;
    int mask[3];
};

class Store{
public:
    int num;
    string name;
    map<int, City> cities_map;
};

class DB{
public:
    int num;
    int day;
    int alcohol;
    int ori_alcohol;
    map<string, Store> stores_map;
    map<string, bool> buy_list;
};


struct Log_file{
    int index;
    int day;
    int total_day;
    int connfd;
    string command;
    string results;
};

void read_file(string argv);
vector<Log_file> read_log_file(string argv);
bool judge_correction(string msg);
string int2string(int input);
string show2string();
string replace(string str, string tar, string c);
bool judge_day(string id);
bool check_history(string id);
void read_command(string filename);
timespec diff(timespec start, timespec end);

bool COUT_FLAG = true;
bool N_FLAG = false;
string logfile_name;
DB mydata;
string server, port;
vector<string> commands[30];
double timeStart, timeEnd;
timespec time1, time2;



bool city_compare(City a, City b) {
  return a.sum > b.sum;
}

void correct_answer(ofstream &file, int index, string answer, string wrong){
    file<<"------------------------------------------"<<endl;
    file<<"index :"<<index<<endl;
    file<<"the correction answer is : \n"<<answer<<"|"<<endl<<endl;
    file<<"your answer is : \n"<<wrong<<"|"<<endl;
    file<<"------------------------------------------"<<endl;
}

void check_1(vector<Log_file> log_files, ofstream &error_file){
    string command;
    int count = 0;
    // int not_check=5;
    for(int i=0 ; i<log_files.size() ; i++){
        command = log_files[i].command;
        // if(mydata.day!=log_files[i].day && not_check == 0){
        //     correct_answer(error_file, log_files[i].index,"Day "+int2string(mydata.day), "Day "+int2string(log_files[i].day));
        //     count++;
        //     //break;
        // }
        if(command.compare(0, 6, "login ") == 0){
            string msg = command.substr(6);

            if( judge_correction(msg) && log_static[log_files[i].connfd] == 0 ){
                if( log_files[i].results.compare("Login successful.") != 0 ){
                    count++;
                    correct_answer(error_file, log_files[i].index, "Login successful.", log_files[i].results);
                    //break;
                }
                my_id[log_files[i].connfd] = msg;
                log_static[log_files[i].connfd] = true;
            }
            else{
                if( log_files[i].results.compare("Login failed.") != 0 ){
                    count++;
                    correct_answer(error_file, log_files[i].index, "Login failed.", log_files[i].results);
                    //break;
                }
            }
        }
        else if(command.compare("logout") == 0){
            if(log_static[log_files[i].connfd] == false){
                if( log_files[i].results.compare("Logout failed.") != 0 ){
                    count++;
                    correct_answer(error_file, log_files[i].index, "Logout failed.", log_files[i].results);
                    //break;
                }
            }
            else{
                if( log_files[i].results.compare("Logout successful.") != 0 ){
                    count++;
                    correct_answer(error_file, log_files[i].index,"Logout successful.", log_files[i].results);
                    //break;
                }
                log_static[log_files[i].connfd] = false;

                mydata.buy_list[ my_id[log_files[i].connfd] ] = true;
                log_id.push_back( my_id[log_files[i].connfd] );

                my_id[log_files[i].connfd] = "";
            }
        }
        else if(command.compare(0, 4, "buy ") == 0){
            vector<string>buy_msg;
            string tmp;
            stringstream ss(command.substr(4));
            int connfd = log_files[i].connfd;
            int day = mydata.day;
            string id = my_id[connfd];
            bool buy_fail = true;

            string store;
            int city_idx, mask[3];
            ss>>store>>city_idx>>mask[0]>>mask[1]>>mask[2];
            
            if( log_static[connfd] && ((day==7) || (day%2==1 && (id[9]-'0')%2==1) || (day%2==0 && (id[9]-'0')%2==0)) && (mydata.buy_list.find(id) == mydata.buy_list.end()) ){
            
                if( ((mask[0] + mask[1] + mask[2]) <= 3) && mask[0]>=0 && mask[1]>=0 && mask[2]>=0){
            
                    if( mydata.stores_map.find(store) != mydata.stores_map.end() ){
            
                        if( mydata.stores_map[store].cities_map.find(city_idx) != mydata.stores_map[store].cities_map.end() ){

                            City * ptr = &(mydata.stores_map[store].cities_map[city_idx]);
            
                            if( (ptr->mask[0] >= mask[0]) && (ptr->mask[1] >= mask[1]) && (ptr->mask[2] >= mask[2])){
            
                                mydata.alcohol--;
                                if(mydata.alcohol < 5){
                                    mydata.alcohol = mydata.ori_alcohol;
                                }
                                ptr->mask[0] -= mask[0];
                                ptr->mask[1] -= mask[1];
                                ptr->mask[2] -= mask[2];
                                
                                mydata.buy_list[id] = true;
                                log_static[connfd] = false;
                                mydata.day++;
                                buy_fail = false;
                                if( mydata.day > 7 ){
                                    mydata.day = 1;
                                    mydata.buy_list.clear();
                                }
                            }
                        }
                    }
                }
            }


            if(buy_fail){
                if( log_files[i].results.compare("Buy failed.") != 0 ){
                    count++;
                    correct_answer(error_file, log_files[i].index,"Buy failed.", log_files[i].results);
                    //break;
                }
            }
            else{
                save_areas.end.push_back(i);
                for(int j = i-1 ; j>=0 ; j--){
                    if(log_files[j].connfd == log_files[i].connfd){
                        save_areas.start.push_back(j);
                        break;
                    }
                }
                if( log_files[i].results.compare("Disinfecting hands with alcohol. Putting masks into envelope. Thank you for your purchase.") != 0){
                    count++;
                    correct_answer(error_file, log_files[i].index,"Disinfecting hands with alcohol. Putting masks into envelope. Thank you for your purchase.", log_files[i].results);
                    //break;
                }
            }
        }
        else if(command.compare("show") == 0){

        }
        else if(command.compare("statics") == 0){

        }
        else{
            if( log_files[i].results.compare("Input format not valid.") != 0){
                count++;
                correct_answer(error_file, log_files[i].index,"Input format not valid.", log_files[i].results);
                //break;
            }
        }
    }
}

bool check_flag(int index){
    for(int i=0 ; i<save_areas.start.size() ; i++){
        if(index > save_areas.start[i] && index<save_areas.end[i])
            return false;
    }
    return true;
}

void check_2(vector<Log_file> log_files, ofstream &error_file){
    string command;
    int count = 0;
    int buysucc = 0;
    // int not_check=5;
    for(int i=0 ; i<log_files.size() ; i++){
        command = log_files[i].command;
        // if(mydata.day!=log_files[i].day && not_check == 0){
        //     correct_answer(error_file, log_files[i].index,"Day "+int2string(mydata.day), "Day "+int2string(log_files[i].day));
        //     count++;
        //     //break;
        // }
        if(command.compare(0, 6, "login ") == 0){
            string msg = command.substr(6);

            if( judge_correction(msg) && log_static[log_files[i].connfd] == 0 ){
                if( log_files[i].results.compare("Login successful.") != 0 ){
                    count++;
                    correct_answer(error_file, log_files[i].index, "Login successful.", log_files[i].results);
                    //break;
                }
                my_id[log_files[i].connfd] = msg;
                log_static[log_files[i].connfd] = true;
            }
            else{
                if( log_files[i].results.compare("Login failed.") != 0 ){
                    count++;
                    correct_answer(error_file, log_files[i].index, "Login failed.", log_files[i].results);
                    //break;
                }
            }
        }
        else if(command.compare("logout") == 0){
            if(log_static[log_files[i].connfd] == false){
                if( log_files[i].results.compare("Logout failed.") != 0 ){
                    count++;
                    correct_answer(error_file, log_files[i].index, "Logout failed.", log_files[i].results);
                    //break;
                }
            }
            else{
                if( log_files[i].results.compare("Logout successful.") != 0 ){
                    count++;
                    correct_answer(error_file, log_files[i].index,"Logout successful.", log_files[i].results);
                    //break;
                }
                log_static[log_files[i].connfd] = false;

                mydata.buy_list[ my_id[log_files[i].connfd] ] = true;
                log_id.push_back( my_id[log_files[i].connfd] );

                my_id[log_files[i].connfd] = "";
            }
        }
        else if(command.compare(0, 4, "buy ") == 0){
            vector<string>buy_msg;
            string tmp;
            stringstream ss(command.substr(4));
            int connfd = log_files[i].connfd;
            int day = mydata.day;
            string id = my_id[connfd];
            bool buy_fail = true;

            string store;
            int city_idx, mask[3], k = 0;
            ss>>store>>city_idx>>mask[0]>>mask[1]>>mask[2];
            
            if( log_static[connfd] && ((day==7) || (day%2==1 && (id[9]-'0')%2==1) || (day%2==0 && (id[9]-'0')%2==0)) && (mydata.buy_list.find(id) == mydata.buy_list.end()) ){
            
                if( ((mask[0] + mask[1] + mask[2]) <= 3) && mask[0]>=0 && mask[1]>=0 && mask[2]>=0){
            
                    if( mydata.stores_map.find(store) != mydata.stores_map.end() ){
            
                        if( mydata.stores_map[store].cities_map.find(city_idx) != mydata.stores_map[store].cities_map.end() ){

                            City * ptr = &(mydata.stores_map[store].cities_map[city_idx]);
            
                            if( (ptr->mask[0] >= mask[0]) && (ptr->mask[1] >= mask[1]) && (ptr->mask[2] >= mask[2])){
            
                                mydata.alcohol--;
                                if(mydata.alcohol < 5){
                                    mydata.alcohol = mydata.ori_alcohol;
                                }
                                ptr->mask[0] -= mask[0];
                                ptr->mask[1] -= mask[1];
                                ptr->mask[2] -= mask[2];
                                
                                mydata.buy_list[id] = true;
                                log_static[connfd] = false;
                                mydata.day++;
                                buysucc++;
                                buy_fail = false;
                                if( mydata.day > 7 ){
                                    mydata.day = 1;
                                    mydata.buy_list.clear();
                                }
                            }
                            else{
                                k = 1;
                                // cerr<<k;
                            }
                        }
                            else{
                            	k = 2;
                                // cerr<<k;
                            }
                    }
                            else{
                            	k = 3;
                                // cerr<<k;
                            }
                }
                            else{
                            	k = 4;
                                // cerr<<k;
                            }
            }
                            else{
                            	k = 5;
                                // cerr<<k;
                            }

            if(buy_fail){
                if( log_files[i].results.compare("Buy failed.") != 0 ){
                    count++;
                    correct_answer(error_file, log_files[i].index,"Buy failed." , log_files[i].results);
                    //break;
                }
            }
            else{
                save_areas.end.push_back(i);
                for(int j = i-1 ; j>=0 ; j--){
                    if(log_files[j].connfd == log_files[i].connfd){
                        save_areas.start.push_back(j);
                        break;
                    }
                }
                if( log_files[i].results.compare("Disinfecting hands with alcohol. Putting masks into envelope. Thank you for your purchase.") != 0){
                    count++;
                    correct_answer(error_file, log_files[i].index,"Disinfecting hands with alcohol. Putting masks into envelope. Thank you for your purchase.", log_files[i].results);
                    //break;
                }
            }
        }

        else if(command.compare("show") == 0){
            string s = show2string();
            string msg = log_files[i].results;
            if(msg.compare(0, s.length(), s) && check_flag(i)){
                count++;
                correct_answer(error_file, log_files[i].index, s, msg);
                //break;
            }
        }
        else if(command.compare("statics") == 0){
            int x = atoi(log_files[i].results.c_str());
            if(x != mydata.alcohol && check_flag(i)){
                count++;
                correct_answer(error_file, log_files[i].index,int2string(mydata.alcohol), int2string(x));
                //break;
            }
        }
        else{
            if( log_files[i].results.compare("Input format not valid.") != 0){
                count++;
                correct_answer(error_file, log_files[i].index,"Input format not valid.", log_files[i].results);
                //break;
            }
        }
    }

    cerr<<"Buy successful :"<<buysucc<<endl;
    if(count  == 0){
        cerr<<"Congratulations!!! All "<<log_files.size()<<" commands are all correct!!!"<<endl;
    }
    else if (count == 1){
        cerr<<"There is "<<count<<" error in "<<log_files.size()<<" commands."<<endl;
        cerr<<"Maybe you can find it ><"<<endl;
    }
    else{
        cerr<<"There are "<<count<<" errors in "<<log_files.size()<<" commands QQ."<<endl;
        cerr<<"Please keep on going and never give up."<<endl;
    }
}

void clear(){
    log_id.clear();
    for(int i=0 ; i<100 ; i++){
        log_static[i] = 0;
        my_id[i] = "";
    }
}



int myclient(){
    int connfd[30], total = 0;
    int closed_fd = 0, total_con = 0;
    for(int i = 0 ; i < 30 ; i++){
        if(!commands[i].empty()){
            connfd[i] = connectsock(server.c_str(), port.c_str(), "tcp");
            total_con++;
        }
        else{
            connfd[i] = -1;
        }
    }
    while(closed_fd < total_con){
        // cout<<closed_fd<<endl;
        for(int i = 0 ; i < 30 ; i++){
            if(!commands[i].empty() && connfd[i] != -1){
                string snd;
                snd = commands[i][0];
                if(N_FLAG)
                    snd += "\n";
                commands[i].erase(commands[i].begin()); 
                if(write(connfd[i], snd.c_str(), snd.length()) < 0){
                    perror("Error write :");
                    cerr<<"Error "<<connfd[i]<<endl;
                }
                total++;
                if(COUT_FLAG)
                    cerr<<i<<" "<<connfd[i]<<" :"<<snd<<endl;
            }
            else if(connfd[i] != -1){
                close(connfd[i]);
                // cout<<closed_fd<<" "<<connfd[i]<<" disconnect\n";
                connfd[i] = -1;
                closed_fd++;
            }
            // usleep(10);
        }
        for(int i = 0 ; i < 30 ; i++){

            if(connfd[i] != -1){
                char rcv[BUF_SIZE];
                memset(rcv, 0, BUF_SIZE);
                int n = read(connfd[i], rcv, BUF_SIZE);
                if(n == 0){
                    cout<<connfd[i]<<" disconnect\n";
                    close(connfd[i]);
                    connfd[i] = -1;
                    closed_fd++;
                    break;
                }
                else if(n == -1){
                    perror("Error read :");
                    break;
                }
                string msg (rcv);
                if(COUT_FLAG && rcv[0] != 'S'){
                    // cerr<<connfd[i]<<" readed\n";
                    cout<<connfd[i]<<" "<<msg<<endl;
                }
                if( msg.find("Disinfecting") != -1){
                    buy_succ ++;
                    while((msg.find("purchase.") == -1)){
                        memset(rcv, 0, BUF_SIZE);
                        n = read(connfd[i], rcv, BUF_SIZE);
                        if(n == 0){
                            cout<<connfd[i]<<" disconnect\n";
                            close(connfd[i]);
                            connfd[i] = -1;
                            closed_fd++;

                            break;
                        }
                        else if(n == -1){
                            perror("Error read :");
                            break;
                        }
                        msg = rcv;
                        if(COUT_FLAG && rcv[0] != 'S'){
                            cout<<connfd[i]<<" "<<msg<<endl;
                        }
                    }
                }
            }
        }
    }
    return total;
}


void city_sort(){

    for ( map<string, Store>::iterator it = mydata.stores_map.begin() ; it != mydata.stores_map.end(); it++){
        cout<<it->first<<endl;
        City arr[it->second.cities_map.size()];
        int i = 0;
        for ( map<int, City>::iterator it_2 = it->second.cities_map.begin(); it_2 != it->second.cities_map.end(); it_2++){
            it_2->second.sum = it_2->second.mask[0] + it_2->second.mask[1] + it_2->second.mask[2];
            arr[i] = it_2->second;
            i++;
        }
        sort(arr, (arr + it->second.cities_map.size()), city_compare);
        for (i = 0; i < it->second.cities_map.size() ; i++){
            cout<<arr[i].id<<" "<<arr[i].name<<" "<<arr[i].sum<<endl;
        }
    }
}

int main(int argc, char *argv[]){
    clock_gettime(CLOCK_REALTIME, & time1);
    if(argc == 9){
        if( !strcmp(argv[6], "false")){
            COUT_FLAG = false;
        }
        else{
            COUT_FLAG = true;
        }
        logfile_name = argv[7];
        N_FLAG = true;
    }
    else if(argc == 8){
        if( !strcmp(argv[6], "false")){
            COUT_FLAG = false;
        }
        else{
            COUT_FLAG = true;
        }
        logfile_name = argv[7];
    }
    else if(argc == 7){
        if( !strcmp(argv[6], "false") ){
            COUT_FLAG = false;
        }
        else{
            COUT_FLAG = true;
        }
    }
    else if(argc != 6){
        cout<<"Please enter correct input: ./checker <ip> <port> <day of week> <alcohol amount> <command file>\n";
        exit(1);
    }
    // signal(SIGINT, handler);
    server = argv[1];
    port = argv[2];
    read_command(argv[5]);
    int total_cmd = myclient();

    clock_gettime(CLOCK_REALTIME, & time2);
    cout << "\nUse " << diff(time1, time2).tv_sec << "." << diff(time1, time2).tv_nsec <<" s to process "<<total_cmd<<" commands"<<endl;
    cerr<<"Buy successful :"<<buy_succ<<endl;
    cout << "Please stop the server, and press enter.\n";
    
    string str;
    getline (cin, str);
    // perror("Error :");
    // cerr<<"|"<<str<<"|\n";
    
    mydata.alcohol = atoi(argv[4]);
    mydata.ori_alcohol = atoi(argv[4]);
    mydata.day = atoi(argv[3]);
    read_file("inventory.txt");
    DB temp = mydata;

    vector<Log_file> log_files = read_log_file("result.txt");
    //check_log_file(log_files);

    ofstream error_file;
    error_file.open("error_log.txt", ifstream::out);
    if (!error_file.is_open()){
        perror("Can't open error file.\n");
        exit(1);
    }

    check_1(log_files, error_file);
    //cout<<save_areas.start.size()<<endl;
    clear();
    mydata = temp;
    check_2(log_files, error_file);
    error_file.close();
    city_sort();
    return 0;
}






















void read_command(string filename){
    ifstream file;
    file.open(filename.c_str(), ifstream::in);
    if (!file){
        cerr<<"Error opening file.\n";
        exit(1);
    }
    while(!file.eof()){
        string str;
        getline(file, str);
        if(file.eof()){
            break;
        }
        stringstream ss(str);
        string idx, client_idx, cmd;
        getline(ss, idx, ',');
        getline(ss, client_idx, ',');
        getline(ss, cmd, ',');
        commands[atoi(client_idx.c_str())].push_back(cmd);
        // cout<<"/ "<<idx<<" / "<<client_idx<<" / "<<cmd<<endl;
    }
    file.close();
}

void read_file(string argv){
    ifstream read_file;
    read_file.open(argv.c_str(), ifstream::in);
    if (!read_file.is_open()){
        perror("Can't open input file.\n");
        exit(1);
    }
    read_file >> mydata.num;
    for (int i = 0; i < mydata.num; i++){
        string data;
        Store temp_store;
        read_file >> temp_store.name;
        read_file >> temp_store.num;
        for (int j = 0; j < temp_store.num; j++){
            City temp_city;
            read_file >> temp_city.id;
            read_file >> temp_city.name;
            for (int k = 0; k < 3; k++){
                read_file >> temp_city.mask[k];
            }
            // temp_store.citys.push_back(temp_city);
            temp_store.cities_map[temp_city.id] = temp_city;
        }
        // mydata.stores.push_back(temp_store);
        mydata.stores_map[temp_store.name] = temp_store;
    }
}

vector<Log_file> read_log_file(string argv){
    ifstream file;
    vector<Log_file> log;
    char *p;
    file.open(argv.c_str(), ifstream::in);
    if (!file.is_open()){
        perror("Can't open log file.\n");
        exit(1);
    }
    while (!file.eof()){

        Log_file temp;

        string str;
        getline(file, str);
        if(file.eof()){
            break;
        }
        stringstream ss(str);
        getline(ss, str, ',');
        temp.index = atoi(str.c_str());
        getline(ss, str, ',');
        temp.day = atoi(str.c_str());
        getline(ss, str, ',');
        temp.connfd = atoi(str.c_str());
        getline(ss, temp.command, ',');
        // ss>>temp.results;
        getline(ss, temp.results, '\r');
        log.push_back(temp);
    }
    return log;
}

bool judge_correction(string msg){
    if(msg.size() >10){
        cout<<"the length is excessed 10\n";
        return 0;
    }
    int num[10];
    num[0] = msg[0]-55;
    switch(msg[0]){
        case 'A':
        case 'B':
        case 'C':
        case 'D':
        case 'E':
        case 'F':
        case 'G':
        case 'H':
        case 'W':
            num[0] = msg[0]-55;
            break;
        case 'I':
            num[0] = msg[0]-39;
            break;
        case 'J':
        case 'K':
        case 'L':
        case 'M':
        case 'N':
            num[0] = msg[0]-56;
            break;
        case 'O':
            num[0] = msg[0]-44;
            break;
        case 'P':
        case 'Q':
        case 'R':
        case 'S':
        case 'T':
        case 'U':
        case 'V':
        case 'Z':
            num[0] = msg[0]-57;
            break;
        case 'X':
        case 'Y':
            num[0] = msg[0]-58;
            break;
    }
    for(int i=1 ; i<msg.size() ; i++){
        string s;
        s.push_back(msg[i]);
        num[i] = atoi(s.c_str());
    }
    int Y = num[0]/10 + (num[0]%10)*9;
    int sum = Y;
    for(int i=1 ; i<9 ; i++){
        sum+=num[i]*(9-i);
    }
    int Z = 10-sum%10;
    if(Z == num[9])
        return 1;
    return 0;
}

string show2string(){
    stringstream ss;
    for ( map<string, Store>::iterator it = mydata.stores_map.begin() ; it != mydata.stores_map.end(); it++){
        ss << it->second.name << " ";
        for ( map<int, City>::iterator it_2 = it->second.cities_map.begin(); it_2 != it->second.cities_map.end(); it_2++){
            ss << it_2->second.id << " ";
            ss << it_2->second.name << " ";
            ss << it_2->second.mask[0] << " ";
            ss << it_2->second.mask[1] << " ";
            ss << it_2->second.mask[2] << " ";
        }
    }
    // cout<< ss.str();
    string str = ss.str();
    return str.substr(0, str.length()-1);
}

string int2string(int input){
    stringstream ss;
    string output;
    ss << input;
    ss >> output;
    return output;
}

string replace(string str, string tar, string c){
    while(true){
        int pos = str.find_first_of(tar);
        if(pos == -1){
            break;
        }
        str.replace(pos, 1, c);
    }
    return str;
}

bool judge_day(string id){
    int today = mydata.day;
    int n = id[9] - '0';
    if((today == 7) || (today%2 == 1 && n%2 == 1) || (today%2 == 0 && n%2 == 0)){
        return true;
    }
    else{
        return false;
    }
}

bool check_history(string id){
    for(int i=0 ; i<log_id.size() ; i++){
        if(id == log_id[i])
            return false;
    }
    return true;
}

timespec diff(timespec start, timespec end) {
    timespec temp;
    if ((end.tv_nsec - start.tv_nsec) < 0) {
        temp.tv_sec = end.tv_sec - start.tv_sec - 1;
        temp.tv_nsec = 1000000000 + end.tv_nsec - start.tv_nsec;
    }
    else {
        temp.tv_sec = end.tv_sec - start.tv_sec;
        temp.tv_nsec = end.tv_nsec - start.tv_nsec;
    }
    return temp;
}