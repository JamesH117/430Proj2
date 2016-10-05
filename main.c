#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include "structs.h"
#include <math.h>

int line = 1;
int num_camera;
int total_objects;
int list_i = 0;


obj_camera main_camera;
scene_object *obj_list;
pixels *pixel_buffer;

static inline double square(double v){
	return v*v;
}
static inline void normalize(double* v){
	double len = sqrt(square(v[0] + square(v[1]) + square(v[2])));
	v[0] /= len;
	v[1] /= len;
	v[2] /= len;
}
static inline double vector_length(double* v){
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

double next_number(FILE* json) {
  double value;
  fscanf(json, "%lf", &value);
  //printf("Value is: %lf\n", value);
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
        //printf("Working with a %camera\n", current_object);
      }
    else if (strcmp(value, "sphere") == 0) {
        //Working with Sphere object
        //Put future values into sphere object and then put into list
        total_objects += 1;
        current_object = 's';
        obj_list[list_i].type = 's';
        //printf("Working with a %cphere\n", current_object);
      }
    else if (strcmp(value, "plane") == 0) {
        //Working with Plane object
        //Put future values into plane object and then put into list
        total_objects += 1;
        current_object = 'p';
        obj_list[list_i].type = 'p';
        //printf("Working with a %clane\n", current_object);
      }
    else {
        fprintf(stderr, "Error: Unknown type, \"%s\", on line number %d.\n", value, line);
        exit(1);
      }
    skip_ws(json);
    int cam_vars = 0;
	int obj_vars = 0;
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

                if((strcmp(key, "width") == 0)){
                    main_camera.width = value;
                    cam_vars+=1;
                }

                if((strcmp(key, "height") == 0)){
                    main_camera.height = value;
                    cam_vars +=1;
                }

                if((strcmp(key, "radius") == 0)){
                    obj_list[list_i].radius = value;
                    obj_vars +=1;
                }


            }
            else if ((strcmp(key, "color") == 0) || (strcmp(key, "position") == 0) || (strcmp(key, "normal") == 0)) {
                //Depending on which is key, put value into that object *value
                double* value = next_vector(json);

                if((strcmp(key, "color") == 0)){
                    obj_list[list_i].color = malloc(3*sizeof(double));
                    obj_list[list_i].color = value;
                    if(obj_list[list_i].color[0] > 1 || obj_list[list_i].color[3] > 1 || obj_list[list_i].color[2] > 1){
                        fprintf(stderr, "ERROR: Some color value is greater than 1.\n");
                        exit(1);
                    }
                    obj_vars +=1;
                }

                if((strcmp(key, "position") == 0)){
                    obj_list[list_i].position = malloc(3*sizeof(double));
                    obj_list[list_i].position = value;
                    obj_vars +=1;
                }

                if((strcmp(key, "normal") == 0)){
                    obj_list[list_i].normal = malloc(3*sizeof(double));
                    obj_list[list_i].normal = value;
                    obj_vars +=1;
                }

            }
            else {
                fprintf(stderr, "Error: Unknown property, \"%s\", on line %d.\n",
                key, line);

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
            if(obj_vars != 3){
                fprintf(stderr, "Error: One of your objects does not have the correct amount of parameters to render, each object needs only 3!\n");
                exit(1);
            }
            obj_vars = 0;
            //printf("Incremented by %d\n", sizeof(scene_object));
        }
        if(current_object == 'c'){
            if(cam_vars != 2){
            fprintf(stderr, "ERROR: Your camera has either too little parameters or too many parameters.\n");
            exit(1);
            }
        cam_vars = 0;
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

double plane_intersection(double* Ro, double* Rd, double* position, double* normal){
    normalize(normal);
    double a = normal[0];
    double b = normal[1];
    double c = normal[2];

    //D is the length of the shortest line segment from the origin to the plane
    //D is distance from origin/camera to the plane


    double x0 = position[0];
    double y0 = position[1];
    double z0 = position[2];

    double d = -(a*x0 + b*y0 + c*z0);


    double den = (a*Rd[0] + b*Rd[1] + c*Rd[2]);
    if(den == 0.0) return -1;
    double t = -(a*Ro[0] + b*Ro[1] + c*Ro[2] + d)/(a*Rd[0] + b*Rd[1] + c*Rd[2]);

    return t;
}

double sphere_intersection(double* Ro, double* Rd, double* position, double radius){
    //Center points of sphere
    double xc = position[0];
    double yc = position[1];
    double zc = position[2];


    double a = square(Rd[0])+square(Rd[1])+square(Rd[2]);
    double b = 2*(Rd[0]*(Ro[0]-xc) + Rd[1]*(Ro[1]-yc) + Rd[2]*(Ro[2]-zc));
    double c = (square((Ro[0]-xc)) + square((Ro[1]-yc)) + square((Ro[2]-zc)) - square(radius));

    double desc = (square(b) - 4*a*c);

    //If descriminate is negative, no intersection
    if(desc < 0.0) return -1;

    double t0 = ((-b - sqrt(square(b) - 4.0*c*a))/(2.0*a));
    double t1 = ((-b + sqrt(square(b) - 4.0*c*a))/(2.0*a));

    //If descriminant is negative, imaginary number, dont return
    //If t0 is negative don't return it, return t1
    //Return t0 first
    //Return t1 next
    if(t0 > 0.0)return t0;
    if(t1> 0.0) return t1;


    return -1;


}

void raycast(double num_width, double num_height){
    int x = 0;
    int y = 0;
    int i;

    //Where the camera is sitting
    double Ro[3] = {0.0, 0.0, 0.0};


    double width = main_camera.width;
    double height = main_camera.height;
    double N = num_width;
    double M = num_height;
    double pixel_width = width/N;
    double pixel_height = height/M;

    //Center of the view plane
    double center[2] = {0.0, 0.0};


    //c_xyz is center of the view plane

    //Distance from camera to view plane
    double p_z = 1;

    for(y=0; y<M; y+=1){

        double p_y = center[1] - height/2.0 + pixel_height*(y+0.5);

        for(x=0; x<N; x+=1){

            double p_x = center[0] - width/2.0 + pixel_width*(x+0.5);
            double Rd[3] = {p_x, p_y, p_z};
            //printf("Rd is: %lf %lf %lf\n", Rd[0], Rd[1], Rd[2]);
            normalize(Rd);
            //printf("Normalized Rd is: %lf %lf %lf\n", Rd[0], Rd[1], Rd[2]);

            double best_t = INFINITY;
            double *best_c;
            for(i=0; i<=list_i; i+=sizeof(scene_object)){
                double t = 0;
                if(obj_list[i].type == 's'){
                        t = sphere_intersection(Ro, Rd, obj_list[i].position, obj_list[i].radius);
                }
                if(obj_list[i].type == 'p'){
                        t = plane_intersection(Ro, Rd, obj_list[i].position, obj_list[i].normal);
                }
                if(t > 0 && t < best_t){
                    best_t = t;

                    best_c = obj_list[i].color;
                }


                if(best_t > 0 && best_t != INFINITY){

                    double r = best_c[0]*255;
                    double g = best_c[1]*255;
                    double b = best_c[2] * 255;
                    int int_r = (int)r;
                    int int_g = (int)g;
                    int int_b = (int)b;
                    int pos = (int)((M - y -1)*N +x);

                    pixel_buffer[pos].r = int_r;
                    pixel_buffer[pos].g = int_g;
                    pixel_buffer[pos].b = int_b;

                }
            }
        }
    }
}

int write(int w, int h, FILE* output_image){
    FILE *fp;
    char magic_number[2] = {'P', '6'};
    int width = w;
    int height = h;
    int j;

    fp = fopen(output_image, "wb");

    if(fp == 0){
        fprintf(stderr, "Error: Unable to create file for output image.\n");
        exit(1);
    }
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

int main(int argc, char** argv) {
    if(argc != 5){
        fprintf(stderr, "Error: Not all arguments were provided or too many were given.\n");
        exit(1);
    }
    obj_list = malloc(sizeof(scene_object)*128);
    double N = (double)atoi(argv[1]);
    double M = (double)atoi(argv[2]);
    pixel_buffer = (pixels*)malloc(sizeof(pixels)*N*M);
    memset(pixel_buffer, 0, 3*N*M);

    read_scene(argv[3]);

    raycast(N, M);
    write(N, M, argv[4]);

    return 0;
}
