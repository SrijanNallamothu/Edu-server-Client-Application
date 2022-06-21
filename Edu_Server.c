// IMT2019524

// Compilation of this File - gcc -g Edu_server.c -lrt -D_REENTRANT -lpthread -o Edu_server

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/types.h>
#include <semaphore.h>
#include <fcntl.h>
#include <sys/stat.h>
#include <mqueue.h>
#include <unistd.h>
#include <pthread.h>
#include <signal.h>

#define MSG_VAL_LEN 16

// For the client queue message

#define CLIENT_Q_NAME_LEN 16

// For the server queue message

#define MSG_TYPE_LEN 16


// Given configuration

#define MIN_COURSES 10
#define MAX_COURSES 15
#define MIN_TEACHERS 5
#define MAX_TEACHERS 10

// Defining Strutures

typedef struct{

    char client_q[CLIENT_Q_NAME_LEN];
    char msg_val[MSG_VAL_LEN];

} client_msg_t;

typedef struct{

    char msg_type[MSG_TYPE_LEN];
    char msg_val[MSG_VAL_LEN];

} server_msg_t;

static client_msg_t client_msg;

#define SERVER_QUEUE_NAME "/server_msgq"
#define QUEUE_PERMISSIONS 0660
#define MAX_MESSAGES 10
#define MAX_MSG_SIZE sizeof(client_msg_t)
#define MSG_BUFFER_SIZE (MAX_MSG_SIZE * MAX_MESSAGES)

int num_co = 0;     // Number of courses 
int num_tc = 0;     // Number of teachers 

int max_c, min_c, max_t, min_t;


struct Course{ // Courses

    char *name;
    char *teacher;

};

struct Course courses[15];

char *teachers[10];  // Teachers

int num_un_co = 0;  // Number of couses with teachers unassigned

sem_t bin_sem1;

// THread function which iterated until interrupted and keeps printing the report for every 10 s


void *thread_function() 
{
    while (1)
    {
        sem_wait(&bin_sem1);
        printf("----------------------------------------------------------------------\n");
        printf("#### Report #### \n");
        printf("----------------------------------------------------------------------\n");
        printf(" *** Courses ***  \n");
        
        for (int i = 0; i < max_c; i++){
            
            if (strcmp(courses[i].name, "NULL") != 0){
                printf("----------------------------------\n");
                printf("Course = %s\n", courses[i].name);
                printf("Teacher = %s\n", courses[i].teacher);
                printf("----------------------------------\n");
            }
        }
        
        printf("----------------------------------------------------------------------\n");
        printf(" *** Teachers *** \n");
        
        for (int i = 0; i < max_t; i++){
            if (strcmp(teachers[i], "NULL") != 0){
                printf("----------------------------------\n");
                printf(" Teacher : %s\n", teachers[i]);
                printf("----------------------------------\n");
            }
        }
        
        printf("----------------------------------------------------------------------\n");
        
        sem_post(&bin_sem1);
        
        sleep(10);
    }
}

// Function to store the results after Exiting.

void Store_Results(int sig_num)
{
    printf(" saving results \n");
    
    printf("Thank you ... \n");

    FILE *p = fopen("results.txt", "w");
    
    if (p == NULL)
    {
        printf("Error \n");
        exit(1);
    }

    fprintf(p, " *** Courses **** \n");
    
    for (int i = 0; i < max_c; i++) 
        if (strcmp(courses[i].name, "NULL") != 0) fprintf(p, " Course: %s Teacher : %s\n", courses[i].name, courses[i].teacher);
    
    fprintf(p, " *** Teachers *** : \n");
    
    for (int i = 0; i < max_t; i++) 
        if (strcmp(teachers[i], "NULL") != 0) fprintf(p, " Teacher Name : %s\n", teachers[i]);
   
    fclose(p);
    
    exit(0);
}


// Function to execute the given instructions

int Execute_Command(char *cmd) 

{
    char *cmds[100];
    char *token = strtok(cmd, " ");
    
    int n = 0; // to keep track value of server

    while (token != NULL) 
    {
        cmds[n] = token;
        token = strtok(NULL, " ");
        n++;
    }

    sem_wait(&bin_sem1);
    
    int val = 0;

    // ADD A COURSE
    
    if (strcmp(cmds[0], "ADD_COURSE") == 0)
    
    {
        
        for (int i = 1; i < n; i++)
            
            if (num_co < max_c) {
                
                char *name = cmds[i];

                if (name[strlen(name) - 1] == ',') name[strlen(name) - 1] = '\0';
                
                int n = 0; // value

                for (int k = 0; k < max_c; k++){
                    
                    if (strcmp(courses[k].name, name) == 0){ // If course already exists
                        
                        n = 1;
                        val = 1;
                        
                        break;
                    }
                }

                // if not exists add to couses and increment number of courses and un assigned
                
                if (n == 0){ 
                    
                    struct Course course;
                    
                    course.name = name;
                    
                    course.teacher = "NULL";
                    
                    int i = 0;
                    
                    for (i = 0; i < max_c; i++)
                    
                    {
                        if (strcmp(courses[i].name, "NULL") == 0){
                            
                            courses[i] = course;
                            
                            num_co++;
                            
                            num_un_co++;
                            
                            break;
                        }
                    }
                }
            } 
            
            else {
                
                val = 3; // cannot add the course  if number of couses above the Rangelimit 
            
            }
    }

    // DELETE A COURSE
    
    else if (strcmp( cmds[0], "DEL_COURSE") == 0){
        
        for (int i = 1; i < n; i++)

            if (num_co > min_c) {

                char *course = cmds[i];
                
                if (course[strlen(course) - 1] == ',') course[strlen(course) - 1] = '\0';
                
                int  h = 0;

                for (int k = 0; k < max_c; k++){
                
                    if (strcmp(courses[k].name, course) == 0){
                
                        h = 1;
                
                        courses[k].name = "NULL";
                
                        if (strcmp(courses[k].teacher, "NULL") == 0) num_un_co--;
                
                        courses[k].teacher = "NULL";
                
                        num_co--;
                
                        break;
                    }
                }
                
                if (h == 0) {
                    
                    val = 2; // If course doesn't exist

                }
            }
            else {

                val = 3; // Number of courses going below range limit

            }
    
    } 
    
    // ADD A TEACHER
    
    else if (strcmp(cmds[0], "ADD_TEACHER") == 0) {
        
        for (int i = 1; i < n; i++){
            
            if (num_tc < max_c){
               
                char *teacher = cmds[i];

                if (teacher[strlen(teacher) - 1] == ',') teacher[strlen(teacher) - 1] = '\0';
               
                int h = 0;

                for (int k = 0; k < max_t; k++){
                    
                    if (strcmp(teachers[k], teacher) == 0){
                    
                        val = 4;
                        
                        h = 1;
                    
                    } // Teacher already exists
                }
                
                if (h == 0){
                    
                    int k = 0;
                    
                    for (k = 0; k < max_c; k++){
                        
                        if (strcmp(teachers[k], "NULL") == 0){
                            
                            teachers[k] = teacher;
                            
                            num_tc++;
                            
                            break;
                        }
                    }
                }    
            }

            else {
                
                val = 6; // Range linit above or below
            }
        }
    }

    // DELETE A TEACHER

    else if (strcmp(cmds[0], "DEL_TEACHER") == 0){
        
        if (num_tc > 0){
            
            for (int i = 1; i < n; i++){
                
                char *teacher = cmds[i];
                
                if (teacher[strlen(teacher) - 1] == ','){
                    
                    teacher[strlen(teacher) - 1] = '\0';
               
                }
                
                int h = 0;

                for (int k = 0; k < max_t; k++){
                
                    if (strcmp(teachers[k], teacher) == 0){
                        
                        h = 1;
                        
                        teachers[k] = "NULL";
                        
                        num_tc--;
                        
                        break;
                    }
                
                }
                
                for (int k = 0; k < max_c; k++){
                    
                    if (strcmp(courses[k].teacher, teacher) == 0){
                        
                        courses[k].teacher = "NULL";
                        
                        num_un_co++;
                    }
                }
                
                if (h == 0) 
                {

                val = 5; // Teacher does not exist !
                
                }
            }
        }
        
        else{
            
             val = 6; //Number of teachers going below/above Range limit   !
        }
    }
    
    else {
        
    val = -1;

    }

    // ASSIGN A TEACHER
    
    if (num_un_co > 0){

        if (num_tc >= 1)
    {
        for (int i = 0; i < max_c; i++)
        {
            if (strcmp(courses[i].name, "NULL") != 0 && strcmp(courses[i].teacher, "NULL") == 0)
            
            {

                // Random Allocation

                int rn = rand(), i = (rn % (num_tc)), c = 0; 
                
                for (int j = 0; j < max_t; j++)
                
                {
                    if (strcmp(teachers[j], "NULL") != 0)
                    
                    {
                        if (i == c) 
                        
                        {
                            courses[i].teacher = (char *)malloc(strlen(teachers[j]));
                            
                            strcpy(courses[i].teacher, teachers[j]);
                            
                            break;
                        } 
                        
                        else i++;
                    }
                }
            }
        }
    }


    }
    
    sem_post(&bin_sem1);
    
    return val;
}


int main(int argc, char **argv)

{
    
    
    mqd_t qd_srv, qd_client; // Server and Client Msg queue descriptors
    
    int res_sem = sem_init(&bin_sem1, 0, 1);

    printf("Server MsgQ: Welcome !!! \n");

    printf(" EDU SERVER CLIENT APPLICATION  \n");


    printf("Enter the required configuration : \n ");


    printf("Min Courses: ");
    scanf("%d", &min_c);

    printf("Max Courses: ");
    scanf("%d", &max_c);

    printf("Min Teachers: ");
    scanf("%d", &min_t);

    printf("Max Teachers: ");
    scanf("%d", &max_t);


    if ((min_c < MIN_COURSES) || (min_c > MAX_COURSES)) { 
    
    min_c = 10;

    }
    if ((max_c < MIN_COURSES) || (max_c > MAX_COURSES)) {
    
    max_c = 15;

    }
    if ((max_t < MIN_TEACHERS) || (max_t > MAX_COURSES)) {
        
    min_t = 5;
    
    }
    if ((min_t < MIN_TEACHERS) || (min_t > MAX_COURSES)) {
    
    max_t = 10;

    }

    for (int i = 0; i < max_c; i++)
    {
     
        courses[i].name = "NULL";
        courses[i].teacher = "NULL";
    
    }
    for (int i = 0; i < max_t; i++)
    {
    
        teachers[i] = "NULL";
    
    }

    signal(SIGINT, Store_Results);

    struct mq_attr attr;

    attr.mq_flags = 0;
    attr.mq_maxmsg = MAX_MESSAGES;
    attr.mq_msgsize = MAX_MSG_SIZE;
    attr.mq_curmsgs = 0;

    if ((qd_srv = mq_open(SERVER_QUEUE_NAME, O_RDONLY | O_CREAT, QUEUE_PERMISSIONS,&attr)) == -1)
    {
        perror("Server MsgQ: mq_open (qd_srv)");
        exit(1);
    }

    pthread_t thread_id;
    
    int r = pthread_create(&thread_id, NULL, thread_function, "Created a Report thread");

    if (r != 0)
    
    {
        perror("Error: Thread creation failed\n");  // If thread creation failed
        exit(EXIT_FAILURE);
    }


    client_msg_t in_msg;
    
    int val,i;
    
    while (1)
    {

        if (mq_receive(qd_srv, (char *)&in_msg, MAX_MSG_SIZE, NULL) == -1)
        
        {
            perror("Server msgq: mq_receive");
            exit(1);
        }
        
        char *cmd = (char *)malloc(strlen(in_msg.msg_val));
        
        strcpy(cmd, in_msg.msg_val);

         // Executing the command 

        int val = Execute_Command(cmd);

        printf("%s: Server MsgQ: message received.\n", in_msg.msg_val);

        i++;

        server_msg_t out_msg;
        
        strcpy(out_msg.msg_type, "Server msg");
        sprintf(out_msg.msg_val, "%d", val);

        // Open the client queue using the client queue name received
        
        if ((qd_client = mq_open(in_msg.client_q, O_WRONLY)) == 1)
        {
            perror("Server MsgQ: Not able to open the client queue");
            continue;
        }
        
        if (mq_send(qd_client, (char *)&out_msg, sizeof(out_msg), 0) == -1)
        {
            perror("Server MsgQ: Not able to send message to the client queue");
            continue;
        }

    } 
    
    // End of while(1)

} 

// End of main()