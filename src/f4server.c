#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>

#include <fcntl.h>
#include <signal.h>

#include <sys/types.h>
#include <sys/stat.h>
#include <sys/sem.h>
#include <sys/shm.h>


#include "semaphore.h"
#include "commands.h"
#include "f4logic.h"

static volatile int status = 1;

void intHandler(int dummy) {
    printf("Stopping... %d\n", dummy);
    status = 0;
}

int init_fifo(char name[]) {
    remove(name);//TODO Think if multiple instance running
    if (mkfifo(name, S_IRUSR | S_IWUSR) != 0) {
        perror("Creation FIFO :");
        exit(1);
    }

    return open(name, O_RDONLY | O_NONBLOCK);
}


int main(int argc, char *argv[]) {


    // controlla il numero di argomenti di argc
    // esecuzione del server con numero minore di parametri (o 0) comporta la stampa di un messaggio di aiuto
    if (argc != 5) {
        printf("Usage: %s <row> <column> sym_1 sym_2\n", argv[0]);
        return 0;
    }

    // trasformiamo argv1 e 2 in interi
    int row = atoi(argv[1]);
    int column = atoi(argv[2]);

    //controllo validitá della matrice
    if (row < 5) {
        printf("The input row must be >= 5\n");
        return 1;
    }
    if (column < 5) {
        printf("The input column must be < 5\n");
        return 1;
    }

    //inizializziamo i simboli usati dai due giocatori che devono essere diversi tra di loro
    char symbols[2];
    if (*argv[3] == *argv[4]) {
        printf("Can't use same symbols\n");
        return 1;
    }
    symbols[0] = *argv[3];
    symbols[1] = *argv[4];

    //Creation FIFO for init connection
    //No need to have sem has if the read or write is below PIPE_BUF then you can have multiple reader and writer
    //https://www.gnu.org/software/libc/manual/html_node/Pipe-Atomicity.html
    //https://stackoverflow.com/questions/17410334/pipe-and-thread-safety
    //Min size for PIPE_BUF is 512 byte

    signal(SIGINT, intHandler); // Read CTRL+C and stop the program


    //Create fifo for connecting
    int fd_first_input = init_fifo(DEFAULT_PATH);

    struct client_info clients[2];
    int queue_size = 0;
    while (1) {
        //Read a client info from the FIFO
        struct client_info buffer;
        ssize_t n = read(fd_first_input, &buffer, sizeof(struct client_info));
        if (n > 0) {
            printf("Client connecting with :\n");
            printf("pid :%d\n", buffer.pid);
            printf("key id : %d\n", buffer.key_id);
            printf("mode : %c\n", buffer.mode);

            if (buffer.mode == '*') {
                //Single logic here
            } else {
                clients[queue_size++] = buffer;
                if (queue_size == 2) {
                    queue_size = 0;
                    //Create fork and pass logic
                    if (fork() == 0) {
                        //TODO store child and terminate them.
                        break;
                    }
                }
            }
        }
        if (status == 0) {
            //Main program finish
            return 1;
        }
    }
    //Child stuff
    key_t key_msg_qq_input = ftok(DEFAULT_PATH, getpid());

    //Send the symbols to the clients
    cmd_send(clients[0], CMD_SET_SYMBOL, &symbols[0]);
    cmd_send(clients[1], CMD_SET_SYMBOL, &symbols[1]);
    //Broadcast symbol
    cmd_broadcast(clients, CMD_SET_MSG_QQ_ID, &key_msg_qq_input);

    //Calculating the size of the shared memory
    struct shared_mem_info shm_mem_inf;
    shm_mem_inf.mem_size = row * column * sizeof(pid_t);

    //Key for the shared memory and the semaphore
    //Todo maybe random generation for key ?
    shm_mem_inf.key = ftok(".", getpid());



    //TODO Creation of sem for shared memory

    //Create shared memory
    if (shmget(shm_mem_inf.key, shm_mem_inf.mem_size, IPC_CREAT | IPC_CREAT | S_IRUSR | S_IWUSR) == -1) {
        //TODO handle error if there is multiple shared_memory (should be impossible)
    }
    pid_t **matrix = shmat(shm_mem_inf.key, NULL, 0);
    //Broadcast info of shared memory to clients
    cmd_broadcast(clients, CMD_SET_SH_MEM, &shm_mem_inf);

    //Tell the client to update their internal shared memory
    cmd_broadcast(clients, CMD_UPDATE, NULL);




    //Set the turn to the first player
    int turn_num = 0;
    struct client_info player = cmd_turn(clients, turn_num);


    struct client_action action;
    int game = 1;
    while (game) {
        //Read incoming msg queue
        if (msgrcv(key_msg_qq_input, &action, CMD_ACTION, CMD_ACTION / 100, MSG_NOERROR) > 0) {
            if (action.pid == player.pid) {
                pid_t result = f4_play(matrix, column, action.pid, row, column);
                if (result == -1) {
                    cmd_send(player, CMD_INPUT_ERROR, NULL);
                } else if (result == player.pid) {
                    cmd_broadcast(clients, CMD_WINNER, NULL);
                    game = 0;
                }
                turn_num = !turn_num;
                player = cmd_turn(clients, turn_num);

            }
        }
    }

    //pipe id da file preciso per cominciare la connessione al server protetto da semaphore


    /*
     * S <- C (pid,mode,pipe)
     *
     *
     *
     * if single{
     *  single process fork
     * }else{
     * fork(pipe1,pipe2)
     * wait_antoher_client
     * S <- C (pid,mode)
     * linkpipe1 S -> C1
     * linkpipe2 S -> C2
     *
     *  S -> broadcast_all()
     *
     * pipe1("symbol[0]")
     * pipe2("symbol[1]")
     * create pipe_rcv (forse message queue)
     * create sem
     * broadcast_pipe(pipe_rcv.id,sem.id)
     *
     * shared_memory(matrix)
     * broadcast_pipe(set_sem_id,sem.id);
     * broadcast_pipe(set_shared_memory.id,sem.id)
     * broadcast_pipe(shared_memory.id,sem.id)
     *
     * pipe1(your_turn)
     *
     * // S <- C(pid,column)
     *
     * // S <- C(pid,commando)
     *
     * while(wait(input)){
     * input = read_input();
     *
     * //Logic commands(pid, abbandono(CTRL-C)){
     * winner(other);
     * break;
     * }
     *
     * check_right(sender)
     *
     *
     *
     * //Logic sem
     * get_shared_mem(matrix)
     * int result = runf4logic(matrix,input,pid,size[matrix]);
     * save_shared_meme(matrix)
     * //Logic sem
     * broadcast_pipe("update")
     * // C(read_shared_mem)
     *
     * if(result == error)
     *  pipe<c>("error")
     * }
     * if(result == winner){
     * broadcast_pipe("winner","symbol")
     * break();
     * }
     * player()
     * } <---
     * destroy sem,shared,pipe
     */




    // simboli vanno comunicati ai due giocatori

    //Logica per la connesione

    //Creare area memoria per matrice
}