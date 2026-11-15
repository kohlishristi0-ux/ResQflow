#pragma once
extern "C" { struct EVP_MD; const EVP_MD* EVP_sha256();
int PKCS5_PBKDF2_HMAC(const char*,int,const unsigned char*,int,int,const EVP_MD*,int,unsigned char*); }
