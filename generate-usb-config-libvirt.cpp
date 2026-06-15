#include <cstdio>
#include <cstring>

bool check_n_conv(char *c) {
    if ('0' <= *c && '9' >= *c) {
        return true;
    } else if ('A' <= *c && *c <= 'Z') {
        *c += 'a' - 'A';
        return true;
    } else if ('a' <= *c && *c <= 'z') {
        return true;
    } else {
        return false;
    }
}

int main(int argc, char **argv) {
    if (argc != 2) {
        fprintf(stderr, "Usage: %s XXXX:XXXX\n", argv[0]);
        return 1;
    }

    char *id = argv[1];
    if (strlen(id) != 9) {
        fprintf(stderr, "Error: USB device ID has to be of format XXXX:XXXX\n");
        return 2;
    }
    bool valid = true;
    valid &= check_n_conv(id);
    valid &= check_n_conv(id + 1);
    valid &= check_n_conv(id + 2);
    valid &= check_n_conv(id + 3);
    valid &= id[4] == ':';
    valid &= check_n_conv(id + 5);
    valid &= check_n_conv(id + 6);
    valid &= check_n_conv(id + 7);
    valid &= check_n_conv(id + 8);
    if (!valid) {
        fprintf(stderr, "Error: USB device ID has to be of format XXXX:XXXX\n");
        return 3;
    }
    id[4] = 0;
    printf("<hostdev mode='subsystem' type='usb'><source><vendor "
           "id='0x%s'/><product id='0x%s'/></source></hostdev>",
           id, id + 5);

    return 0;
}
