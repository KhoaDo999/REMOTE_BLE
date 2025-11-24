#include "AES128Encryptor.h"

#include "mbedtls/aes.h"// thư viện cung cấp các hàm mã giải    

AES128Encryptor::AES128Encryptor(const unsigned char *key)
{
    for (int i = 0; i < 16; i++)
    {
        this->key[i] = key[i];
    }
}

void AES128Encryptor::encrypt(const unsigned char *input, unsigned char *output)
{
    mbedtls_aes_context aes;// chứa key trạng thái, buffer nội bộ
    mbedtls_aes_init(&aes);//chuẩn bị thao tác mã hóa
    mbedtls_aes_setkey_enc(&aes, this->key, 128);//lấy key trong constructor với độ dài 128 bit để cho cbi mã hóa
    mbedtls_aes_crypt_ecb(&aes, MBEDTLS_AES_ENCRYPT, input, output);// mã hóa: chế độ ecb, vì đây là chế độ đơn giản nhất trong dàn aes
    mbedtls_aes_free(&aes);// giải phóng bộ nhớ. tránh rò rỉ
}

// String AES128Encryptor::encryptToString(const String &input)

// {
//     unsigned char paddedInput[16] = {0};
//     unsigned char encrypted[16] = {0};

//     int len = input.length();
//     if (len > 16)
//         len = 16;
//     for (int i = 0; i < len; i++)
//     {
//         paddedInput[i] = input.charAt(i);
//     }

//     encrypt(paddedInput, encrypted);

//     String result = "";
//     for (int i = 0; i < 16; i++)
//     {
//         if (encrypted[i] < 16)
//             result += "0";
//         result += String(encrypted[i], HEX);
//     }
//     return result;
// }