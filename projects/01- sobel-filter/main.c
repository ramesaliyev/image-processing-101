#include <stdio.h>
#include <stdint.h>
#include <stdlib.h>
#include <stdbool.h>
#include <string.h>
#include <ctype.h>

/**
 * README
 * 
 * - Both P5 and P2 format supported as input.
 * - Output file will be in it's original format.
 *   For example if your original PGM was P2, output PGM will
 *   be in P2 format (and vice-versa). 
 * 
 * Code structure:
 * Search like (x) to jump directly to section.
 * 
 * (0) Program configuration definitions.
 * (1) Data types and structures.
 * (2) Common generic utilities.
 * (3) PGM related functions.
 * (4) Kernel-Image Operations
 * (5) Filter Operations
 * (6) Menu operations
 * (7) Main
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
 * (1) Data types and structures.
 */
typedef struct PGM PGM;
typedef struct Kernel Kernel;
typedef uint8_t Pixel;
typedef uint8_t Cell;

struct PGM {
  char* type;
  Pixel* pixels;
  int maxValue;
  int width;
  int height;
};

struct Kernel {
  Cell* cells;
  int width;
  int height;
};

/**
 * (2) Common generic utilities.
 */
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
 * (4) Kernel Operations
 */
Kernel* createKernel(int width, int height) {
  Kernel* kernel = (Kernel*) malloc(sizeof(Kernel));

  if (kernel == NULL) {
    printf("Error: Could not allocate memory for Kernel.");
    return NULL;
  }

  kernel->cells = (Cell*) calloc(width * height, sizeof(Cell));

  if (kernel->cells == NULL) {
    printf("Error: Could not allocate memory for Kernel cells.");
    return NULL;
  }

  kernel->width = width;
  kernel->height = height;

  return kernel;
}

void freeKernel(Kernel* kernel) {
  free(kernel->cells);
  free(kernel);
}

PGM* applyKernel(PGM* input, Kernel* kernel) {
  PGM* output = createPGM(input->type, input->maxValue, input->width, input->height);

  // printf("\n%d %d %d\n%d %d %d\n%d %d %d\n\n",
  //   kernel->cells[0], kernel->cells[1], kernel->cells[2],
  //   kernel->cells[3], kernel->cells[4], kernel->cells[5],
  //   kernel->cells[6], kernel->cells[7], kernel->cells[8] 
  // );

  return output;
}

/**
 * (5) Filter Operations
 */
PGM* applySobelFilter(PGM* input) {
  // Create and fill kernel
  Kernel* kernel = createKernel(3, 3);
  Cell cells[9] = {1,2,3,4,5,6,7,8,9};
  memcpy(kernel->cells, cells, 9 * sizeof(Cell));

  PGM* output = applyKernel(input, kernel);
  freeKernel(kernel);

  return output;
}

/**
 * (6) Menu operations
 */
void print_incorrect_args() {
  printf("Error: Incorrect arguments, please check your arguments, type 'help' to see usages!\n");
}

void menu_sobel() {
  char* input = strtok(NULL, " ");
  char* output = strtok(NULL, " ");

  if (input == NULL) {
    print_incorrect_args();
    return;
  }

  if (output == NULL) {
    output = DEFUALT_OUTPUT_NAME;
  }

  PGM* pgm_input = readPGM(input);
  if (pgm_input == NULL) return;

  PGM* pgm_output = applySobelFilter(pgm_input);
  if (pgm_output != NULL) {
    writePGM(pgm_output, output);
    freePGM(pgm_output);
  };

  freePGM(pgm_input);

  printf("-> Sobel filter successfully applied to %s and result saved to %s\n", input, output);
}

void help() {
  printf("---------------------------------------------------------------------------------------------------------------\n");
  printf("$ %-45s %s\n", "sobel <input.pgm> [<output.cpgm>]", "- will use sobel filter to detect edges in input pgm (default output = "DEFUALT_OUTPUT_NAME")");
  printf("$ %-45s %s\n", "help", "- display this message");
  printf("$ %-45s %s\n", "exit", "- exit from program");
  printf("---------------------------------------------------------------------------------------------------------------\n");
}

/**
 * (7) Main
 */
int main() {
  printf("-------------------------------------\n");
  printf("Welcome to YTU Improc 2021\n");
  printf("Available commands are listed bellow:\n");
  help();
  
  while(true) {
    printf("$ ");
    char* line = scanLine();
    
    if (strcmp(line, "") != 0) {
      char* cmd = strtok(line, " ");

      if (strcmp(cmd, "sobel") == 0) {
        menu_sobel();
      } else if (strcmp(cmd, "help") == 0) {
        help();
      } else if (strcmp(cmd, "exit") == 0) {
        break;
      } else {
        printf("Error: Unknown command '%s'\n", cmd);
      }
    }

    free(line);
  }
  
  return 0;
}