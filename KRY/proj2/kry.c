/**
 * @file kry.c
 * @author Adam Zvara - 232242 (xzvara01@stud.fit.vutbr.cz)
 * @brief Implementation of MAC using SHA-256 and length extension attack
 * @date 04/2024
 */

#include <stdio.h>
#include <unistd.h>
#include <stdbool.h>
#include <stdlib.h>
#include <ctype.h>
#include <stdint.h>
#include <string.h>
#include <assert.h>
#include <endian.h>

/* ======================== Parsing arguments ======================== */
enum action {CHKSUM = 1, GENMAC, VALMAC, EXTATTACK};

struct prog_args {
    enum action action;
    char *key; // Secret key to generate MAC
    char *chs; // MAC of input message
    char *attack_msg; // Format of the message to be used in the length extension attack
    unsigned int attack_key_len; // Length of the secret key
};

// Add action to the struct if it is not already present
int args_assign_action(struct prog_args *args, const enum action action) {
    if (args->action != 0) {
        fprintf(stderr, "Error: Multiple actions specified\n");
        return 1;
    }

    args->action = action;
    return 0;
}

void print_usage() {
    printf("Usage: kry [OPTION] INPUT\n"\
           "Options:\n"\
           "  -h\t\tPrint this help message\n"\
           "  -c\t\tCalculate checksum (SHA-256) of given input\n"\
           "  -s\t\tGenerate MAC using SHA-256 of given input (must be used with -k)\n"\
           "  -v\t\tValidate MAC of given input (must be used with -k -m)\n"\
           "  -e\t\tExecute length extension attack of given MAC (must be used with -n -m -a)\n"\
           "  -k KEY\tSecret key used to generate MAC\n"\
           "  -m CHS\tMAC of the input\n"\
           "  -n NUM\tSecret key length\n"\
           "  -a MSG\tMAC\n");
}

// Parse arguments and assign them to the struct
// Source: https://www.gnu.org/software/libc/manual/html_node/Example-of-Getopt.html
int parse_args(int argc, char **argv, struct prog_args *args) {
    int res = 0;
    if (argc < 2) {
        print_usage();
        return 1;
    }

    int opt;
    while ((opt = getopt(argc, argv, "hcsvek:m:n:a:")) != -1) {
        switch (opt) {
            case 'h':
                print_usage();
                return 1;
            case 'c':
                res = args_assign_action(args, CHKSUM);
                break;
            case 's':
                res = args_assign_action(args, GENMAC);
                break;
            case 'v':
                res = args_assign_action(args, VALMAC);
                break;
            case 'e':
                res = args_assign_action(args, EXTATTACK);
                break;
            case 'k':
                args->key = optarg;
                break;
            case 'm':
                args->chs = optarg;
                break;
            case 'n':
                args->attack_key_len = atoi(optarg);
                break;
            case 'a':
                args->attack_msg = optarg;
                break;
            default:
                return 1;
        }
    }

    if (res != 0) {
        return res;
    }

    // Check if required arguments for chosen action are present
    if (args->action == GENMAC && args->key == NULL) {
        fprintf(stderr, "Error: -s requires -k\n");
        return 1;
    } else if (args->action == VALMAC && (args->key == NULL || args->chs == NULL)) {
        fprintf(stderr, "Error: -v requires -k and -m\n");
        return 1;
    } else if (args->action == EXTATTACK && (args->chs == NULL || args->attack_msg == NULL || args->attack_key_len == 0)) {
        fprintf(stderr, "Error: -e requires -a, -m and -n\n");
        return 1;
    } else if (args->action == 0) {
        fprintf(stderr, "Error: No action specified\n");
        return 1;
    }

    return 0;
}

/* ======================== Input data manipulation ======================== */

struct input {
    uint8_t *data;  // Input message
    size_t cap;     // Capacity of the buffer
    size_t len;     // Used length
};

// Reallocate input data buffer
void input_realloc(struct input *in, const size_t new_cap) {
    in->cap = new_cap;
    void *data_tmp = realloc(in->data, in->cap);
    if (data_tmp == NULL) {
        fprintf(stderr, "Error: Memory allocation failed\n");
        free(in->data);
        exit(1);
    }
    in->data = data_tmp;
}

// Reallocate input data buffer if needed
void _realloc_needed(struct input *in, const size_t new_len) {
    if (new_len >= in->cap) {
        input_realloc(in, new_len * 2);
    }
}

// Append character to input data
void input_append_char(struct input *in, const int c) {
    _realloc_needed(in, in->len + 1);
    in->data[in->len] = c;
    in->len++;
}

// Append sequence `seq` of size `len` to input data
void input_append_seq(struct input *in, const void *seq, const size_t len) {
    _realloc_needed(in, in->len + len);
    memcpy(in->data + in->len, seq, len);
    in->len += len;
}

// Insert sequence `seq` of size `len` to the start of input data
void input_insert_start_seq(struct input *in, const void *seq, const size_t len) {
    _realloc_needed(in, in->len + len);
    memmove(in->data + len, in->data, in->len);
    memcpy(in->data, seq, len);
    in->len += len;
}

// Read input from stdin
void input_read_stdin(struct input *in) {
    int c;
    while ((c = fgetc(stdin)) != EOF) {
        input_append_char(in, c);
    }
}

// Print input data to stdout ending with newline
void input_print(const struct input *in) {
    for (size_t i = 0; i < in->len; i++) {
        printf("%c", in->data[i]);
    }
    printf("\n");
}

// Free input data buffer
void input_free(struct input *in) {
    free(in->data);
}

/* ======================== SHA-256 ======================== */

/* Add padding and parse as described in Secure Hash Standard (SHS)
   `prev_length` is added to length of the original message in `in`
   (0 for normal SHA-256, >0 for length extension attack) */
void padding(struct input *in, const uint32_t prev_length) {
    uint64_t original_len = in->len;
    // Always append 0x80
    input_append_char(in, 0x80);

    // Calculate the remaining padding up to 448 bits
    const size_t base = original_len % 64;
    // If the padding + length does not fit into current block, add another block
    const size_t k = (base <= 55) ? 55 - base : 119 - base;
    char pad[k];
    memset(pad, 0, k);

    // Append padding
    input_append_seq(in, pad, k);

    // Append length of the original message (in bits)
    original_len += prev_length;
    original_len = htobe64(original_len * 8);
    input_append_seq(in, &original_len, 8);

    assert(in->len % 64 == 0 && "Message is not padded correctly to 512b");
}

// Basic SHA-256 macros
#define CH(x, y, z)  (((x) & (y)) ^ (~(x) & (z)))
#define MAJ(x, y, z) (((x) & (y)) ^ ((x) & (z)) ^ ((y) & (z)))
#define ROTR(x, n)   (((x) >> (n)) | ((x) << (32 - (n))))
#define SHR(x, n)    ((x) >> (n))

// Composite SHA-256 macros
#define SIG0(x) (ROTR((x), 2) ^  ROTR((x), 13) ^ ROTR((x), 22))
#define SIG1(x) (ROTR((x), 6) ^  ROTR((x), 11) ^ ROTR((x), 25))
#define sig0(x) (ROTR((x), 7) ^  ROTR((x), 18) ^ SHR((x), 3))
#define sig1(x) (ROTR((x), 17) ^ ROTR((x), 19) ^ SHR((x), 10))

// Default hash values used to start SHA256
const uint32_t init_hash_values[8] =
    {0x6a09e667, 0xbb67ae85, 0x3c6ef372, 0xa54ff53a,
     0x510e527f, 0x9b05688c, 0x1f83d9ab, 0x5be0cd19};

/* Calculate SHA-256 hash of the input `in` and return it in `hash`
   The hashing algorithm is initialized with the `hash` array
   `prev_length` is passed to the padding function (0 for normal SHA-256, >0 for length extension attack) */
void sha256(struct input *in, const uint32_t prev_length, uint32_t *hash) {
    // Initial hash values
    uint32_t mschedule[64];
    const uint32_t K256[64] = {0x428a2f98, 0x71374491, 0xb5c0fbcf, 0xe9b5dba5, 0x3956c25b, 0x59f111f1, 0x923f82a4, 0xab1c5ed5,
                         0xd807aa98, 0x12835b01, 0x243185be, 0x550c7dc3, 0x72be5d74, 0x80deb1fe, 0x9bdc06a7, 0xc19bf174,
                         0xe49b69c1, 0xefbe4786, 0x0fc19dc6, 0x240ca1cc, 0x2de92c6f, 0x4a7484aa, 0x5cb0a9dc, 0x76f988da,
                         0x983e5152, 0xa831c66d, 0xb00327c8, 0xbf597fc7, 0xc6e00bf3, 0xd5a79147, 0x06ca6351, 0x14292967,
                         0x27b70a85, 0x2e1b2138, 0x4d2c6dfc, 0x53380d13, 0x650a7354, 0x766a0abb, 0x81c2c92e, 0x92722c85,
                         0xa2bfe8a1, 0xa81a664b, 0xc24b8b70, 0xc76c51a3, 0xd192e819, 0xd6990624, 0xf40e3585, 0x106aa070,
                         0x19a4c116, 0x1e376c08, 0x2748774c, 0x34b0bcb5, 0x391c0cb3, 0x4ed8aa4a, 0x5b9cca4f, 0x682e6ff3,
                         0x748f82ee, 0x78a5636f, 0x84c87814, 0x8cc70208, 0x90befffa, 0xa4506ceb, 0xbef9a3f7, 0xc67178f2};

    // Add padding to message
    padding(in, prev_length);

    // Process message in 512-bit blocks
    for (size_t i = 0; i < in->len; i += 64) {
        // 1. Prepare message schedule
        // Copy first sixteen 32b words (64B) into message schedule
        memcpy(mschedule, in->data + i, 64);
        // Fix endianness (to big-endian)
        for (size_t j = 0; j < 16; j++) {
            mschedule[j] = htobe32(mschedule[j]);
        }
        // Calculate the rest of the message schedule
        for (size_t j = 16; j < 64; j++) {
            mschedule[j] = sig1(mschedule[j - 2]) + mschedule[j - 7] + sig0(mschedule[j - 15]) + mschedule[j - 16];
        }

        // Initialize working variables
        // (0,1,2,3,4,5,6,7)
        // (a,b,c,d,e,f,g,h)
        uint32_t w[8];
        memcpy(w, hash, 32);

        // 3. Main loop
        for (size_t j = 0; j < 64; j++) {
            uint32_t T1 = w[7] + SIG1(w[4]) + CH(w[4], w[5], w[6]) + K256[j] + mschedule[j];
            uint32_t T2 = SIG0(w[0]) + MAJ(w[0], w[1], w[2]);
            w[7] = w[6];
            w[6] = w[5];
            w[5] = w[4];
            w[4] = w[3] + T1;
            w[3] = w[2];
            w[2] = w[1];
            w[1] = w[0];
            w[0] = T1 + T2;
        }

        // 4. Compute the intermediate hash value
        for (size_t j = 0; j < 8; j++) {
            hash[j] += w[j];
        }
    }
}

// Print hash to stdout ending with newline
void print_hash(uint32_t *hash) {
    for (size_t i = 0; i < 8; i++) {
        printf("%08x", hash[i]);
    }
    printf("\n");
}

/* ======================== Checksum generation ======================== */

// Generate SHA-256 checksum of the input and print it to stdout
int checksum(struct input *in) {
    uint32_t hash[8];
    memcpy(hash, init_hash_values, 32);
    sha256(in, 0, hash);
    print_hash(hash);
    return 0;
}

/* ======================== MAC generation ======================== */

// Generate MAC using SHA-256 of the input and print it to stdout
int genmac(const struct prog_args args, struct input *in) {
    // Insert key to the start of the input message
    input_insert_start_seq(in, args.key, strlen(args.key));

    // Calculate SHA-256 and print the hash
    checksum(in);
    return 0;
}

/* ======================== MAC validation ======================== */

// Convert MAC from string in `mac` to hex array of 8 items in `hex`
void mac2hex(const char *mac, uint32_t *hex) {
    char numstring[9] = {0};
    for (size_t i = 0; i < 8; i++) {
        // Separate each word of the hash and convert it to a number
        memcpy(numstring, mac + i * 8, 8);
        hex[i] = strtoul(numstring, NULL, 16);
    }
}

// Validate MAC of the input, print MAC and return 0 if it is valid, otherwise return 1
int valmac(struct prog_args args, struct input *in) {
    // Insert key to the start of the input message
    input_insert_start_seq(in, args.key, strlen(args.key));

    // Calculate SHA-256
    uint32_t hash[8];
    memcpy(hash, init_hash_values, 32);
    sha256(in, 0, hash);

    // Convert input MAC to hex numbers
    uint32_t chs[8];
    mac2hex(args.chs, chs);

    // Compare calculated MAC with input MAC
    int are_equal = 0;
    if (memcmp(hash, chs, 32) != 0) {
        are_equal = 1;
    }

    return are_equal;
}

/* ======================== Length extension attack ======================== */

// Print the message with padding - skip the `key_length` bytes at the start
// print following `text_length` bytes as characters and the rest as hex numbers in \xXX format
void print_padded_msg(struct input *in, uint32_t text_length, uint32_t key_length) {
    for (size_t i = key_length; i < key_length + text_length; i++) {
        printf("%c", in->data[i]);
    }

    for (size_t i = key_length + text_length; i < in->len; i++) {
        printf("\\x%02x", in->data[i]);
    }
}

// Execute length extension attack on the input MAC
int extattack(struct prog_args args, struct input *in) {
    // Load the message to be appended for the length extension attack into input structure
    struct input msg_ext = {0};
    size_t msg_ext_len = strlen(args.attack_msg);
    input_append_seq(&msg_ext, args.attack_msg, msg_ext_len);

    // Convert input MAC to hex numbers
    uint32_t chs[8];
    mac2hex(args.chs, chs);

    // Calculate the new MAC using the length extension attack and print it
    uint32_t prev_length = (in->len + args.attack_key_len + 9); // 9 = +1 for the mandatory 0x80 and 8B of length at the end
    uint32_t prev_blocks_cnt = prev_length / 64;
    if (prev_length % 64 != 0) { // If the padding does not fit into the current block, add another block
        prev_blocks_cnt++;
    }
    sha256(&msg_ext, prev_blocks_cnt * 64, chs);
    print_hash(chs);

    // Next, print the padded message with the appended message
    // Store the original length - this will be printed as normal characters
    size_t original_text_length = in->len;
    // Insert random bytes to start of the message to simulate the secret key
    char key_padding[args.attack_key_len];
    input_insert_start_seq(in, key_padding, args.attack_key_len);
    // Create the padded message text
    padding(in, 0);
    // Print the padded message starting from key_length (will not be printed), then
    // print original message as characters and the rest of the padding as hex numbers
    print_padded_msg(in, original_text_length, args.attack_key_len);
    msg_ext.len = msg_ext_len;
    // After the padded original message, print the message to be appended
    input_print(&msg_ext);
    input_free(&msg_ext);
    
    return 0;
}

/* ======================== Main ======================== */

int main(int argc, char **argv) {
    struct prog_args args = {0};
    int res;

    // Parse arguments
    res = parse_args(argc, argv, &args);
    if (res != 0) {
        return res;
    }

    // Read input message
    struct input input = {0};
    input_realloc(&input, 1024);
    input_read_stdin(&input);

    // Execute action
    switch (args.action) {
    case CHKSUM:
        res = checksum(&input);
        break;
    case GENMAC:
        res = genmac(args, &input);
        break;
    case VALMAC:
        res = valmac(args, &input);
        break;
    case EXTATTACK:
        res = extattack(args, &input);
        break;
    default:
        fprintf(stderr, "Unknown action");
        return 1;
    }

    input_free(&input);
    return res;
}
