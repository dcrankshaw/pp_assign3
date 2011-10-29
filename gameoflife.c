#include <stdio.h>
#include <stdlib.h>
#include <assert.h>
#include "mpi.h"

/**
 * Set the new values for a row of cells
 */

const int COLUMNS = 16;
const int ROWS = 16;
const int ITERATIONS = 64;


void update_row(int *row, int *lower, int *upper, int *new_row)
{
  int i;
  for(i = 0; i < COLUMNS; i++) {
      int val = row[i];
      int neighbors = 0;
      int y;
      int x;
      for(x = -1; x <= 1; x++) {
	  for(y = -1; y <= 1; y++) {
	    int *current_row = row; //saves an else statement to initiliaze it here

	    if(y == -1) {
	      current_row = lower;
	    }
	    else if(y == 1) {
	      current_row = upper;
	    }
	    
	    if(!((x == 0) && (y == 0))) { //don't count the cell we are counting neighbors for
	      int x_val = i + x;
	      if(i + x >= COLUMNS) { //wrap around on the left
		x_val = 0;
		}
	      else if(i + x < 0) { //wrap around on the right
		  x_val = COLUMNS - 1;
		}
	      neighbors += current_row[x_val];
	      }
	  }
      }
      if((row[i] == 0) && (neighbors == 3)) {
	new_row[i] = 1;
      }
      else if((row[i] == 1) && ((neighbors == 2) || (neighbors == 3))) {
	new_row[i] = 1;
      }
      else {
	new_row[i] = 0;
      }
  }

}


int main(int argc, char **argv)
{

  int num_procs;
  int ID;
  MPI_Status stat;

  const int global_grid[ 256 ] = { 0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                   0,0,1,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                   1,1,1,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,
                                   0,0,0,0,0,0,0,0,0,0,0,0,0,0,0,0 };




  

  if( MPI_Init(&argc, &argv) != MPI_SUCESS ) {
    printf("MPI_Init error\n");
  }

  MPI_Comm_size(MPI_COMM_WORLD, &num_procs );
  MPI_Comm_rank(MPI_COMM_WORLD, &ID);

  //foreign_bottom is one row below my_bottom, foreign_top is one row above my_top
  int *my_top, *my_bottom, *new_row, *foreign_top, *foreign_bottom;

  int prev, next;
  int num_rows = ROWS / num_procs;
  int **my_rows = (int **) malloc(sizeof(int *) * num_rows);
  
  int i, j;
  int current_row;
  for(i = 0; i < num_rows; i++) {
    current_row = ID + i;
    my_rows[i] = (int *) malloc(sizeof(int) * COLUMNS);
    for(j = 0; j < COLUMNS; j++) {
      my_rows[i][j] = global_grid[current_row*COLUMNS + j];
    }
  }
  my_top = my_rows[0];
  my_bottom = my_rows[num_rows - 1];
  
  next = (ID + 1) % num_procs;
  prev = ID == 0 ? num_procs - 1 : ID - 1;

  int iter;
  for(iter = 0; iter < ITERATIONS; iter++) {

    if ( ID % 2 == 0) {
      MPI_Ssend( my_bottom, COLUMNS, MPI_INT, next, 2, MPI_COMM_WORLD);
      MPI_Ssend( my_top, COLUMNS, MPI_INT, prev, 2, MPI_COMM_WORLD);
      MPI_Recv ( foreign_top, COLUMNS, MPI_INT, prev, 2, MPI_COMM_WORLD, &stat);
      MPI_Recv ( foreign_bottom, COLUMNS, MPI_INT, next, 2, MPI_COMM_WORLD, &stat);
    }
    else {
      MPI_Recv ( foreign_top, COLUMNS, MPI_INT, prev, 2, MPI_COMM_WORLD, &stat);
      MPI_Recv ( foreign_bottom, COLUMNS, MPI_INT, next, 2, MPI_COMM_WORLD, &stat);
      MPI_Ssend( my_bottom, COLUMNS, MPI_INT, next, 2, MPI_COMM_WORLD);
      MPI_Ssend( my_top, COLUMNS, MPI_INT, prev, 2, MPI_COMM_WORLD);
    }
    /* Send state to 0 */
    if(ID != 0) {
      for(i = 0; i < num_rows; i++) {
	MPI_Ssend(my_rows[i], COLUMNS, MPI_INT, 0, i, MPI_COMM_WORLD);
      }
    }
    else {
      for(i = 1; i < num_procs; i++

    }

  }

  






  for(i = 0; i < num_rows; i++) {
    free(my_rows[i]);
  }
  free(my_rows);

}





/** update_row test main
int main(void)
{
  int *upper = (int *)malloc(sizeof(int)*16);
  int *row = (int *)malloc(sizeof(int)*16);
  int *lower = (int *)malloc(sizeof(int)*16);
  int *new_row = (int *)malloc(sizeof(int)*16);

  const int upper_init[16] = {0, 0, 0, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0};
  const int row_init[16] =   {0, 0, 1, 0, 0, 0, 0, 1, 1, 1, 1, 0, 0, 0, 0, 0};
  const int lower_init[16] = {0, 0, 0, 0, 0, 0, 0, 1, 1, 0, 0, 0, 0, 0, 0, 0};

  int i;
  for(i = 0; i < COLUMNS; i++) {
    upper[i] = upper_init[i];
    row[i] = row_init[i];
    lower[i] = lower_init[i];
  }


  update_row(row, lower, upper, new_row);
  for(i = 0; i < COLUMNS; i++) {
    printf("%i, ", new_row[i]);
  }
  printf("\n");


  free(upper);
  free(lower);
  free(row);
  free(new_row);


  return 0;

}
*/
