#include"0cc.h"

Vector* new_vector() {
  Vector* vec = malloc(sizeof(Vector));
  vec->data = malloc(sizeof(void*) * 16);
  vec->capacity = 16;
  vec->len = 0;
  return vec;
}

void vec_push(Vector* vec, void* elem) {
  if (vec->capacity == vec->len) {
    vec->capacity *= 2;
    vec->data = realloc(vec->data, sizeof(void*) * vec->capacity);
  }
  vec->data[vec->len++] = elem;
}

Map* new_map() {
  Map* map = malloc(sizeof(Map));
  map->keys = new_vector();
  map->vals = new_vector();
  map->types = new_vector();
  return map;
}

void map_put(Map* map, char* key, void* val, Type* type) {
  vec_push(map->keys, key);
  vec_push(map->vals, val);
  vec_push(map->types, type);
}

void* map_get(Map* map, char* key) {
  for (int i = map->keys->len - 1; i >= 0; i--)
    if (strcmp(map->keys->data[i], key) == 0)
      return map->vals->data[i];
  return NULL;
}

void* map_get_type(Map* map, char* key) {
  for (int i = map->keys->len - 1; i >= 0; i--)
    if (strcmp(map->keys->data[i], key) == 0)
      return map->types->data[i];
  return NULL;
}

void runtest();

// test for vector
int expect(int line, int expected, int actual) {
  if(expected == actual) return 0;
  fprintf(stderr, "%d: %d expected, but got %d\n", line, expected, actual);
  return 1;
} 

void runtest() {
  Vector* vec = new_vector();
  expect(__LINE__, 0, vec->len);

  for(int i=0;i<100;i++) vec_push(vec, (void*)(size_t)i);
  
  expect(__LINE__, 100, vec->len);
  expect(__LINE__, 0, (int)vec->data[0]);
  expect(__LINE__, 50, (int)vec->data[50]);
  expect(__LINE__, 99, (int)vec->data[99]);

  printf("OK\n");
}

