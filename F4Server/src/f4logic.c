#include "f4logic.h"


int check_winner(pid_t** matrix, pid_t player, int row_size, int column_size) {
    // Check horizontally
    for (int row = 0; row < row_size; row++) {
        for (int col = 0; col < column_size - 3; col++) {
            if (matrix[row][col] == player &&
                matrix[row][col + 1] == player &&
                matrix[row][col + 2] == player &&
                matrix[row][col + 3] == player) {
                return player; // Player wins horizontally
            }
        }
    }

    // Check vertically
    for (int row = 0; row < row_size - 3; row++) {
        for (int col = 0; col < column_size; col++) {
            if (matrix[row][col] == player &&
                matrix[row + 1][col] == player &&
                matrix[row + 2][col] == player &&
                matrix[row + 3][col] == player) {
                return player; // Player wins vertically
            }
        }
    }

    // Check diagonally (top-left to bottom-right)
    for (int row = 0; row < row_size - 3; row++) {
        for (int col = 0; col < column_size - 3; col++) {
            if (matrix[row][col] == player &&
                matrix[row + 1][col + 1] == player &&
                matrix[row + 2][col + 2] == player &&
                matrix[row + 3][col + 3] == player) {
                return player; // Player wins diagonally (top-left to bottom-right)
            }
        }
    }

    // Check diagonally (top-right to bottom-left)
    for (int row = 0; row < row_size - 3; row++) {
        for (int col = 3; col < column_size; col++) {
            if (matrix[row][col] == player &&
                matrix[row + 1][col - 1] == player &&
                matrix[row + 2][col - 2] == player &&
                matrix[row + 3][col - 3] == player) {
                return player; // Player wins diagonally (top-right to bottom-left)
            }
        }
    }

    return 0; // Player does not win
}


/*
 * Funzione modifica matrice e aggiunge il player
 * Controlla se e una giocata valida ritrona -1
 * if winner ritorna il pid del winner
 * else ritorna 0
 *
 * TODO save matrix
 */
int f4_play(pid_t** matrix,int column,pid_t player,int row_size,int column_size){
    //Controlla se la colonna esiste
    if(column >= column_size){
        return -1;
    }

    //Controlla che ci sia spazio sulla colonn
    char is_max_col = 0;
    int i;
    for (i = 0; i < column_size; ++i){
        if(matrix[column][i] == 0){
            matrix[column][i] = player;
            is_max_col = 1;
            break;
        }
    }
    if(!is_max_col){
        return -1;
    }
    return check_winner(matrix,player,row_size,column_size);
}


void clean_array(pid_t ** matrix,int row_size,int column_size){
    for (int i = 0; i < row_size; ++i)
        for (int j = 0; j < column_size; ++j){
            matrix[i][j] = 0;
        }
}
ciao