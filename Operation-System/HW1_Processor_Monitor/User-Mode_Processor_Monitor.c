#include <stdio.h>
#include <stdlib.h>
///#include <sys/wait.h>
#include <unistd.h>
///#include <sys/msg.h>
#include <linux/ipc.h>
#include <linux/msg.h>

int totalTask = 0;
int pids[100];
int results[100];
key_t  msgKey;
int    queueId;


struct msgData{
    int taskId;
    int parentId;
    int childPid;
    int status;
};

int send_message( int qid, struct msgData *msgBuf ){
        int     result, length;

        /// The length is essentially the size of the structure minus sizeof(mtype)
        ///length = sizeof(struct msgData) - sizeof(long);
        length = sizeof(struct msgData);

        if((result = msgsnd( qid, msgBuf, length, IPC_NOWAIT )) == -1){
                return(-1);
        }

        return(result);
}

int read_message( int qid, long type, struct msgData *msgBuf ){
        int     result, length;

        ///The length is essentially the size of the structure minus sizeof(mtype) */
        ///length = sizeof(struct msgData) - sizeof(long);
        length = sizeof(struct msgData);


        if((result = msgrcv( qid, msgBuf, length, type, IPC_NOWAIT )) == -1){
                return(-1);
        }

        return(result);
}


int open_queue( key_t keyval ){
        int     queue_id;
        if((queue_id = msgget( keyval, IPC_CREAT | 0660 )) == -1){
                return(-1);
        }
        return(queue_id);
}





int main(int argc, char *argv[]){
    printf("#########Start###########\n");
    totalTask = argc;

    //int pids[totalTask];
    //int results[totalTask];

    initArray(pids,totalTask);
    initArray(results,totalTask);

    ///Generate our IPC key value
    ///msgKey = ftok(".", 'm');

    msgKey = IPC_PRIVATE;
    if(( queueId = open_queue( msgKey)) == -1) {
            perror("open_queue.\n");
            exit(1);
    }

    callChild(0, argc,argv);
    printf("=================[Result]=================\n");
    printf("[  PID From parent ]= ");
    showInfo(pids,totalTask);
    printf("[Status From parent]= ");
    showInfo(results,totalTask);
    printf("#########End#############\n");
    return 0;
}

void initArray(int array[],int count){
     int i=0;
     for (i=0;i<count;i++){
            array[i]=-999;
     }
}

void showInfo(int array[],int count){
    int i=0;
    for (i=0;i<count;i++){
        if (i ==(count -1)){
            printf("%d \n",array[i]);
        }else{
            printf("%d-->",array[i]);
        }
    }
}



void callChild(int front,int back, char *argv[] ){


   front++;
   back--;
   printf("front=%d, back=%d\n",front,back);

   if (back == 0 ) return;
   int status;
   int msgId;


   pid_t childPid = fork();


if (childPid >= 0){

        if (childPid ==0 ){
          printf("front=%d, back=%d, <child> pid=%d, parent_pid=%d, childPid=%d\n",front,back,getpid(),getppid(),childPid);
          callChild(front,back,argv )   ;
          int exeInfo = execlp(argv[front],argv[front],NULL);

        }else{
           wait(&status);
           printf("<waiting parent>front=%d,back=%d, pid=%d, childPid=%d\n",front,back,getpid(),childPid);
           printf("********<waiting parent>front=%d,back=%d, pid=%d, childPid=%d, WIFEXITED=%d,,WEXITSTATUS=%d,WIFSTOPPED=%d,WSTOPSIG=%d,WIFSIGNALED=%d,WTERMSIG=%d \n",front,back,getpid(),childPid, WIFEXITED(status), WEXITSTATUS(status),WIFSTOPPED(status),WSTOPSIG(status) ,WIFSIGNALED(status), WTERMSIG(status));

           int signal = -1;

           if (WIFEXITED(status)){
                signal = WEXITSTATUS(status);
                printf("[Message] WEXITSTATUS(status)=%d, Child process exited normally.\n",signal);

          }else if (WIFSIGNALED(status)){
                signal = WTERMSIG(status);
                printf("[Message] WTERMSIG(status)=%d, Child process exited abnormally. \n",signal);
          }else if ( WIFSTOPPED(status) ){
                signal = WSTOPSIG(status);
                printf("[Message] WSTOPSIG(status)=%d, Child process stop with some reason.\n",signal);
          }else{
              signal = WEXITSTATUS(status);
              printf("[Message] WEXITSTATUS(status)=%d, Child process with unknown status.\n",signal);
              if ( (!WIFEXITED(status)) && (!WIFSIGNALED(status))  &&  (!WIFSTOPPED(status)) && WIFCONTINUED(status) ){
                     printf("[Message] Child process continue from stop status.\n");
              }
          }

          showSignalInfo(signal);



           if (front == 1){
              pids[0] =getpid();
              results[0] = 0;
              pids[1] = childPid;
              results[1] =signal ;

              struct msgData msg;
              memset(&msg,'\0',sizeof(struct msgData));

              int recvResult = -1;
              struct msgData recvMsg;

              int i=1;
              while ( (recvResult = read_message(queueId,0,&recvMsg)) >=0 ){

                  printf("no=%d,recvResult=%d, recvMsg.taskId=%d, recvMsg.parentId=%d, recvMsg.childPid=%d, recvMsg.status=%d \n",i,recvResult,recvMsg.taskId,recvMsg.parentId,recvMsg.childPid,recvMsg.status);

                  pids[recvMsg.taskId]=recvMsg.childPid;
                  results[recvMsg.taskId]=recvMsg.status;
              }


           }else{
                    struct msgData msg;
                    msg.taskId = front;
                    msg.parentId = getpid();
                    msg.childPid=childPid;
                    msg.status =  signal;

                    int sendResult = send_message( queueId, &msg );

                    if (sendResult == -1){
                        printf("<send msg error!!>front=%d,back=%d, pid=%d, childPid=%d\n",front,back,getpid(),childPid);
                    }else{
                        printf("<Send Msg>front=%d,back=%d, pid=%d, childPid=%d\n",front,back,getpid(),childPid);
                    }
           }
            printf("-------------------------------------------------------------\n");
            //showInfo(pids,totalTask);
            //showInfo(results,totalTask);

        }

   }else{
         printf("Error in for child.\n");
    }


}



void showSignalInfo(int signal){
     printf("[Signal=%d] ",signal);
     switch( signal ){
            case 1: printf("Hangup detected on controlling terminal or death of controlling process.\n"); break;
            case 2: printf("Interrupt from keyboard.\n"); break;
            case 3: printf("Quit from keyboard.\n"); break;
            case 4: printf("Illegal Instruction.\n"); break;
            case 6: printf("Abort signal from abort.\n"); break;
            case 8: printf("Floating point exception. \n");break;
            case 9: printf("Kill signal. \n");break;
            case 11: printf("Invalid memory reference or Segmental Fault.\n");break;
            case 13: printf("Broken pipe. \n");break;
            case 14: printf("Timer signal from alarm. \n");break;
            case 15: printf("Termination signal. \n");break;                       
            default:
                printf("Other Signal Code=%d, Please refer to 'man 7 singal' . \n", signal);break;
        }
}




 ///segmentation fault: WIFEXITED=0,,WEXITSTATUS=0,WIFSTOPPED=0,WSTOPSIG=0,WIFSIGNALED=1,WTERMSIG=11
/// abort : WIFEXITED=0,,WEXITSTATUS=0,WIFSTOPPED=0,WSTOPSIG=0,WIFSIGNALED=1,WTERMSIG=6
 ///  alarm :  WIFEXITED=0,,WEXITSTATUS=0,WIFSTOPPED=0,WSTOPSIG=0,WIFSIGNALED=1,WTERMSIG=14
 /// Normal : WIFEXITED=1,,WEXITSTATUS=43,WIFSTOPPED=0,WSTOPSIG=43,WIFSIGNALED=0,WTERMSIG=0

/// catch stop & continue signal
/// WIFEXITED=0,,WEXITSTATUS=19,WIFSTOPPED=1,WSTOPSIG=19,WIFSIGNALED=0,WTERMSIG=127
/// SIGNALED : WIFEXITED=0, ,WEXITSTATUS=0,WIFSTOPPED=0,WSTOPSIG=0,WIFSIGNALED=1,WTERMSIG=2
/// STOPPED :  WIFEXITED=0,,WEXITSTATUS=19,WIFSTOPPED=1,WSTOPSIG=19,WIFSIGNALED=0,WTERMSIG=127
///  CONTINUED : WIFEXITED=0,,WEXITSTATUS=255,WIFSTOPPED=0,WSTOPSIG=255,WIFSIGNALED=0,WTERMSIG=127
