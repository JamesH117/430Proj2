#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "structs.h"
#include "caster.c"
#include <math.h>

int line = 1;
int num_camera;
int total_objects;
int list_i = 0;


obj_camera main_camera;
scene_object *obj_list;
pixels *pixel_buffer;

static inline float square(float v){
	return v*v;
}
static inline void normalize(float* v){
	float len = sqrt(square(v[0] + square(v[1]) + square(v[2])));
	v[0] /= len;
	v[1] /= len;
	v[2] /= len;
}
static inline float vector_length(float* v){
    return sqrt(square(v[0])+square(v[1])+square(v[2]));

}

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

float next_number(FILE* json) {
  float value;
  fscanf(json, "%f", &value);
  //printf("Value is: %f\n", value);
  // Error check this..
  return value;
}

float* next_vector(FILE* json) {
  float* v = malloc(3*sizeof(float));
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

//Error Test for if a value is not given
void read_scene(char* filename) {
    int c;
    char current_object;

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
                float value = next_number(json);
                //printf("list_i is: %d\n", list_i);
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
                float* value = next_vector(json);
                //printf("list_i is: %d\n", list_i);
                if((strcmp(key, "color") == 0)){
                    obj_list[list_i].color = malloc(3*sizeof(float));
                    obj_list[list_i].color = value;
                }

                if((strcmp(key, "position") == 0)){
                    obj_list[list_i].position = malloc(3*sizeof(float));
                    obj_list[list_i].position = value;
                }

                if((strcmp(key, "normal") == 0)){
                    obj_list[list_i].normal = malloc(3*sizeof(float));
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
        //printf("Before increment, list_i is: %d\n", list_i);
        if(current_object != 'c'){
            list_i += sizeof(scene_object);
            //printf("Incremented by %d\n", sizeof(scene_object));
        }

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

float plane_intersection(float* Ro, float* Rd, float* position, float* normal){
    normalize(normal);
    float a = normal[0];
    float b = normal[1];
    float c = normal[2];
    //float d = vector_length(position);
    //Shouldnt d always be -1?
    float d = -1;
    float x0 = position[0];
    float y0 = position[1];
    float z0 = position[2];

    //printf("a: %f b: %f c: %f x0: %f y0: %f z0: %f ", a,b,c,x0,y0,z0);
    //printf("number : %f\n", (a*(Rd[0]) + b*(Rd[1]) + c*(Rd[2])));
    //printf("Rd is: %f %f %f\n", Rd[0], Rd[1], Rd[2]);

    //float numerator = (-a*Ro[0] + a*x0 -b*Ro[1] + b*y0 -c*Ro[2] + c*z0);
    //float denominator = (a*Rd[0] + b*Rd[1] + c*Rd[2]);
    float den = (a*Rd[0] + b*Rd[1] + c*Rd[2]);
    //if(den == 0) return -1;
    float t = -(a*Ro[0] + b*Ro[1] + c*Ro[2] + d)/(a*Rd[0] + b*Rd[1] + c*Rd[2]);
    //float t = numerator/denominator;
    //float t;
    //printf("t is: %f\n", t);
    return t;
}

float sphere_intersection(float* Ro, float* Rd, float* position, float radius){
    float x0 = position[0];
    float y0 = position[1];
    float z0 = position[2];

    float a = square(Rd[0])+square(Rd[1])+square(Rd[2]);
    float b = 2*(Rd[0]*(Ro[0]-x0) + Rd[1]*(Ro[1]-y0) + Rd[2]*(Ro[2]-z0));
    float c = square((Ro[0]-x0)) + square((Ro[1]-y0)) + square((Ro[2]-z0)) - square(radius);

    /*if(a != (float)1){
        fprintf(stderr, "Ray direction for Sphere was not normalized\n");
        exit(1);
        }*/

    float t0 = (-b - sqrt((square(b) - 4*c)))/2;
    float t1 = (-b + sqrt((square(b) - 4*c)))/2;

    if(t0 < t1) return t0;
    if(t0 > t1) return t1;
    if(t0>0) return t0;
    if(t1>0) return t1;

}

void raycast(float num_width, float num_height){
    int x = 0;
    int y = 0;
    int i;

    float Ro[3] = {0.0, 0.0, 0.0};
    float center[2] = {0.0,0.0};

    float width = main_camera.width;
    float height = main_camera.height;
    float N = num_width;
    float M = num_height;
    float pixel_width = width/N;
    float pixel_height = height/M;
    float p_z = 1;

    for(y=0; y<M; y+=1){
        float p_y = center[1] - height/2.0 + pixel_height*(y+0.5);

        for(x=0; x<N; x+=1){
            float p_x = center[0] - width/2.0 + pixel_width*(x+0.5);
            float Rd[3] = {p_x, p_y, p_z};
            //printf("Rd is: %f %f %f\n", Rd[0], Rd[1], Rd[2]);
            normalize(Rd);
            //printf("Normalized Rd is: %f %f %f\n", Rd[0], Rd[1], Rd[2]);

            for(i=0; i<=list_i; i+=sizeof(scene_object)){
                float t = 0;
                if(obj_list[i].type == 's'){
                        t = sphere_intersection(Ro, Rd, obj_list[i].position, obj_list[i].radius);
                        //printf("Hello\n");
                        //printf("t is: %f\n", t);
                }
                if(obj_list[i].type == 'p'){
                        t = plane_intersection(Ro, Rd, obj_list[i].position, obj_list[i].normal);
                        //printf("t is: %f\n", t);
                }
                if(t > 0){
                    //printf("%f\n", obj_list[i].color[2]);
                    float r = obj_list[i].color[0] * 255.0;
                    float g = obj_list[i].color[1] * 255.0;
                    float b = obj_list[i].color[2] * 255.0;
                    int int_r = (int)r;
                    int int_g = (int)g;
                    int int_b = (int)b;
                    int pos = (int)(y*N +x);
                    //printf("Position is: %d\n", pos);
                    pixel_buffer[pos].r = int_r;
                    pixel_buffer[pos].g = int_g;
                    pixel_buffer[pos].b = int_b;
                    //pixel_buffer[y*N + x].r = obj_list[i].color[0] * 255;
                    //pixel_buffer[y*N + x].g = obj_list[i].color[1] * 255;
                    //pixel_buffer[y*N + x].b = obj_list[i].color[2] * 255;
                }
            }







        }
    }


    /*int j=0;
    while(j <= list_i){
        if(obj_list[j].type == 's'){
            //Raycast sphere
            printf("Sphere\n");
        }
        if(obj_list[j].type == 'p'){
            //Raycast Plane
            //plane_intersection(Ro,Rd, obj_list[j].color,obj_list[j].position,obj_list[j].normal);
            printf("Plane\n");
        }
        j += sizeof(scene_object);
    }*/
}

int write(int w, int h){
    FILE *fp;
    char magic_number[2] = {'P', '6'};
    int width = w;
    int height = h;
    int j;

    fp = fopen("image.ppm", "wb");

    /*if(fp == 0){
        fprintf(stderr, "Error: Unable to create file '%s' \n", output_file);
        exit(1);
    }*/
    fwrite(magic_number, sizeof(magic_number), sizeof(magic_number)-1, fp);
    fprintf(fp,"\n%d %d", width, height);
    fprintf(fp,"\n%d", 255);

    fprintf(fp,"\n");
            for (j=0; j<width*height; j++){
                    /*if(j%70 == 0 && j != 0){
                        fwrite("\n", 1, 1, fp);
                    }*/
                    fwrite(&pixel_buffer[j].r,1,1, fp);
                    fwrite(&pixel_buffer[j].g,1,1, fp);
                    fwrite(&pixel_buffer[j].b,1,1, fp);
            }

    return 0;
}

int main(int c, char** argv) {
    obj_list = malloc(sizeof(scene_object)*128);
    float N = 1000;
    float M = 1000;
    pixel_buffer = (pixels*)malloc(sizeof(pixels)*N*M);
    memset(pixel_buffer, 255, 3*N*M);

    read_scene(argv[1]);
    printf("list_i is now: %d\n", list_i);
    raycast(N, M);
    write(N, M);
    //list_i -= sizeof(scene_object);
    //printf("type: %c color: %f %f %f\n", obj_list[0].type, obj_list[0].color[0], obj_list[0].color[1], obj_list[0].color[2]);
    return 0;
}
