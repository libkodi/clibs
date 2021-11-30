#include <iostream>
#include <cerrno>
#include <cstring>
#include "clibs/encrypt/rsa.hpp"
#include "clibs/encrypt/base64.hpp"
#include "clibs/error.hpp"

using namespace clibs;
using namespace clibs::encrypt;

void print_public_key(RSA* rsa) {
    std::string pubkey;
    std::cout << "\n\nPublicKey:" << std::endl;

    if (!rsa_read_pkcs1_pubkey(rsa, pubkey)) {
        std::cout << "Failed to read pkcs1 public key." << std::endl;
    } else {
        std::cout << "PKCS1:" << std::endl;
        std::cout << pubkey << std::endl;
    }

    if (!rsa_read_pkcs8_pubkey(rsa, pubkey)) {
        std::cout << "Failed to read pkcs8 public key." << std::endl;
    } else {
        std::cout << "PKCS8:" << std::endl;
        std::cout << pubkey << std::endl;
    }
}

void print_private_key(RSA* rsa) {
    std::string prikey;
    std::cout << "\n\nPrivateKey:" << std::endl;

    if (!rsa_read_pkcs1_prikey(rsa, prikey)) {
        std::cout << "Failed to read pkcs1 private key." << std::endl;
    } else {
        std::cout << "PKCS1:" << std::endl;
        std::cout << prikey << std::endl;
    }

    if (!rsa_read_pkcs8_prikey(rsa, prikey)) {
        std::cout << "Failed to read pkcs8 private key." << std::endl;
    } else {
        std::cout << "PKCS8:" << std::endl;
        std::cout << prikey << std::endl;
    }
}

void print_sign_with_verify(RSA* rsa, const char* con) {
    size_t conlen = strlen(con);
    size_t signlen = base64_encode_size(128);
    char sign_result[signlen];

    std::cout << "\n\nSignature With Verify:" << std::endl;

    if (!rsa_sign(rsa, con, conlen, EVP_sha256(), sign_result, &signlen)) {
        std::cout << "Failed to generate signature." << std::endl;
    } else {
        sign_result[signlen] = '\0';
        std::cout << "Sign: " << con << " ----> " << sign_result << std::endl;
    }

    if (!rsa_verify(rsa, sign_result, signlen, con, conlen, EVP_sha256())) {
        std::cout << "Failed to verify signature." << std::endl;
    } else {
        std::cout << "Verify: Success" << std::endl;
    }
}

void print_pubkey_encrypt_prikey_decrypt(RSA* rsa, const char* con) {
    size_t conlen = strlen(con);
    size_t encrypt_size = rsa_encrypt_base64_size(rsa, conlen);
    char encrypted[encrypt_size];

    std::cout << "\n\nPublicKey Encrypt & PrivateKey Decrypt" << std::endl;

    if (!rsa_public_encrypt(rsa, con, conlen, RSA_PKCS1_PADDING, encrypted, &encrypt_size)) {
        std::cout << "Failed to encrypt content." << std::endl;
    } else {
        std::cout << "PublicKey Encrypted:" << std::endl;
        std::cout << encrypted << std::endl;
    }

    size_t decrypt_size = rsa_decrypt_size(encrypt_size);
    char decrypted[decrypt_size];

    if (!rsa_private_decrypt(rsa, encrypted, encrypt_size, RSA_PKCS1_PADDING, decrypted, &decrypt_size)) {
        std::cout << "Failed to decrypt content." << std::endl;
    } else {
        decrypted[decrypt_size] = '\0';
        std::cout << "PrivateKey Decrypt:" << std::endl;
        std::cout << decrypted << std::endl;
    }
}

void print_pubkey_decrypt_prikey_encrypt(RSA* rsa, const char* con) {
    size_t conlen = strlen(con);
    size_t encrypt_size = rsa_encrypt_base64_size(rsa, conlen);
    char encrypted[encrypt_size];

    std::cout << "\n\nPrivateKey Encrypt & PublicKey Decrypt" << std::endl;

    if (!rsa_private_encrypt(rsa, con, conlen, RSA_PKCS1_PADDING, encrypted, &encrypt_size)) {
        std::cout << "Failed to encrypt content." << std::endl;
    } else {
        std::cout << "PrivateKey Encrypted:" << std::endl;
        std::cout << encrypted << std::endl;
    }

    size_t decrypt_size = rsa_decrypt_size(encrypt_size);
    char decrypted[decrypt_size];

    if (!rsa_public_decrypt(rsa, encrypted, encrypt_size, RSA_PKCS1_PADDING, decrypted, &decrypt_size)) {
        std::cout << "Failed to decrypt content." << std::endl;
    } else {
        decrypted[decrypt_size] = '\0';
        std::cout << "PublicKey Decrypt:" << std::endl;
        std::cout << decrypted << std::endl;
    }
}

int main(int argc, char const *argv[])
{
    RSA* rsa = rsa_keypair(1024);

    if (rsa == NULL) {
        std::cout << "Failed to create keys." << std::endl;
        exit(0);
    }

    /**
     * 生成密钥对
     */
    std::string prikey, pubkey;

    print_public_key(rsa);
    print_private_key(rsa);

    /**
     * 签名与验签
     */
    
    const char* con = "Hello RSA!";
    size_t conlen = strlen(con);
    
    print_sign_with_verify(rsa, con);

    /**
     * 加密与解密
     */
    print_pubkey_encrypt_prikey_decrypt(rsa, con);
    print_pubkey_decrypt_prikey_encrypt(rsa, con);

    rsa_free(rsa);

    return 0;
}