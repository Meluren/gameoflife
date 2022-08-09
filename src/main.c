#include <SDL2/SDL.h>
#include <stdbool.h>
#include <time.h>


// Global constants.
const int SCR_WIDTH      = 640;
const int SCR_HEIGHT     = 480;
const int CELL_SIZE      = 8;
const int FPS            = 240;
const int SIMULATION_FPS = 20;

// Color to draw cells with.
const Uint8 RED = 0xFF;
const Uint8 GRN = 0xFF;
const Uint8 BLU = 0xFF;



// Derived constants. These shouldn't be modified directly.
const int N_CELLS_ROW = SCR_WIDTH  / CELL_SIZE;
const int N_CELLS_COL = SCR_HEIGHT / CELL_SIZE;
const Uint32 COLOR = (((RED << 8) + GRN) << 8) + BLU;
const int FRAMES_PER_ITERATION = FPS / SIMULATION_FPS;


void drawScreen(SDL_Window* window, bool cells[N_CELLS_ROW][N_CELLS_COL]);
void updateCells(bool cells[N_CELLS_ROW][N_CELLS_COL]);
int readInput(bool cells[N_CELLS_ROW][N_CELLS_COL], char* fileName);
void populateRandomCells(bool cells[N_CELLS_ROW][N_CELLS_COL]);


int main(int argc, char** argv) {
    // Check that we were supplied with a filename.
    if (argc > 2) {
        fprintf(stderr, "Error: Too many arguments.");
        return EXIT_FAILURE;
    }

    // Do a quick sanity check on the size of cells as compared to the
    // dimensions of the screen.
    if ((SCR_WIDTH % CELL_SIZE) || (SCR_HEIGHT % CELL_SIZE)) {
        fprintf(stderr, "Error: Screen dimension must be a multiple of cell dimension.\n");
        return EXIT_FAILURE;
    }

    // Check that our simulation speed isn't higher than our FPS.
    if (FRAMES_PER_ITERATION == 0) {
        fprintf(stderr, "Error: Simulation FPS cannot be higher than FPS.\n");
        return EXIT_FAILURE;
    }



    // Initialize SDL and create the main window and event handler.
    SDL_Init(SDL_INIT_EVERYTHING);
    SDL_Window* window = SDL_CreateWindow("Conway's Game of Life",
                                SDL_WINDOWPOS_CENTERED, SDL_WINDOWPOS_CENTERED,
                                              SCR_WIDTH, SCR_HEIGHT, 0);
    SDL_Event e;


    // Create a 2D boolean array to hold our living and dead cells.
    bool cells[N_CELLS_ROW][N_CELLS_COL];
    if (argc == 1)
        populateRandomCells(cells);
    if (argc == 2) 
        if (readInput(cells, argv[1]) != EXIT_SUCCESS) return EXIT_FAILURE;


    /* MAIN LOOP */
    bool quit = false;
    for (int frame = 0; !quit; frame = (frame+1) % FRAMES_PER_ITERATION) {
        Uint32 frameStartTime = SDL_GetTicks();

        // Check if user wants to quit.
        while (SDL_PollEvent(&e)) {
            if ((e.type == SDL_QUIT) || 
               ((e.type == SDL_KEYDOWN) && (e.key.keysym.sym == SDLK_ESCAPE)))
                    quit = true;
        }

        // Draw the screen and update the cell array.
        drawScreen(window, cells);
        if (frame == 0)
            updateCells(cells);

        // Wait until we hit the FPS target.
        Uint32 frameTime = SDL_GetTicks() - frameStartTime;
        SDL_Delay((frameTime > 1000 / FPS) ? 0 : 1000 / FPS - frameTime);
    }

    
    return EXIT_SUCCESS;
}


/* Function: updateCells
 * ---------------------
 *  Performs one iteration of the Game of Life.
 *
 *  cells: Boolean 2D array of living and dead cells to update to the next
 *         generation.
 */
void updateCells(bool cells[N_CELLS_ROW][N_CELLS_COL]) {
    // Create temporary array to hold the new cells.
    bool newCells[N_CELLS_ROW][N_CELLS_COL];

    // Perform one iteration of Conway's Game of Life by checking each cell and
    // its neighbors, then applying the rules of the Game.
    for (int y = 0; y < N_CELLS_COL; y++) {
        for (int x = 0; x < N_CELLS_ROW; x++) {
            int neighbors = 0;
            for (int dx = -1; dx <= 1; dx++) {
                if ((x + dx < 0) || (x + dx >= N_CELLS_ROW)) continue;
                for (int dy = -1; dy <= 1; dy++) {
                    if ((y + dy < 0) || (y + dy >= N_CELLS_COL)) continue;
                    neighbors += cells[x+dx][y+dy];
                }
            }

            // Living cells die from loneliness or overpopulation, otherwise
            // they stay alive.
            if (cells[x][y]) {
                if      (neighbors < 3) newCells[x][y] = false;
                else if (neighbors > 4) newCells[x][y] = false;
                else                    newCells[x][y] = true;
            }
            // Dead cells stay dead unless they have exactly 3 live neighbors.
            else {
                if   (neighbors == 3) newCells[x][y] = true;
                else                  newCells[x][y] = false;
            }
        }
    }

    // Copy the temporary cell array to the cell array.
    for (int y = 0; y < N_CELLS_COL; y++)
        for (int x = 0; x < N_CELLS_ROW; x++)
            cells[x][y] = newCells[x][y];
}


/* Function: drawScreen
 * --------------------
 *  Performs the screen drawing process, including the clearing of the screen.
 *
 *  window: Pointer the the SDL_Window to draw on.
 *  cells:  Boolean 2D array of living and dead cells to draw.
 */
void drawScreen(SDL_Window* window, bool cells[N_CELLS_ROW][N_CELLS_COL]) {
    SDL_Surface* surface = SDL_GetWindowSurface(window);

    // Clear the screen.
    SDL_FillRect(surface, NULL, 0);

    // Draw each living cell.
    for (int y = 0; y < N_CELLS_COL; y++) {
        for (int x = 0; x < N_CELLS_ROW; x++) {
            if (cells[x][y])
                SDL_FillRect(surface, &(SDL_Rect) {x*CELL_SIZE, y*CELL_SIZE,
                                                   CELL_SIZE, CELL_SIZE}, COLOR);
        }
    }

    SDL_UpdateWindowSurface(window);
}


/* Function: readInput
 * -------------------
 *  Read input from stdin into a boolean array. Text files work well. The '#'
 *  character represents living cells while the '.' character represents 
 *  dead cells.
 *
 *  cells:     Boolean 2D array populated by the input from stdin.
 *  inputFile: Filename containing initial position of cells.
 *
 *  returns: EXIT_SUCCESS on success and EXIT_FAILURE on failure.
 */
int readInput(bool cells[N_CELLS_ROW][N_CELLS_COL], char* fileName) {
    // Open the input file for reading.
    FILE* fs = fopen(fileName, "r");
    if (fs == NULL) {
        fprintf(stderr, "Error: Could not open file: %s.\n", fileName);
        return EXIT_FAILURE;
    }

    // Read input file assigning '#' for live cells and '.' for dead cells.
    char c;
    int x, y;
    for (x = 0, y = 0; (c = fgetc(fs)) != EOF;) {
        if (c == '\n') {
            x = 0;
            y++;
        } else if (c == '#') {
            cells[x][y] = true;
            x++;
        } else if (c == '.') {
            cells[x][y] = false;
            x++;
        } else {
            fprintf(stderr, "Error: Unknown character '%c' encountered in input file!\n", c);
            fclose(fs);
            return EXIT_FAILURE;
        }
    }
    
    fclose(fs);
    return EXIT_SUCCESS;
}


/* Function: populateRandomCells
 * -----------------------------
 *  If no file is supplied as input, this function fills the cells array
 *  with alive and dead cells at random.
 *
 *  cells: Boolean 2D array to be populated by alive and dead cells.
 */
void populateRandomCells(bool cells[N_CELLS_ROW][N_CELLS_COL]) {
    srand(time(NULL));
    for (int x = 0; x < N_CELLS_ROW; x++)
        for (int y = 0; y < N_CELLS_COL; y++)
            cells[x][y] = rand() % 2;
}
