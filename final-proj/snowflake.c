#include <stdio.h>
#include <stdlib.h>
#include <stdbool.h>

#define WIDTH 51
#define HEIGHT 51
#define ALPHA 1
#define BETA 0.8
#define GAMMA 0.01
#define NUM_NEIGHBORS 4

typedef struct {
    float u;   // water used in diffusion
    float v;   // water not used in diffusion
    float s;   // total water
    bool is_receptive;
} Cell;

Cell cells[WIDTH][HEIGHT];
float s_vals[WIDTH][HEIGHT]; // Array to store s values for visualization or debugging

// Get neighbors for a specific coordinate
int get_neighbors(Cell* neighbors[], int x, int y) {
    int count = 0;

    if (x > 0) neighbors[count++] = &cells[x-1][y];
    if (x < WIDTH - 1) neighbors[count++] = &cells[x+1][y];
    if (y > 0) neighbors[count++] = &cells[x][y-1];
    if (y < HEIGHT - 1) neighbors[count++] = &cells[x][y+1];

    return count;
}

void initialize_grid() {
    for (int i = 0; i < WIDTH; i++) {
        for (int j = 0; j < HEIGHT; j++) {
            cells[i][j].s = BETA;
            cells[i][j].is_receptive = false;
            s_vals[i][j] = BETA; // Initialize s_vals
        }
    }
    // Set the center cell
    cells[WIDTH/2][HEIGHT/2].s = 1.0;
    cells[WIDTH/2][HEIGHT/2].is_receptive = true;
    s_vals[WIDTH/2][HEIGHT/2] = 1.0; // Set center in s_vals
}

void update_s_vals() {
    for (int i = 0; i < WIDTH; i++) {
        for (int j = 0; j < HEIGHT; j++) {
            s_vals[i][j] = cells[i][j].s;
        }
    }
}

void print_s_vals() {
    for (int i = 0; i < WIDTH; i++) {
        for (int j = 0; j < HEIGHT; j++) {
            printf("%.2f ", s_vals[i][j]);
        }
        printf("\n");
    }
    printf("\n");
}

void one_iter() {
    Cell* neighbors[NUM_NEIGHBORS];
    int num_neighbors;

    // Determine receptive sites
    for (int i = 0; i < WIDTH; i++) {
        for (int j = 0; j < HEIGHT; j++) {
            if (cells[i][j].is_receptive) {
                cells[i][j].u = 0;
                cells[i][j].v = cells[i][j].s;
                cells[i][j].s = cells[i][j].v + GAMMA;
            } else {
                cells[i][j].u = cells[i][j].s;
                cells[i][j].v = 0;
            }
        }
    }

    // Diffusion process
    for (int i = 0; i < WIDTH; i++) {
        for (int j = 0; j < HEIGHT; j++) {
            num_neighbors = get_neighbors(neighbors, i, j);
            float sum_u = 0;

            for (int k = 0; k < num_neighbors; k++) {
                sum_u += neighbors[k]->u;
            }
            float u_avg = sum_u / num_neighbors;
            cells[i][j].u += ALPHA / 2 * (u_avg - cells[i][j].u);
            cells[i][j].s = cells[i][j].u + cells[i][j].v;

            // Update receptiveness based on the new s
            if (cells[i][j].s >= 1) {
                cells[i][j].is_receptive = true;
            } else {
                bool any_frozen = false;
                for (int k = 0; k < num_neighbors; k++) {
                    if (neighbors[k]->s >= 1) {
                        any_frozen = true;
                        break;
                    }
                }
                cells[i][j].is_receptive = any_frozen;
            }
        }
    }
}

int main() {
    initialize_grid();

    // run the simulation for 100 iterations as an example
    for (int iter = 0; iter < 100; iter++) {
        one_iter();
        update_s_vals();
        printf("Iteration %d:\n", iter + 1);
        print_s_vals();
    }

    return 0;
}
