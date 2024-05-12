#include <stdio.h>
#include <string>

#include "mesh/mesh.hpp"
#include "mesh/obj_file.hpp"
#include "util.hpp"

float consume_number(char* chars, int* idx) {
    std::string number;

    char c = '.';

    while (
        (chars[*idx] <= '9' && chars[*idx] >= '0') ||
        chars[*idx] == '.' || chars[*idx] == '-'
    ) {
        c = chars[*idx];
        number.push_back(chars[*idx]);
        *idx = *idx + 1;
    }

    return std::stof(number);
}

ObjFile parse_obj(char* chars) {
    ObjFile file;

    char c = ' ';
    int idx = 0;
    std::string line;

    while(chars[idx] != '\0') {
        while(chars[idx] != '\n') {
            line.push_back(chars[idx]);
            idx++;

            if(line.starts_with("v ")) {
                float x = consume_number(chars, &idx);
                idx++;
                float y = consume_number(chars, &idx);
                idx++;
                float z = consume_number(chars, &idx);
                file.objects[file.objects.size() - 1].verticies.push_back(Vertex { glm::vec3(x, y, z), glm::vec2(0.0f, 0.0f) });
            }else if(line.starts_with("vn ")) {
                float x = consume_number(chars, &idx);
                idx++;
                float y = consume_number(chars, &idx);
                idx++;
                float z = consume_number(chars, &idx);
                file.objects[file.objects.size() - 1].normals.push_back(glm::vec3(x, y, z));
            }else if(line.starts_with("f ")) {
                while(true) {
                    file.objects[file.objects.size() - 1].indicies.push_back(
                        (GLushort)consume_number(chars, &idx)
                    );
                    if(chars[idx] == '\n') break;
                    idx++;
                }
            }
        }

        if(line.starts_with("o ")) {
            line.erase(0, 2);
            auto obj = Object {
                .name = (char*)malloc(line.size() + 1)
            };

            memcpy(obj.name, line.c_str(), line.size());
            obj.name[line.size()] = '\0';

            file.objects.push_back(obj);
        }

        // printf("Line %s", line.c_str());

        line = "";
        idx++;
    }

    return file;
}

/*
 * @return Error code
*/
int Mesh::load_obj (const char* path) {
    char* chars = file_read(path);
    if(chars == NULL) return -1;

    auto obj_file = parse_obj(chars);

    vertices = obj_file.objects[0].verticies;
    normals = obj_file.objects[0].normals;
    indices = obj_file.objects[0].indicies;

    free(chars);

    return 0;
}