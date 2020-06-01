#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <stdbool.h>
#include <sys/socket.h>
#include <errno.h>
#include <strings.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include <signal.h>
#include <sys/wait.h>
#include <pthread.h>
#include <semaphore.h>
#include <string>
#include <iostream>
#include <fstream>

#define BUY_ID_NUM 100000

// declare for storing information of inventory
typedef struct Inv{
    int line, a, b, c, type_num;
    char type[20];
    char city[50];
} inv;
inv data[15][100];
char s[100];
int num;

// declare for socket
int sockfd;

// declare for day, alcohol, buied_ID, result
int day;
int alcohol = 50;
int buied_num = 0;
char buied_ID[BUY_ID_NUM][10];
std::string writeToResult;
int result_index = 0;
std::ofstream result("result.txt");

// declare for mutex
pthread_mutex_t all_mutex;


void exit_handler(int signum){

    // close socket
    close(sockfd);

    // write result to result.txt
    result << writeToResult;
    result.close();

    // destroy mutex
    pthread_mutex_destroy(&all_mutex);

}

bool check_ID(char ID[]){
    int y;
    // I, O, W, Z have different rule
    if (ID[0] == 'I'){
        y = 34;
    }
    else if (ID[0] == 'O'){
        y = 35;
    }
    else if (ID[0] == 'W'){
        y = 32;
    }
    else if (ID[0] == 'Z'){
        y = 33;
    }
    else if (ID[0] >= 'A' && ID[0] <= 'H'){
        y = ID[0] - 'A' + 10;
    }
    else if (ID[0] >= 'J' && ID[0] <= 'N'){
        y = ID[0] - 'J' + 18;
    }
    else if (ID[0] >= 'P' && ID[0] <= 'V'){
        y = ID[0] - 'P' + 23;
    } 
    else if (ID[0] == 'X'){
        y = 30;
    }
    else if (ID[0] == 'Y'){
        y = 31;
    }
    int Y = y/10 + (y%10)*9;
    int Z = 0;
    Z = 10 - (Y + (int)(ID[1] - '0')*8 + (int)(ID[2] - '0')*7 + (int)(ID[3] - '0')*6 + (int)(ID[4] - '0')*5 + (int)(ID[5] - '0')*4 + (int)(ID[6] - '0')*3 + (int)(ID[7] - '0')*2 + (int)(ID[8] - '0')*1)%10;
    if (Z == (int)(ID[9] - '0')){
        return true;
    }
    else {
        return false;
    }
}

bool check_day_of_week(char ID[]){

    // check day of week
    bool day_of_week;
    if (day == 7){
        day_of_week = true;
    }
    else if ( day%2 != ((int)(ID[9] - '0'))%2 ){
        day_of_week = false;
    }
    else {
        day_of_week = true;
    }

    // check if buied
    if (day_of_week == false){
        return false;
    }
    else {   
        printf("check ID : %s\n", ID);
        for (int i = 0; i < buied_num; i++){
            int counter = 0;
            for (int j = 0; j < 10; j++){
                if (ID[j] == buied_ID[i][j]){
                    counter++;
                }
            }
            if (counter >= 10){
                return false;
            }
        }
        return true;
    }

}

void server_show(int fd){

    std::string show_info = "show\n";
    std::string write_info;
    char tmp[30];

    for (int i = 0; i < num; ++i){
        // print type of place
        printf("%s\n", data[i][0].type);
        show_info = show_info + data[i][0].type + '\n';
        if (i == 0){
            write_info = write_info + data[i][0].type;
        }
        else {
            write_info = write_info + ' ' + data[i][0].type;
        }

        for (int j = 0; j < data[i][0].type_num; ++j)
        {
            // print mask inventory in every city
            printf("%d %s %d %d %d\n", data[i][j].line, data[i][j].city, data[i][j].a, data[i][j].b, data[i][j].c);
            snprintf(tmp, sizeof(tmp), "%d %s %d %d %d\n", data[i][j].line, data[i][j].city, data[i][j].a, data[i][j].b, data[i][j].c);
            show_info = show_info + tmp;
            snprintf(tmp, sizeof(tmp), " %d %s %d %d %d", data[i][j].line, data[i][j].city, data[i][j].a, data[i][j].b, data[i][j].c);
            write_info = write_info + tmp;
        }
    }
    
    write(fd, show_info.c_str(), strlen(show_info.c_str()));
    snprintf(tmp, sizeof(tmp), "%d,%d,%d,show,", result_index, day, fd);
    show_info = tmp;
    writeToResult = writeToResult + show_info + write_info + '\n';
    result_index++;

}

void server_login(int fd, char buf[], char loginID[], bool *iflogin){

    std::string login_succ = "Login successful.\n";
    std::string login_fail = "Login failed.\n";
    std::string login_info;
    char buf_ID[5] = {};
    char tmp_ID[10];
    char tmp[50];
    bool succ_fail = false; // true for login successful, false for login failed.

    if (*iflogin == false){
        // have not login, check day of week and if buied
        sscanf(buf, "%s %10s", buf_ID, tmp_ID);
        bool result = check_ID(tmp_ID);
        if (result == true){
            // login successful
            *iflogin = true;
            succ_fail = true;
            for (int i = 0; i < 10; i++){
                loginID[i] = tmp_ID[i];
            }
            printf("Login successful. %s\n", loginID);
            write(fd, login_succ.c_str(), strlen(login_succ.c_str()));
        }
        else {
            // login failed
            succ_fail = false;
            printf("Login failed.\n");
            write(fd, login_fail.c_str(), strlen(login_fail.c_str()));
        }
    }
    else {
        // have login, login failed
        succ_fail = false;
        printf("Login failed.\n");
        write(fd, login_fail.c_str(), strlen(login_fail.c_str()));
    }

    snprintf(tmp, sizeof(tmp), "%d,%d,%d,%s,", result_index, day, fd, buf);
    login_info = tmp;

    if (succ_fail == true){
        writeToResult = writeToResult + login_info + login_succ;
    }
    else {
        writeToResult = writeToResult + login_info + login_fail;
    }
    result_index++;

}

void server_logout(int fd, char ID[], bool *iflogin){

    printf("logout.");
    std::string logout_succ = "Logout successful.\n";
    std::string logout_fail = "Logout failed.\n";
    std::string logout_info;
    char tmp[30];
    char buf_ID[6] = {};
    printf("ID : %s\n", ID);

    snprintf(tmp, sizeof(tmp), "%d,%d,%d,logout,", result_index, day, fd);
    logout_info = tmp;

    if (*iflogin == false){
        // have not login, logout failed
        printf("Logout failed.\n");
        write(fd, logout_fail.c_str(), strlen(logout_fail.c_str()));
        writeToResult = writeToResult + logout_info + logout_fail;
    }
    else {
        // have login, logout successful, put this ID to buied_ID array
        *iflogin = false;
        for (int i = 0; i < 10; i++){
            buied_ID[buied_num][i] = ID[i];
        }
        buied_num++;
        
        printf("Logout successful.\n");
        write(fd, logout_succ.c_str(), strlen(logout_succ.c_str()));
        writeToResult = writeToResult + logout_info + logout_succ;
    }
    result_index++;

}

void server_buy(int fd, char info[], char ID[], bool *iflogin){

    printf("%s\n", info);

    std::string buy_fail = "Buy failed.\n";
    std::string disinfect = "Disinfecting hands with alcohol.";
    std::string putting_mask = "Putting masks into envelope.";
    std::string thankyou = "Thank you for your purchase.";
    std::string buy_info;
    char tmp[50];
    char buy_buf[10];
    char type[20];
    char city[1];
    int mask_num[3];
    int city_num;

    snprintf(tmp, sizeof(tmp), "%d,%d,%d,%s,", result_index, day, fd, info);
    buy_info = tmp;
    
    if (*iflogin == false){
        // have not login
        printf("Buy failed. Have not login\n");
        write(fd, buy_fail.c_str(), strlen(buy_fail.c_str()));
        buy_info = buy_info + buy_fail;
    }
    else {
        bool result = check_day_of_week(ID);
        if (result == false){
            // have login but not your day
            printf("Buy failed. not your day. Day : %d\n", day);
            write(fd, buy_fail.c_str(), strlen(buy_fail.c_str()));
            buy_info = buy_info + buy_fail;
        }
        else {
            // have login and can buy
            sscanf(info, "%s %s %s %d %d %d",buy_buf, type, city, &(mask_num[0]), &(mask_num[1]), &(mask_num[2]));
            city_num = (int)(city[0] - '0');
            if (strncmp(type, "XXX", 3) == 0){
                // no such city name, buy failed
                printf("Buy failed. No such type\n");
                write(fd, buy_fail.c_str(), strlen(buy_fail.c_str()));
                buy_info = buy_info + buy_fail;
            }
            else {
                if (mask_num[0] > 3 || mask_num[1] > 3 || mask_num[2] > 3){
                    // number of one kind of mask is bigger than 3
                    printf("Buy failed. bigger than 3\n");
                    write(fd, buy_fail.c_str(), strlen(buy_fail.c_str()));
                    buy_info = buy_info + buy_fail;
                }
                else if (mask_num[0] < 0 || mask_num[1] < 0 || mask_num[2] < 0){
                    // number of one kind of mask is smaller than 0
                    printf("Buy failed. smaller than 0\n");
                    write(fd, buy_fail.c_str(), strlen(buy_fail.c_str()));
                    buy_info = buy_info + buy_fail;
                }
                else if (mask_num[0] + mask_num[1] + mask_num[2] > 3){
                    // sum of buy mask is bigger than 3
                    printf("Buy failed. sum bigger than 3\n");
                    write(fd, buy_fail.c_str(), strlen(buy_fail.c_str()));
                    buy_info = buy_info + buy_fail;
                }
                else{
                    
                    for (int i = 0; i < num; i++){
                        if (strcmp(type, data[i][0].type) == 0){
                            // find the correct type
                            if (data[i][0].type_num < (int)(city[0] - '0')){
                                // no such city
                                printf("Buy failed. No such city.\n");
                                write(fd, buy_fail.c_str(), strlen(buy_fail.c_str()));
                                buy_info = buy_info + buy_fail;
                            }
                            else if (data[i][city_num-1].a < mask_num[0] || data[i][city_num-1].b < mask_num[1] || data[i][city_num-1].c < mask_num[2]){
                                // not enough mask inventory
                                printf("Buy failed. Not enough mask inventory\n");
                                write(fd, buy_fail.c_str(), strlen(buy_fail.c_str()));
                                buy_info = buy_info + buy_fail;
                            }
                            else {
                                // go to buy mask
                                data[i][city_num-1].a -= mask_num[0];
                                data[i][city_num-1].b -= mask_num[1];
                                data[i][city_num-1].c -= mask_num[2];

                                // Disinfecting hands with alcohol
                                printf("Disinfecting hands with alcohol.\n");
                                write(fd, disinfect.c_str(), strlen(disinfect.c_str()));                                
                                alcohol = alcohol - 1;
                                // sleep(1);
                                if (alcohol < 5){
                                    alcohol = 50;
                                }
                                
                                // Putting masks into envelope
                                printf("Putting masks into envelope.\n");
                                write(fd, putting_mask.c_str(), strlen(putting_mask.c_str()));
                                // sleep(1);

                                // Thank you for your purchase
                                printf("Thank you for your purchase.\n");
                                write(fd, thankyou.c_str(), strlen(thankyou.c_str()));
                                // sleep(1);

                                buy_info = buy_info + disinfect + ' ' + putting_mask + ' ' + thankyou + '\n';

                                // add ID to buied ID list
                                for (int i = 0; i < 10; i++){
                                    buied_ID[buied_num][i] = ID[i];
                                }
                                buied_num++;

                                // logout and move to next day
                                *iflogin = false;
                                day++;
                                if (day == 8){
                                    day = 1;
                                    for (int i = 0; i < BUY_ID_NUM; i++){
                                        memset(buied_ID[i], ' ', 10);
                                    }
                                    buied_num = 0;
                                }                               
                            }
                            break;
                        }
                    } 
                }
            }
        }
    }

    writeToResult = writeToResult + buy_info;
    result_index++;

}

void server_statics(int fd){

    std::string str;
    std::string statics_info;
    char tmp[30];
    char ch[10];

    printf("%d\n", alcohol);
    snprintf(ch, sizeof(ch), "%d\n", alcohol);
    str = ch;
    write(fd, str.c_str(), strlen(str.c_str()));
    snprintf(tmp, sizeof(tmp), "%d,%d,%d,statics,%d\n", result_index, day, fd, alcohol);
    statics_info = tmp;
    
    writeToResult = writeToResult + statics_info;
    result_index++;

}

void server_invalid(int fd){

    std::string invalid_info;
    char tmp[30];

    printf("Input format not valid.\n");
    std::string invalid = "Input format not valid.\n";
    write(fd, invalid.c_str(), strlen(invalid.c_str()));
    snprintf(tmp, sizeof(tmp), "%d,%d,%d,exit,", result_index, day, fd);
    invalid_info = tmp;

    writeToResult = writeToResult + invalid_info + invalid;
    result_index++;

}


void *mask_server(void *conn){

    bool iflogin = false;
    int fd = *((int *)conn);
    char buf[50];
    char buf_ID[5];
    char ID[10];

    while (read(fd, &buf, 30)){
        pthread_mutex_lock(&all_mutex);
        if (strncmp(buf, "show", 4) == 0){
            server_show(fd);
        }
        else if (strncmp(buf, "login", 5) == 0){
            server_login(fd, buf, ID, &iflogin);
        }
        else if (strncmp(buf, "logout", 6) == 0){
            server_logout(fd, ID, &iflogin);
        }
        else if (strncmp(buf, "buy", 3) == 0){
            server_buy(fd, buf, ID, &iflogin);
        }
        else if (strncmp(buf, "statics", 7) == 0){
            server_statics(fd);
        }
        else{
            server_invalid(fd);
        }
        memset(buf, '\0', 30);
        pthread_mutex_unlock(&all_mutex);
    }

    free(conn);
    pthread_exit(NULL);
}


int main(int argc, char *argv[]){

    signal(SIGPIPE, SIG_IGN);
    signal(SIGINT, exit_handler);

    // check if input invalid
    if(argc != 3){
        printf("Usage: %s <day_of_week> <port>\n", argv[0]);
        return 0;
    }
    day = atoi(argv[1]);

    // read information from inventory
    FILE *fp = fopen("inventory.txt", "r");
    if (fp == NULL){
        perror ("Error opening file");
    }
    fgets(s, 100, fp);
    sscanf(s, "%d", &num);

    for (int i = 0; i < num; ++i) {
        fgets(s, 100, fp);
        sscanf(s, "%s", data[i][0].type);

        fgets(s, 100, fp);
        sscanf(s, "%d", &(data[i][0].type_num));
        for (int j = 0; j < data[i][0].type_num; ++j)
        {
            fgets(s, 100, fp);
            sscanf(s, "%d %s %d %d %d", &(data[i][j].line), data[i][j].city, &(data[i][j].a), &(data[i][j].b), &(data[i][j].c));
        }
    }
    fclose(fp);

    // set buied_ID to zero
    for (int i = 0; i < BUY_ID_NUM; i++){
        for (int j = 0; j < 10; j++){
            buied_ID[i][j] = ' ';
        }
    }

    // socket
    struct sockaddr_in addr_cln;
	struct sockaddr_in servaddr;
	socklen_t sLen;
	
	if((sockfd = socket(AF_INET, SOCK_STREAM, 0))<0){
		printf("socket error\n");
		return 0;
	}
    bzero(&servaddr, sizeof(servaddr));
	servaddr.sin_family = AF_INET;
	servaddr.sin_port = htons(strtol(argv[2], NULL, 10));
	servaddr.sin_addr.s_addr = htonl(INADDR_ANY);

	int yes = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &yes, sizeof(yes));
	if(bind(sockfd, (struct sockaddr*)&servaddr, sizeof(servaddr))<0){
		printf("bind error\n");
		return 0;
	}
	if(listen(sockfd, 30)<0){
		printf("listen error\n");
		return 0;
	}

    pthread_t thread[30];
    int i = 0;

    while (true) {
        // wait for connection
        int *connfd = (int *)malloc(sizeof(int));
        if ((*connfd = accept(sockfd, (struct sockaddr *)&addr_cln, &sLen)) == -1) {
            perror("Error return from accept()");
            exit(EXIT_FAILURE);
        }

        // create thread
        if (pthread_create(&thread[i], NULL, mask_server, (void *)connfd) != 0) {
            perror("Error return from pthread_create()");
            exit(-1);
        }
        i++;
    }

    return 0;
}