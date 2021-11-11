#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>
#include <math.h>

/**
 * README
 * 
 * - Both P5 and P2 format supported as input.
 * - Output file will be in it's original format.
 *   For example if your original PGM was P2, output PGM will
 *   be in P2 format (and vice-versa). 
 * 
 * Code structure:
 * Search like "(x)" to jump directly to section.
 * 
 * (0) Program configuration definitions.
 * (1) Data types and structures.
 * (2) Common generic utilities.
 * (3) PGM related functions.
 * (4) Neighbors Operations
 * (5) Image-Processing operation.
 * (6) Main
 */

/**
 * (0) Program configuration definitions.
 */
#define LINESIZE 256
#define NLCHR '\n'
#define NLSTR "\n"
#define MAX_COLOR 255
#define P5 "P5"
#define P2 "P2"
#define COMMENT_IDENTIFIER '#'
#define DEFUALT_OUTPUT_NAME "output.pgm"

/**
 * (1) Type definitions and data structures.
 */
typedef struct PGM PGM;
typedef uint8_t Pixel;
typedef struct Neighbors Neighbors;
typedef int (*Kernel)(Neighbors*);

struct PGM {
  char* type;
  Pixel* pixels;
  int maxValue;
  int width;
  int height;
};

struct Neighbors {
  int* cells;
  int distance;
};

/**
 * (2) Common generic utilities.
 */
bool streq(char* a, char* b) {
  return strcmp(a, b) == 0;
}

char* mallocstr(int size) {
  return (char*) malloc((size + 1) * sizeof(char));
}

char* copystr(char* str) {
  char* copy = mallocstr(strlen(str));
  if (copy) strcpy(copy, str);
  return copy;
}

FILE* openFile(char* filename, char* as) {
  FILE* file = fopen(filename, as);
  if (file == NULL) {
    printf("Error: Could not open file; name=%s, mode=%s\n", filename, as);
    return NULL;
  };
  return file;
}

char* scanLine() {
  char* input = mallocstr(LINESIZE);
  fgets(input, LINESIZE, stdin);

  if (input[strlen(input)-1] != NLCHR) {
    int ch;
    while (((ch = getchar()) != NLCHR) && (ch != EOF));
  }

  if (strcmp(input, NLSTR) == 0) {
    strcpy(input, "");
  } else if (strcmp(input, " \n") == 0) {
    strcpy(input, "");
  } else {
    input[strcspn(input, "\r\n")] = 0;
  }

  return input;
}

/**
 * (3) PGM related functions. 
 */
int skipWhitespace(FILE* file) {
  int chr;
  while ((chr = fgetc(file)) != EOF && isspace(chr));
  if (chr != EOF) fseek(file, -1, SEEK_CUR);
  return chr;
}

void skipCommentsAndWhitespace(FILE* file) {
  int nextChr = skipWhitespace(file);

  // Go one byte back if not comment.
  if (nextChr == COMMENT_IDENTIFIER) {
    // Skip line.
    char line[LINESIZE];
    fgets(line, sizeof(line), file);
      
    // Continue to skipping next comments.
    skipCommentsAndWhitespace(file);
  }
}

PGM* createPGM(char* type, int maxValue, int width, int height) {
  PGM* pgm = (PGM*) malloc(sizeof(PGM));

  if (pgm == NULL) {
    printf("Error: Could not allocate memory for PGM.");
    return NULL;
  }

  pgm->pixels = (Pixel*) calloc(width * height, sizeof(Pixel));

  if (pgm->pixels == NULL) {
    printf("Error: Could not allocate memory for PGM pixels.");
    return NULL;
  }

  pgm->type = copystr(type);
  pgm->maxValue = MAX_COLOR; //maxValue;
  pgm->width = width;
  pgm->height = height;

  return pgm;
}

PGM* readPGM(char* filepath) {
  FILE* file = openFile(filepath, "rb");
  
  if (file == NULL) {
    printf("Error: Cannot find PGM file in given path: %s\n", filepath);
    return NULL;
  }
  
  char pgmType[LINESIZE];
  int maxValue;
  int width;
  int height;

  skipCommentsAndWhitespace(file);
  fscanf(file, "%s", pgmType);

  if (strcmp(pgmType, P5) && strcmp(pgmType, P2)) {
    printf("Error: Incorrect PGM type, it should be either "P5" or "P2". Given type: %s\n", pgmType);
    return NULL;
  }
  
  skipCommentsAndWhitespace(file);
  fscanf(file, "%d %d", &(width), &(height));
  skipCommentsAndWhitespace(file);
  fscanf(file, "%d", &(maxValue));
  fgetc(file); // Skip last whitespace before pixels.

  PGM* pgm = createPGM(pgmType, maxValue, width, height);

  if (pgm == NULL) {
    printf("Error: Cannot read PGM due to memory allocation issues.\n");
    return NULL;
  }

  int pixelCount = pgm->width * pgm->height;

  // Read pixel values into pgm.
  if (!strcmp(pgmType, P5)) {
    // Read P5
    fread(pgm->pixels, sizeof(Pixel), pixelCount, file);
  } else {
    // Read P2
    int i, color;
    for (i = 0; i < pixelCount; i++) {
      skipWhitespace(file);
      fscanf(file, "%d", &(color));
      pgm->pixels[i] = color;

      if (color < 0 || color > pgm->maxValue) {
        printf("Error: Out of bound (0~%d) value %d read as color from PGM.\n", pgm->maxValue, color);
        return NULL;        
      }
    }
  }

  fclose(file);

  return pgm;
}

void writePGMWithType(PGM* pgm, char* filepath, char* type) {
  FILE* file = openFile(filepath, "wb");
  
  // Write header
  fprintf(file, 
    "%s\n%d %d\n%d\n",
    type,
    pgm->width,
    pgm->height,
    pgm->maxValue
  );
  
  int pixelCount = pgm->width * pgm->height;

  // Write pixels.
  if (!strcmp(type, P5)) {
    // Write P5
    fwrite(pgm->pixels, sizeof(Pixel), pixelCount, file);
  } else {
    // Write P2
    int i;
    for (i = 0; i < pixelCount; i++) {
      fprintf(file, "%d%c", pgm->pixels[i], ((i+1) % pgm->width) ? ' ' : '\n');
    }
  }
  
  fclose(file);
}

void writePGM(PGM* pgm, char* filepath) {
  writePGMWithType(pgm, filepath, pgm->type);
}

void freePGM(PGM* pgm) {
  free(pgm->type);
  free(pgm->pixels);
  free(pgm);
}

/**
 * (4) Neighbors Operations
 */
Neighbors* createNeighbors(int distance) {
  Neighbors* neighbors = (Neighbors*) malloc(sizeof(Neighbors));

  if (neighbors == NULL) {
    printf("Error: Could not allocate memory for Neighbors.");
    return NULL;
  }

  neighbors->cells = (int*) calloc(distance * distance, sizeof(int));

  if (neighbors->cells == NULL) {
    printf("Error: Could not allocate memory for Neighbors cells.");
    return NULL;
  }

  neighbors->distance = distance;

  return neighbors;
}

void freeNeighbors(Neighbors* neighbors) {
  free(neighbors->cells);
  free(neighbors);
}

/**
 * (5) Image-Processing Operations
 */
PGM* applyKernel(PGM* input, int ker_size, Kernel kernel) {
  int in_width = input->width;
  int in_height = input->height;
  int ker_margin = ker_size - 1;
  int out_width = in_width - ker_margin;
  int out_height = in_height - ker_margin;

  PGM* output = createPGM(input->type, input->maxValue, out_width, out_height);
  if (output == NULL) return NULL;

  Neighbors* neighbors = createNeighbors(ker_size);
  if (neighbors == NULL) return NULL;

  int r, c, x, y; // multi-dimensional indexes.
  // process inner rows and columns with margin
  for (r = 0; r < out_height; r++) {
    for (c = 0; c < out_width; c++) {
      int i_out = (r * out_width) + c;

      // fill neighbors flat matrix (target matrix of convolution)
      for (y = 0; y < ker_size; y++) {
        for (x = 0; x < ker_size; x++) {
          int i_mat = (y * ker_size) + x;
          int i_in = ((r + y) * in_width) + c + x;

          neighbors->cells[i_mat] = (int) input->pixels[i_in];
        }
      }

      // Pass neighbors flat matrix to kernel function to process, and set result.
      output->pixels[i_out] = kernel(neighbors);
    }  
  }

  freeNeighbors(neighbors);
  return output;
}

int averageFilterKernel(Neighbors* neighbors) {
  float count = neighbors->distance * neighbors->distance;
  float sum = 0;
  int i;

  for (i = 0; i < count; i++) {
    sum += neighbors->cells[i];
  }

  int average = (int) round(sum / count);
  return average;
}

int medianFilterKernel(Neighbors* neighbors) {
  float count = neighbors->distance * neighbors->distance;
  int* cells = neighbors->cells;

  int i, key, j;
  for (i = 1; i < count; i++) {
    key = cells[i];
    j = i - 1;

    while (j >= 0 && cells[j] > key) {
      cells[j + 1] = cells[j];
      j = j - 1;
    }
    cells[j + 1] = key;
  }

  int median = cells[(int) round(count / 2.0)];
  return median;
}

/**
 * (6) Main
 */
int printIncorrectArgsMsg() {
  printf("Error: Incorrect arguments, please check your arguments, type 'help' to see usages!\n");
  return 0;
}

int printHelpMsg() {
  printf("---------------------------------------------------------------------------------------------------------------\n");
  printf("$ %-45s %s\n", "average <input.pgm> [<output.cpgm>]", "- will apply average filter to image (default output = "DEFUALT_OUTPUT_NAME")");
  printf("$ %-45s %s\n", "median <input.pgm> [<output.cpgm>]", "- will apply median filter to image  (default output = "DEFUALT_OUTPUT_NAME")");
  printf("$ %-45s %s\n", "help", "- display this message");
  printf("---------------------------------------------------------------------------------------------------------------\n");
  return 0;
}

int main(int argc, char **argv) {
  // Handle arguments and help command
  if (argc <= 1) {
    return printHelpMsg();
  }

  char* cmd = argv[1];
  char* input = argv[2];
  char* output = argv[3];

  if (streq(cmd, "help")) {
    return printHelpMsg(); 
  }

  if (argc == 2) {
    return printIncorrectArgsMsg();
  }

  if (argc == 3) {
    output = DEFUALT_OUTPUT_NAME;
  }

  // Handle filter commands.
  char* name;
  Kernel kernel;
  int kernelSize;

  if (streq(cmd, "average")) {
    name = "Average";
    kernelSize = 3;
    kernel = averageFilterKernel;
  } else if (streq(cmd, "median")) {
    name = "Median";
    kernelSize = 3;
    kernel = medianFilterKernel;
  } else {
    return printIncorrectArgsMsg();
  }

  // Apply kernel to the PGM.
  PGM* pgm_input = readPGM(input);
  if (pgm_input == NULL) return 0;

  PGM* pgm_output = applyKernel(pgm_input, kernelSize, kernel);
  if (pgm_output != NULL) {
    writePGM(pgm_output, output);
    freePGM(pgm_output);
  };

  freePGM(pgm_input);
  
  printf("-> %s filter successfully applied to %s and result saved to %s\n", name, input, output);
  return 0;
}
