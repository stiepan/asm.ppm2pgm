#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <ctype.h>

struct Pixel;
struct Image;

typedef struct Pixel** Matrix;
typedef struct Image* Image;

extern char weight_r, weight_g, weight_b;

extern char* to_grey(Matrix, int, int);

char last_error[1000];

struct Pixel
{
    char rgb[3];
};

struct Image
{
    int width, height, maxval;
    Matrix m;
};

void discard_comment(FILE* fptr)
{
    int is_comment = 0;
    char last_c;
    while (!feof(fptr)) {
        last_c = getc(fptr);
        if (isspace(last_c)) {
            if (last_c == '\n' || last_c == '\r') {
                is_comment = 0;
            }
            continue;
        }
        if (!is_comment) {
            if (last_c != '#') {
                ungetc(last_c, fptr);
                break;
            }
            is_comment = 1;
            continue;
        }
    }
    return;
}

int parse_header(FILE* fptr, Image image)
{
    int version;
    int vls[3];
    discard_comment(fptr);
    if (fscanf(fptr, "P%d", &version) != 1) {
        sprintf(last_error, "Error on reading format");
        return 1;
    }
    if (version != 3) {
        sprintf(last_error, "Unsupported format P%d. Expected P3.", version);
        return 1; 
    }
    for (int i = 0; i < 3; i++) {
        discard_comment(fptr);
        if (fscanf(fptr, "%d", vls + i) != 1) {
            sprintf(last_error, "Error while reading image header (%d)", i);
            return 1;
        }
    }
    image->width = vls[0];
    image->height = vls[1];
    image->maxval= vls[2];
    if (image->width < 1 || image->height < 1) {
        sprintf(last_error, "Incorrect raster size");
        return 1;
    }
    discard_comment(fptr);
    return 0;
}

int read_image(FILE* fptr, Image image)
{
    if (parse_header(fptr, image)) {
        return 1;
    }

    image->m = (Matrix)malloc(sizeof(struct Pixel*) * image->height);
    if (!image->m) {
        sprintf(last_error, "Malloc failure");
        return 1;
    }
    image->m[0] = (struct Pixel*)malloc(sizeof(struct Pixel) * image->height * image->width);
    if (!image->m[0]) {
        sprintf(last_error, "Malloc failure (2)");
        return 1;
    }
    for (int x = 0; x < image->height; x++) {
        image->m[x] = image->m[0] + x * image->width;
        for (int y = 0; y < image->width; y++) {
            for (int i = 0; i < 3; i++) {
                if (fscanf(fptr, "%hhu", image->m[x][y].rgb + i) != 1) {
                    sprintf(last_error, "Error while reading colour value");
                    return 1;
                }
            }
        }
    }
    return 0;
}

void free_image(Image image)
{
    if (!image) {
        return;
    }

	if (image->m) {
		if (*image->m) {
			free(*image->m);
		}
		free(image->m);
	}

	free(image);
}

void save_image(Image image, char* grey, char* filename)
{
	char generic_name[] = "a.pgm";
	char* n_name = generic_name;
	int fname_len = strlen(filename);
	char* ext = filename + fname_len - 3;
	FILE* fptr = NULL;
	int saved = 0;
	if (fname_len > 4 && strcmp(ext, "ppm") == 0) {
		strcpy(ext, "pgm");
		n_name = filename;
	}

	do {
        fptr = fopen(n_name, "w");
        if (!fptr) {
            sprintf(last_error, "Could not open file for writing '%s'", n_name);
            break;
        }
		fprintf(fptr, "P2\n%d %d\n%d\n", image->width, image->height, image->maxval);
		for (int x = 0; x < image->height; x++) {
			for (int y = 0; y < image->width; y++) {
				fprintf(fptr, "%hhu ", grey[x * image->width + y]);
			}
			fprintf(fptr, "\n");
		}
		saved = 1;
    }
    while (0);

	if (fptr) {
        if (fclose(fptr)) {
            sprintf(last_error, "Error while closing the file");
        }
		else if (saved) {
			printf("Saved as %s\n", n_name);
		}
    }
}

int main(int argv, char** argc)
{
    FILE* fptr = NULL;
	Image image = (Image)malloc(sizeof(*image));
    int loaded = 0;
    int loaded_r, loaded_g;
	
    do {
		if (!image) {
			sprintf(last_error, "Image allocation failure");
			break;
		}
        if (argv < 2) {
            sprintf(last_error,
                "Usage: %s filename [red_weight green_weight]\n"
                "Blue coloour = 256 - red and green", argc[0]);
            break;
        }
        if (argv >= 4) {
            loaded_r = atoi(argc[2]);
            loaded_g = atoi(argc[3]);
            if ((loaded_r == 0 && argc[2][0] != '0') || 
                (loaded_g == 0 && argc[3][0] != '0') ||
                loaded_r < 0 || loaded_g < 0) {
                sprintf(last_error, "Colour weight must be a number 0-255");
                break;
            }
            if ((loaded_r + loaded_g) > 255 || (loaded_r + loaded_g) < 1) {
                sprintf(last_error, "Red and green sum must be within 1-255 range");
                break;
            }
            weight_r = (char)loaded_r;
            weight_g = (char)loaded_g;
            weight_b = (char)(256 - loaded_r - loaded_g);
        }
        fptr = fopen(argc[1], "r");
        if (!fptr) {
            sprintf(last_error, "Could not open file '%s'", argc[1]);
            break;
        }
        if (read_image(fptr, image)) {
            break;
        }
        loaded = 1;
    }
    while (0);

    if (loaded) {
        char* grey = to_grey(image->m, image->width, image->height);
        if (!grey) {
            sprintf(last_error, "Conversion error - no memeory allocated");
        }
        else {
            save_image(image, grey, argc[1]);
            free(grey);
        }
    }

    free_image(image);

    if (fptr) {
        if (fclose(fptr)) {
            sprintf(last_error, "Error while closing the file");
        }
    }

    if (strlen(last_error)) {
        printf("%s\n", last_error);
    }
    return 0;
}
