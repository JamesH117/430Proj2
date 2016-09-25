#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "structs.h"

int line = 1;
int num_camera;
int total_objects;
int list_i = 0;

obj_sphere *sphere_list;
obj_plane *plane_list;
obj_camera main_camera;
scene_object *obj_list;



char current_object;

// next_c() wraps the getc() function and provides error checking and line
// number maintenance
int next_c(FILE* json) {
  int c = fgetc(json);
#ifdef DEBUG
  printf("next_c: '%c'\n", c);
#endif
  if (c == '\n') {
    line += 1;
  }
  if (c == EOF) {
    fprintf(stderr, "Error: Unexpected end of file on line number %d.\n", line);
    exit(1);
  }
  return c;
}


// expect_c() checks that the next character is d.  If it is not it emits
// an error.
void expect_c(FILE* json, int d) {
  int c = next_c(json);
  if (c == d) return;
  fprintf(stderr, "Error: Expected '%c' on line %d.\n", d, line);
  exit(1);
}


// skip_ws() skips white space in the file.
void skip_ws(FILE* json) {
  int c = next_c(json);
  while (isspace(c)) {
    c = next_c(json);
  }
  ungetc(c, json);
}


// next_string() gets the next string from the file handle and emits an error
// if a string can not be obtained.
char* next_string(FILE* json) {
  char buffer[129];
  int c = next_c(json);
  if (c != '"') {
    fprintf(stderr, "Error: Expected string on line %d.\n", line);
    exit(1);
  }
  c = next_c(json);
  int i = 0;
  while (c != '"') {
    if (i >= 128) {
      fprintf(stderr, "Error: Strings longer than 128 characters in length are not supported.\n");
      exit(1);
    }
    if (c == '\\') {
      fprintf(stderr, "Error: Strings with escape codes are not supported.\n");
      exit(1);
    }
    if (c < 32 || c > 126) {
      fprintf(stderr, "Error: Strings may contain only ascii characters.\n");
      exit(1);
    }
    buffer[i] = c;
    i += 1;
    c = next_c(json);
  }
  buffer[i] = 0;
  return strdup(buffer);
}

double next_number(FILE* json) {
  double value;
  fscanf(json, "%f", &value);
  //printf("%f", value);
  // Error check this..
  return value;
}

double* next_vector(FILE* json) {
  double* v = malloc(3*sizeof(double));
  expect_c(json, '[');
  skip_ws(json);
  v[0] = next_number(json);
  skip_ws(json);
  expect_c(json, ',');
  skip_ws(json);
  v[1] = next_number(json);
  skip_ws(json);
  expect_c(json, ',');
  skip_ws(json);
  v[2] = next_number(json);
  skip_ws(json);
  expect_c(json, ']');
  return v;
}


void read_scene(char* filename) {
  int c;
  FILE* json = fopen(filename, "r");

  if (json == NULL) {
    fprintf(stderr, "Error: Could not open file \"%s\"\n", filename);
    exit(1);
  }

  skip_ws(json);

  // Find the beginning of the list
  expect_c(json, '[');

  skip_ws(json);

  // Find the objects

  while (1) {
    c = fgetc(json);
    if (c == ']') {
      fprintf(stderr, "Error: This is the worst scene file EVER.\n");
      fclose(json);
      return;
    }
    if (c == '{') {
      skip_ws(json);

      // Parse the object
    char* key = next_string(json);
    if (strcmp(key, "type") != 0) {
        fprintf(stderr, "Error: Expected \"type\" key on line number %d.\n", line);
        exit(1);
      }
    skip_ws(json);
    expect_c(json, ':');
    skip_ws(json);
    char* value = next_string(json);
    if (strcmp(value, "camera") == 0) {
        if(num_camera >1){
            fprintf(stderr, "Error: More than one camera object has been provided, will not render.");
            exit(1);
        }
        num_camera += 1;
        //Working with camera object
        //Put future values into camera object
        current_object = 'c';
        printf("Working with a %camera\n", current_object);
      }
    else if (strcmp(value, "sphere") == 0) {
        //Working with Sphere object
        //Put future values into sphere object and then put into list
        total_objects += 1;
        current_object = 's';
        obj_list[list_i].type = 's';
        printf("Working with a %cphere\n", current_object);
      }
    else if (strcmp(value, "plane") == 0) {
        //Working with Plane object
        //Put future values into plane object and then put into list
        total_objects += 1;
        current_object = 'p';
        obj_list[list_i].type = 'p';
        printf("Working with a %clane\n", current_object);
      }
    else {
        fprintf(stderr, "Error: Unknown type, \"%s\", on line number %d.\n", value, line);
        exit(1);
      }
    skip_ws(json);
    while (1) {
	// , }
        c = next_c(json);
        if (c == '}') {
            // stop parsing this object
            break;
        }
        else if (c == ',') {
            // read another field
            skip_ws(json);
            char* key = next_string(json);
            skip_ws(json);
            expect_c(json, ':');
            skip_ws(json);
            if ((strcmp(key, "width") == 0) || (strcmp(key, "height") == 0) || (strcmp(key, "radius") == 0)) {
                //Depending on which is key, put value into that object value
                double value = next_number(json);
                printf("list_i is: %d\n", list_i);
                if((strcmp(key, "width") == 0))
                    main_camera.width = value;
                if((strcmp(key, "height") == 0))
                    main_camera.height = value;
                if((strcmp(key, "radius") == 0))
                    obj_list[list_i].radius = value;
                //printf("%f\n", &value);
            }
            else if ((strcmp(key, "color") == 0) || (strcmp(key, "position") == 0) || (strcmp(key, "normal") == 0)) {
                //Depending on which is key, put value into that object *value
                double* value = next_vector(json);
                printf("list_i is: %d\n", list_i);
                if((strcmp(key, "color") == 0)){
                    obj_list[list_i].color = malloc(3*sizeof(double));
                    obj_list[list_i].color = value;
                }

                if((strcmp(key, "position") == 0)){
                    obj_list[list_i].position = malloc(3*sizeof(double));
                    obj_list[list_i].position = value;
                }

                if((strcmp(key, "normal") == 0)){
                    obj_list[list_i].normal = malloc(3*sizeof(double));
                    obj_list[list_i].normal = value;
                }

            }
            else {
                fprintf(stderr, "Error: Unknown property, \"%s\", on line %d.\n",
                key, line);
                //char* value = next_string(json);
            }
            skip_ws(json);
        }
        else {
            fprintf(stderr, "Error: Unexpected value on line %d\n", line);
            exit(1);
        }
      }
    skip_ws(json);
    c = next_c(json);
    if (c == ',') {
        //No Operation, another object is coming up
        //Iterate through list of objects by size of 1 object
        skip_ws(json);
        if(current_object != 'c')
            list_i += sizeof(scene_object);
        printf("Incremented by %d\n", sizeof(scene_object));
    }
    else if (c == ']') {
        //Iterated through all objects
        fclose(json);
        return;
    }
    else {
        fprintf(stderr, "Error: Expecting ',' or ']' on line %d.\n", line);
        exit(1);
      }
    }
  }
}

int main(int c, char** argv) {
    obj_list = malloc(sizeof(scene_object)*128);
    read_scene(argv[1]);
    //list_i -= sizeof(scene_object);
    printf("type: %c color: %f %f %f\n", obj_list[0].type, obj_list[0].color[0], obj_list[0].color[1], obj_list[0].color[2]);
    return 0;
}
