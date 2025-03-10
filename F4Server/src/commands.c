#include "commands.h"




//client 0 cmd : CMD_SET_SYMBOL : char
void cmd_send(struct client_info client, long cmd, void* msg){
    struct msg_buffer buffer;
    buffer.mtype = cmd;
    memcpy(buffer.msg,msg, get_msg_size(cmd));
    msgsnd(client.message_qq, &buffer, get_msg_size(cmd), 0);
}


void cmd_broadcast(struct client_info clients[], long cmd, void* msg){
    for (int i = 0; i < 2; ++i) {
        cmd_send(clients[i], cmd, msg);
    }
}

struct client_info cmd_turn(struct client_info clients[],int turn){
    cmd_send(clients[turn],CMD_TURN,NULL);
    return clients[turn];
}

ssize_t get_msg_size(long code){
    if(code == 0){
        return sizeof(long);
    }
    return code/100 - sizeof(long);
}