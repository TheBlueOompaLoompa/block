#include "util.hpp"

// COPY: From a tutorial on how to load shader files
char* file_read(const char* filename) {
	SDL_RWops *rw = SDL_RWFromFile(filename, "rb");
	if (rw == NULL) return NULL;
	
	Sint64 res_size = SDL_RWsize(rw);
	char* res = (char*)malloc(res_size + 1);

	Sint64 nb_read_total = 0, nb_read = 1;
	char* buf = res;
	while (nb_read_total < res_size && nb_read != 0) {
		nb_read = SDL_RWread(rw, buf, 1, (res_size - nb_read_total));
		nb_read_total += nb_read;
		buf += nb_read;
	}
	SDL_RWclose(rw);
	if (nb_read_total != res_size) {
		free(res);
		return NULL;
	}
	
	res[nb_read_total] = '\0';
	return res;
}

void chk_mkdir(const char* dir_path) {
	if(!std::filesystem::exists(dir_path)) {
		mkdir(dir_path, 0777);
	}
}

void save_data(const char* fname, void* src, size_t size) {
	auto file = fopen(fname, "w");

	fwrite(src, size, 1, file);
	fclose(file);
}

void load_data(const char* fname, void* dest, size_t size) {
	auto file = fopen(fname, "r");

	fread(dest, size, 1, file);
	fclose(file);
}