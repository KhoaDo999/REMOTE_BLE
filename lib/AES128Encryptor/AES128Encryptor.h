#ifndef AES128_ENCRYPTOR_H
#define AES128_ENCRYPTOR_H

#include <Arduino.h>

class AES128Encryptor
{
private:
    unsigned char key[16];

public:
    AES128Encryptor(const unsigned char *key);
    void encrypt(const unsigned char *input, unsigned char *output);
    //String encryptToString(const String &input);
};

#endif