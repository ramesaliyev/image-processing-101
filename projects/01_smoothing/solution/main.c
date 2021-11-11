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

/** (0) Program configuration definitions. */
#define LINESIZE 256
#define NLCHR '\n'
#define NLSTR "\n"
#define MAX_COLOR 255
#define P5 "P5"
#define P2 "P2"
#define COMMENT_IDENTIFIER '#'
#define DEFUALT_OUTPUT_NAME "output.pgm"

/** (1) Type definitions and data structures. */
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

/** (2) Common generic utilities. */
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

int comparator(const void *p, const void *q) { 
  return (*(int*)p - *(int*)q);
}

/** (3) PGM related functions. */
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

  pgm->pixels = (Pixel*) calloc(width * height, sizeof(Pixel));
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
  int pixelCount = pgm->width * pgm->height;

  // Read pixel values into pgm.
  if (!strcmp(pgmType, P5)) { // Read P5
    fread(pgm->pixels, sizeof(Pixel), pixelCount, file);
  } else { // Read P2
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
  int pixelCount = pgm->width * pgm->height;

  // Write header
  fprintf(file, "%s\n%d %d\n%d\n", type, pgm->width, pgm->height, pgm->maxValue);

  // Write pixels.
  if (!strcmp(type, P5)) { // Write P5
    fwrite(pgm->pixels, sizeof(Pixel), pixelCount, file);
  } else { // Write P2
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

/** (4) Neighbors Operations */
Neighbors* createNeighbors(int distance) {
  Neighbors* neighbors = (Neighbors*) malloc(sizeof(Neighbors));
  neighbors->distance = distance;
  neighbors->cells = (int*) calloc(distance * distance, sizeof(int));
  return neighbors;
}

void freeNeighbors(Neighbors* neighbors) {
  free(neighbors->cells);
  free(neighbors);
}

/** (5) Image-Processing Operations */
PGM* applyKernel(PGM* input, int ker_size, Kernel kernel) {
  int in_width = input->width;
  int in_height = input->height;
  int ker_margin = ker_size - 1;
  int out_width = in_width - ker_margin;
  int out_height = in_height - ker_margin;

  PGM* output = createPGM(input->type, input->maxValue, out_width, out_height);
  Neighbors* neighbors = createNeighbors(ker_size);

  int r, c, x, y; // multi-dimensional indexes.
  // process inner rows and columns with margin
  for (r = 0; r < out_height; r++) {
    for (c = 0; c < out_width; c++) {
      // fill neighbors flat matrix (target matrix of convolution)
      for (y = 0; y < ker_size; y++) {
        for (x = 0; x < ker_size; x++) {
          int nbor_index = (y * ker_size) + x;
          int in_index = ((r + y) * in_width) + c + x;

          neighbors->cells[nbor_index] = (int) input->pixels[in_index];
        }
      }

      // Pass neighbors flat matrix to kernel function to process, and set result.
      int out_index = (r * out_width) + c;
      output->pixels[out_index] = kernel(neighbors);
    }  
  }

  freeNeighbors(neighbors);
  return output;
}

int averageFilterKernel(Neighbors* neighbors) {
  float count = neighbors->distance * neighbors->distance;
  float sum = 0;
  int i;

  // sum neighbors
  for (i = 0; i < count; i++) {
    sum += neighbors->cells[i];
  }

  // find and return average
  return (int) round(sum / count);
}

int medianFilterKernel(Neighbors* neighbors) {
  int count = neighbors->distance * neighbors->distance;
  int* cells = neighbors->cells;

  // sort cells (with built-in qsort)
  qsort((void*)cells, count, sizeof(cells[0]), comparator); 

  // find and median
  return cells[count / 2];
}

PGM* applyMirrorPadding(PGM* input, int size) {
  int in_width = input->width;
  int in_height = input->height;
  int out_width = in_width + (size * 2);
  int out_height = in_height + (size * 2);

  PGM* output = createPGM(input->type, input->maxValue, out_width, out_height);
  Pixel* in_pixels = input->pixels;
  Pixel* out_pixels = output->pixels;

  int r;
  for (r = 0; r < out_height; r++) {
    int ri_out = (r * out_width);
    int ri_in = 0;

    if (r >= size && r < out_height - size) {
      ri_in = ((r - size) * in_width);
    } else if (r >= out_height - size) {
      ri_in = ((in_height - 1) * in_width);
    }
 
    memset(out_pixels + ri_out, in_pixels[ri_in], size);
    memcpy(out_pixels + ri_out + size, (in_pixels + ri_in), in_width);
    memset(out_pixels + ri_out + out_width - size, in_pixels[ri_in + in_width - 1], size);
  }

  return output;
}

/** (6) Main */
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
  char* cmd = argv[1];
  char* input = argv[2];
  char* output = argc == 4 ? argv[3] : DEFUALT_OUTPUT_NAME;
  
  // Handle missing arguments and help command.
  if (argc <= 1 || streq(cmd, "help")) return printHelpMsg();
  if (argc == 2) return printIncorrectArgsMsg();

  // Handle filter commands.
  Kernel kernel;
  int kernelSize = 3; 

  if (streq(cmd, "average")) {
    kernel = averageFilterKernel;
  } else if (streq(cmd, "median")) {
    kernel = medianFilterKernel;
  } else {
    return printIncorrectArgsMsg();
  }

  // Read PGM.
  PGM* pgm_input = readPGM(input);
  if (pgm_input == NULL) return 0;

  // Apply kernel of choice and padding.
  PGM* pgm_output = applyKernel(pgm_input, kernelSize, kernel);
  PGM* pgm_padded = applyMirrorPadding(pgm_output, kernelSize / 2);
  // Write result.
  writePGM(pgm_padded, output);

  freePGM(pgm_padded);
  freePGM(pgm_output);
  freePGM(pgm_input);

  printf("-> %s filter successfully applied to %s and result saved to %s\n", cmd, input, output);
  return 0;
}
