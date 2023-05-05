#include <unistd.h>

/**
 * Play the action and check if there is a winner
 * @param matrix matrix to play on
 * @param column action
 * @param player pid_t of the player
 * @param row_size row size of the matrix
 * @param column_size column size of the matrix
 * @return -1 if there is a invalid plays,0 normal play,pid_t the pid of the winner
 */
int f4_play(int** matrix,int column,pid_t player,int row_size,int column_size);