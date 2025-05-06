#pragma once

typedef char const *const str_const;

typedef unsigned char uint8_t;
typedef uint8_t *hash; // 16 Byte Array,
typedef uint8_t const *const hash_const;

typedef hash *hashes; // Array type for a list of hashes.
typedef hash_const *const hashes_const;

constexpr unsigned short int MD5_DIGEST_LENGTH = 16;
constexpr uint8_t const EMPTY_HASH[MD5_DIGEST_LENGTH] = {
    0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0, 0};

int stringLength(str_const);
char *stringDup(str_const);
char *stringConcat(str_const, str_const);
bool compareStrings(str_const, str_const);
hash computeHash(hashes_const, int);
bool compareHashes(hash_const, hash_const);
hash hashDup(hash_const);