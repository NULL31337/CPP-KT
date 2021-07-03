#include <cstring>
#include <iostream>

size_t recalc_pref_func(const char* a, const size_t* tab, char b, const size_t* prev) {
  size_t tmp = *prev;
  while (tmp != 0 && a[tmp] != b) {
    tmp = tab[tmp - 1];
  }
  return tmp + (a[tmp] == b);
}

int main(int argc, char *argv[]) {
  if (argc != 3) {
    std::cerr << "Wrong arguments. Expected: the_name_of_the_input_file string_to_find.\n";
    return -1;
  }
  const char* string_to_find = argv[2];
  size_t n = strlen(string_to_find);
  size_t *max_prefix = (size_t*)(malloc(n * sizeof(size_t)));
  if (max_prefix == nullptr) {
    std::cerr << "Can't allocate memory";
    return -1;
  }
  max_prefix[0] = 0;
  for (size_t i = 1; i < n; i++) {
    max_prefix[i] = recalc_pref_func(string_to_find, max_prefix, string_to_find[i], &max_prefix[i - 1]);
  }
  FILE *file = fopen(argv[1], "r");
  if (!file) {
    perror("IOError: Open file.\n");
    free(max_prefix);
    return -1;
  }
  size_t tmp = 0;
  int c;
  while ((c = fgetc(file)) != EOF) {
    tmp = recalc_pref_func(string_to_find, max_prefix, (char)c, &tmp);
    if (tmp == n) {
      std::cout << "Yes" << '\n';
      free(max_prefix);
      fclose(file);
      return 0;
    }
  }
  free(max_prefix);
  if (ferror(file)) {
    char const *str = strerror(errno);
    std::cerr << "Read failed: " << str << '\n';
    fclose(file);
    return -1;
  }
  std::cout << "No" << '\n';
  fclose(file);
  return 0;
}
