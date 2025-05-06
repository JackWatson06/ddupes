#include "lib.h"

#include <openssl/evp.h>

int stringLength(str_const str) {
  int length = 0;
  while (str[length] != '\0') {
    ++length;
  }

  return length + 1; // One for the null char
}

char *stringDup(str_const string_one) {
  int length = stringLength(string_one);

  char *string_two = new char[length];
  for (int i = 0; i < length; i++) {
    string_two[i] = string_one[i];
  }

  return string_two;
}

char *stringConcat(str_const one, str_const two) {
  if (one == nullptr || two == nullptr) {
    return new char[1]{'\0'};
  }

  int string_one_length = stringLength(one);
  int string_two_length = stringLength(two);
  char *concated_string = new char[string_one_length + string_two_length -
                                   1]; // Minus one because stringLength
                                       // includes the null terminator.

  for (int i = 0; i < string_one_length - 1; ++i) {
    concated_string[i] = one[i];
  }

  for (int i = 0; i < string_two_length - 1; ++i) {
    concated_string[i + (string_one_length - 1)] = two[i];
  }

  concated_string[string_one_length + string_two_length - 2] = '\0';
  return concated_string;
}

bool compareStrings(str_const string_one, str_const string_two) {
  int i = 0;
  while (string_one[i] != '\0' && string_two[i] != '\0') {
    if (string_one[i] != string_two[i]) {
      return false;
    }
    ++i;
  }

  return string_one[i] == string_two[i];
}

hash computeHash(hashes_const hashes, int num_of_hashes) {
  EVP_MD_CTX *md_context;
  unsigned char *md5_digest;
  unsigned int md5_digest_len = EVP_MD_size(EVP_md5());

  md_context = EVP_MD_CTX_new();
  EVP_DigestInit_ex(md_context, EVP_md5(), nullptr);

  for (int i = 0; i < num_of_hashes; ++i) {
    EVP_DigestUpdate(md_context, hashes[i], MD5_DIGEST_LENGTH);
  }

  md5_digest = (unsigned char *)OPENSSL_malloc(md5_digest_len);
  EVP_DigestFinal_ex(md_context, md5_digest, &md5_digest_len);
  EVP_MD_CTX_free(md_context);

  /**
   * TODO: Do I really need a free below, if I can free later so
  // I don't need to do a memcp?
   * OPENSSL_free(md5_digest);
   */
  return (hash)md5_digest;
}

bool compareHashes(hash_const hash_one, hash_const hash_two) {
  if (!hash_one || !hash_two) {
    return hash_one == hash_two;
  }

  for (int i = 0; i < MD5_DIGEST_LENGTH; ++i) {
    if (hash_one[i] != hash_two[i]) {
      return false;
    }
  }

  return true;
}

hash hashDup(hash_const hash_one) {
  hash tmp_hash = new uint8_t[MD5_DIGEST_LENGTH];

  for (int i = 0; i < MD5_DIGEST_LENGTH; ++i) {
    tmp_hash[i] = hash_one[i];
  }

  return tmp_hash;
}
